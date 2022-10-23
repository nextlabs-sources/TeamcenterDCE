package com.nextlabs.drm.rmx.batchprocess;

import static com.nextlabs.drm.rmx.batchprocess.configuration.BatchProcessConstants.*;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Calendar;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Map;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.atomic.AtomicLong;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.nextlabs.drm.rmx.batchprocess.dto.DatasetDTO;
import com.nextlabs.drm.rmx.batchprocess.dto.TaskFileDTO;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;
import com.teamcenter.services.loose.core._2006_03.FileManagement.DatasetFileInfo;
import com.teamcenter.services.loose.core._2006_03.FileManagement.GetDatasetWriteTicketsInputData;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.core.ReservationService;
import com.teamcenter.services.strong.core.SessionService;
import com.teamcenter.services.strong.core._2007_01.DataManagement.VecStruct;
import com.teamcenter.services.strong.core._2007_06.DataManagement.ReferenceInfo;
import com.teamcenter.services.strong.core._2007_09.DataManagement.NamedReferenceInfo;
import com.teamcenter.services.strong.core._2007_09.DataManagement.RemoveNamedReferenceFromDatasetInfo;
import com.teamcenter.services.strong.core._2007_12.Session.StateNameValue;
import com.teamcenter.soa.client.FileManagementUtility;
import com.teamcenter.soa.client.GetFileResponse;
import com.teamcenter.soa.client.model.ErrorStack;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.Property;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.Dataset;
import com.teamcenter.soa.client.model.strong.ImanFile;
import com.teamcenter.soa.client.model.strong.ImanVolume;
import com.teamcenter.soa.client.model.strong.POM_system_class;
import com.teamcenter.soa.exceptions.NotLoadedException;

public abstract class FileProcessTask implements Runnable {
	
	private static final Object VOL_LOCK = new Object();
	
	protected final DatasetDTO transDataset;
	protected final Dataset dataset;
	protected String objectName;
	protected final String prefix;
	protected final StringBuilder reportMsgBuilder;
	protected final String stagingFoldPath;
	
	// Tc services
	protected final DataManagementService dmService;
	protected final ReservationService rmService;
	private final SessionService sesService;
	private final FileManagementUtility fmUtil;
	
	// internal lookup service
	private final RefInfoService refInfoService;
	private final VolInfoService volInfoService;
	
	private final AtomicLong totalFilesSize;
	
	protected Queue<TaskFileDTO> processQueue;
	protected Queue<TaskFileDTO> uploadQueue;
	
	protected static final Logger LOGGER = LogManager.getLogger("BPLOGGER");
	protected static final Logger REPORTLOGGER = LogManager.getLogger("REPORTLOGGER");
	
	public FileProcessTask(Dataset dataset, 
			DataManagementService dmService, ReservationService rsvService, 
			SessionService sesService, FileManagementUtility fmUtil, 
			RefInfoService refInfoService, VolInfoService volInfoService,
			String stagingFoldPath, AtomicLong totalFilesSize) throws Exception {
		this.dataset = dataset;
		this.reportMsgBuilder = new StringBuilder("");
		this.stagingFoldPath = stagingFoldPath;
		
		this.dmService = dmService;
		this.rmService = rsvService;
		this.sesService = sesService;
		this.fmUtil = fmUtil;
		
		this.refInfoService = refInfoService;
		this.volInfoService = volInfoService;
		
		this.totalFilesSize = totalFilesSize;
		this.transDataset = new DatasetDTO.Builder(dataset.getUid()).build();
		
		try {
			reportMsgBuilder.append("[" + dataset.getUid() + "] " + dataset.get_object_name() + " ");
			objectName = dataset.get_object_name();
		} catch (NotLoadedException e) {
			LOGGER.error("[" + dataset.getUid() + "] caught err: " + e.getMessage(), e);
			objectName = "-";
		}
		
		prefix = "[" + dataset.getUid() + "] " + objectName;
	}
	
	@Override
	public void run() {
		long startTime = System.nanoTime();
		
		LOGGER.info(prefix + " checkout");
		ServiceData serviceData = rmService.checkout(
				new ModelObject[]{ dataset }, "batchprocess: encrypt/decrypt files", "1");
		
		if (LOGGER.isTraceEnabled())
			BatchProcessUtility.logServiceData(serviceData);
				
		if (validState(serviceData)) {
			try {
				downloadFiles();
				processFiles();
				
				// test persist mod user & date
				readLastModNXL();
				setLastModNXL();
				setIsNXLProtected();
				
				uploadFiles();
			} catch (Exception ex) {
				LOGGER.error(prefix + " caught exception: " + ex.getMessage(), ex);
			}
			
			LOGGER.info(prefix + " cancel checkout");
			serviceData = rmService.cancelCheckout(new ModelObject[]{ dataset });
			if (LOGGER.isTraceEnabled())
				BatchProcessUtility.logServiceData(serviceData);
		}

		fmUtil.term();
		
		long duration = System.nanoTime() - startTime;
		REPORTLOGGER.info(reportMsgBuilder.toString() + "completed in " + BatchProcessUtility.getTimeFormat(duration));
	}
	
	private boolean validState(ServiceData serviceData) {
		if (serviceData.sizeOfPartialErrors() == 0)
			return true;
		else {
			LOGGER.warn(prefix + " caught errors during checkin/checkout");
			for (int i = 0; i < serviceData.sizeOfPartialErrors(); i++) {
				ErrorStack errStack = serviceData.getPartialError(i);
				int[] errCodes = errStack.getCodes();
				
				for (String errMsg : errStack.getMessages()) {
					LOGGER.warn("      Error message: " + errMsg);
				}
				LOGGER.warn("      Error codes: " + Arrays.toString(errCodes));
				
				for (int errCode : errCodes) {
					// dataset is not checkout at 1st checking point
					// in this case, it can checkout by this process
					// and Document Revision's datasets will be
					// checked out all at once.
					if (errCode == 32009)
						return true;
				}
			}
		}
		
		return false;
	}
	
	protected abstract void processFiles(); 
	
	protected abstract boolean filterFileToDownload(String fileExtension, String namedReference);
	
	protected abstract boolean isNXLProtected();
	
	private final void downloadFiles() {
		long startTime = System.nanoTime();
		
		LOGGER.info(prefix + " downloadFiles()");
		processQueue = new LinkedList<>();
		String[] refNames = null;
		ModelObject[] refList = null;
		
		dmService.refreshObjects(new ModelObject[] {dataset});
		LOGGER.debug(prefix + " downloadFiles() getting properties via datamanagement");
		ServiceData serviceData = dmService.getProperties(
				new ModelObject[] {dataset}, PROPS_DATASET);
		
		if (LOGGER.isTraceEnabled())
			BatchProcessUtility.logServiceData(serviceData);
		
		try {
			refNames = dataset.get_ref_names();
			refList = dataset.get_ref_list();
		} catch (NotLoadedException e) {
			LOGGER.error(prefix + " caught exception: " + e.getMessage(), e);
		}
		
		if (refList == null || refList.length <= 0)
			return;
		
		dmService.refreshObjects(refList);
		
		File datasetFolder = new File(stagingFoldPath + File.separator + dataset.getUid());
		try {
			if (!datasetFolder.mkdirs() && !datasetFolder.exists()) {
				LOGGER.error(prefix + " downloadFiles() failed to create folders: " + datasetFolder.getAbsolutePath());
				return;
			}
			
			if (!datasetFolder.canRead() || !datasetFolder.canWrite()) {
				LOGGER.error(prefix + " downloadFiles() failed to read/write folders: " + datasetFolder.getAbsolutePath());
				return;
			}
		} catch (SecurityException se) {
			LOGGER.error(prefix + " downloadFiles() create folders caught exception: " + se.getMessage(), se);
			return;
		}
		
		int fileCount = 0, index = 0;
		long totalFileSizeB = 0L;
		long downloadedSizeB = 0L;
		for (ModelObject mo : refList) {
			if (mo instanceof ImanFile) {
				LOGGER.debug(prefix + " downloadFiles() getting properties via datamanagement");
				serviceData = dmService.getProperties(new ModelObject[] { mo }, PROPS_IMANFILE);
				
				if (LOGGER.isTraceEnabled())
					BatchProcessUtility.logServiceData(serviceData);
				
				ImanFile imanFile = (ImanFile) mo;
				
				try {
					long fileSize = Long.parseLong(imanFile.getPropertyDisplayableValue("byte_size"));
					String originalFileName = imanFile.get_original_file_name();
					String fileExt = imanFile.get_file_ext();
					String newFileName = datasetFolder.getAbsolutePath() + 
							File.separator + originalFileName;
					
					if (filterFileToDownload(fileExt, refNames[index])) {
						GetFileResponse gfs = fmUtil.getFileToLocation(
								mo, newFileName, null, null);
						File downloadedFile = new File(newFileName);
						processQueue.add(new TaskFileDTO(imanFile.getUid(), refNames[index], imanFile, downloadedFile));
						
						if (fileSize != downloadedFile.length())
							REPORTLOGGER.info("invalid file size for " + imanFile.get_original_file_name() + 
									" " + BatchProcessUtility.getSizeFormat(downloadedFile.length()) + "/" + fileSize);
						
						totalFileSizeB += fileSize;
						downloadedSizeB += downloadedFile.length();
						fileCount += gfs.sizeOfFiles();
					}
				} catch (NotLoadedException e) {
					LOGGER.error(prefix + " get properties for imanFile caught exception: " + 
							e.getMessage(), e);
				} catch (NumberFormatException nfe) {
					LOGGER.error(prefix + " get byte_size for imanFile caught number format exception: " +
							nfe.getMessage(), nfe);
				}
			}
			
			index++;
		}
		
		totalFilesSize.getAndAdd(totalFileSizeB);
		
		LOGGER.debug(prefix + " decrypt queue has " + processQueue.size() + " tasks");
		long duration = System.nanoTime() - startTime;
		reportMsgBuilder.append("download[" + fileCount + ", " + 
				BatchProcessUtility.getSizeFormat(downloadedSizeB) + "/" + 
				BatchProcessUtility.getSizeFormat(totalFileSizeB) + ", " + 
				BatchProcessUtility.getTimeFormat(duration) + "] ");
	}
	
	private final void uploadFiles() {		
		long startTime = System.nanoTime();
		
		String dsTypeName = dataset.getTypeObject().getName();
		DatasetFileInfo[] dFileInfos = new DatasetFileInfo[uploadQueue.size()];
		
		RemoveNamedReferenceFromDatasetInfo[] remNRFrmInfos = new RemoveNamedReferenceFromDatasetInfo[1];
		remNRFrmInfos[0] = new RemoveNamedReferenceFromDatasetInfo();
		remNRFrmInfos[0].dataset = (Dataset) dataset;
		NamedReferenceInfo[] namedRefInfos = new NamedReferenceInfo[uploadQueue.size()];
		
		int fileCount = 0, index = 0;
		long totalFileSizeB = 0L;
		
		// source volume
		String srcVolUid = null;
		String srcVolName = null;
		if (!uploadQueue.isEmpty()) {
			TaskFileDTO fileDTO = uploadQueue.peek();
			
			try {
				if (srcVolUid == null) {
					ImanVolume srcVolume = fileDTO.getImanFile().get_volume_tag();
					srcVolUid = srcVolume.getUid();
					srcVolName = volInfoService.getVolumeName(srcVolume);
				}
			} catch (NotLoadedException e) {
				LOGGER.error(prefix + " uploadFiles() get properties caught exception: " + e.getMessage(), e);
			}
		}
		
		while (!uploadQueue.isEmpty()) {
			TaskFileDTO fileDTO = uploadQueue.remove();
			String fileName = fileDTO.getLocalFile().getName();
			String fileExtension = "";
			
			// remove
			namedRefInfos[index] = new NamedReferenceInfo();
			namedRefInfos[index].type = fileDTO.getNamedRef();
			namedRefInfos[index].deleteTarget = true;
			namedRefInfos[index].targetObject = fileDTO.getImanFile();
			
			try {
				LOGGER.info(prefix + " uploadFiles() removing NR: " + fileDTO.getNamedRef() + 
						", " + fileDTO.getImanFile().get_original_file_name());
			} catch (NotLoadedException e) {
				LOGGER.error(prefix + " uploadFiles() get properties caught exception: " + e.getMessage(), e);
			}
			
			// upload
			int lastDot = fileName.lastIndexOf(".");
			if (lastDot > 0)
				fileExtension = fileName.substring(lastDot);
		
			ReferenceInfo namedRef = refInfoService.getNamedReference(dsTypeName, fileExtension);
						
			dFileInfos[index] = new DatasetFileInfo();
			dFileInfos[index].clientId = "" + index;
			dFileInfos[index].fileName = fileDTO.getLocalFile().getAbsolutePath();
			dFileInfos[index].namedReferencedName = namedRef.referenceName;
			dFileInfos[index].isText = !refInfoService.isBinary(namedRef.fileFormat);
			dFileInfos[index].allowReplace = false;
			
			LOGGER.info(prefix + " uploadFiles() adding NR: " + namedRef + ", " + fileName + ", " + dFileInfos[index].isText);
			
			totalFileSizeB += fileDTO.getLocalFile().length();
			fileCount++;
			index++;
		}
		
		remNRFrmInfos[0].nrInfo = namedRefInfos;
		LOGGER.debug(prefix + " uploadFiles() remove NR via data management");
		ServiceData serviceData = dmService.removeNamedReferenceFromDataset(remNRFrmInfos);
		if (LOGGER.isTraceEnabled())
			BatchProcessUtility.logServiceData(serviceData);
		
		GetDatasetWriteTicketsInputData dataUpload = new GetDatasetWriteTicketsInputData();
		dataUpload.dataset = dataset;
		dataUpload.datasetFileInfos = dFileInfos;
		dataUpload.createNewVersion = false;
		
		synchronized (VOL_LOCK) {
			if (srcVolUid != null) {
				StateNameValue[] stateNameVals = new StateNameValue[1];
				stateNameVals[0] = new StateNameValue();
				stateNameVals[0].name = "volume";
				stateNameVals[0].value = srcVolUid;
				LOGGER.info("setting srcVol [" + srcVolUid + "] " + srcVolName);
				ServiceData sesServData = sesService.setUserSessionState(stateNameVals);
				if (LOGGER.isTraceEnabled())
					BatchProcessUtility.logServiceData(sesServData);
			}
					
			LOGGER.debug(prefix + " uploadFiles() add NR via data management");
			serviceData = fmUtil.putFiles(new GetDatasetWriteTicketsInputData[] {dataUpload});
		}
		
		if (LOGGER.isTraceEnabled())
			BatchProcessUtility.logServiceData(serviceData);
		
		if (verifyFilesUploaded(dFileInfos))
			cleanup();
		
		long duration = System.nanoTime() - startTime;
		reportMsgBuilder.append("upload[" + fileCount +", " + 
				BatchProcessUtility.getSizeFormat(totalFileSizeB) + ", " + 
				BatchProcessUtility.getTimeFormat(duration) + "] ");
	}
	
	protected final String truncateExtension(String filename) {
		int indexLastDot = filename.toLowerCase().lastIndexOf(DOT_NXL_FILE_EXT);
		
		if (indexLastDot > 0)
			return filename.substring(0, indexLastDot);
		else
			return filename;
	}
	
	protected final String appendExtension(String filename) {
		int indexLastDot = filename.toLowerCase().lastIndexOf(DOT_NXL_FILE_EXT);
		
		if (indexLastDot > 0)
			return filename;
		else
			return filename + DOT_NXL_FILE_EXT;
			
	}
	
	private final void cleanup() {
		File stageFolder = new File(stagingFoldPath + File.separator + dataset.getUid());
		try {
			FileUtils.deleteDirectory(stageFolder);
		} catch (IOException e) {
			LOGGER.error(prefix + " cleanup() caught exception: " + e.getMessage(), e);
		}
	}
	
	private final boolean verifyFilesUploaded(DatasetFileInfo[] dFileInfos) {
		String[] refNames = null;
		ModelObject[] refList = null;
		
		Set<String> nrFilename = new HashSet<>();
		for (DatasetFileInfo dFileInfo : dFileInfos) {
			nrFilename.add(new String(dFileInfo.namedReferencedName + dFileInfo.fileName));
		}
		
		dmService.refreshObjects(new ModelObject[] {dataset});
		ServiceData serviceData = dmService.getProperties(
				new ModelObject[] {dataset}, PROPS_DATASET);
		
		if (LOGGER.isTraceEnabled())
			BatchProcessUtility.logServiceData(serviceData);
		
		try {
			refNames = dataset.get_ref_names();
			refList = dataset.get_ref_list();
		} catch (NotLoadedException e) {
			LOGGER.error(prefix + " caught exception: " + e.getMessage(), e);
		}
		
		if (refList == null || refList.length <= 0)
			return false;
		
		serviceData = dmService.getProperties(refList, PROPS_IMANFILE);
		
		if (LOGGER.isTraceEnabled())
			BatchProcessUtility.logServiceData(serviceData);
		
		int index = 0;
		for (ModelObject mo : refList) {
			if (mo instanceof ImanFile) {
				String refName = refNames[index];
				ImanFile imanFile = (ImanFile) mo;
				
				try {
					String originalFileName = imanFile.get_original_file_name();
					String verifiedData = refName + originalFileName;
					if (nrFilename.contains(verifiedData))
						nrFilename.remove(refName + originalFileName);
				} catch (NotLoadedException e) {
					LOGGER.error(prefix + " get properties for imanFile caught exception: " + 
							e.getMessage(), e);
				}
			}
			
			index++;
		}
		
		if(nrFilename.size() <= 0)
			return true;
		else {
			LOGGER.warn(prefix + " verifyFilesUploaded() failed to verify: " + nrFilename.toString());
			REPORTLOGGER.info("failed to verify upload for: " + nrFilename.toString());
		}
		
		return false;
	}
	
	private void readLastModNXL() {			
		dmService.getProperties(new ModelObject[] {dataset}, new String[] {
				LAST_MOD_USER, LAST_MOD_DATE, NXL_LAST_MOD_USER, NXL_LAST_MOD_DATE});
				
		try {
			String dcUid = sesService.getTCSessionInfo().user.getUid();
			String strNxlLastModUser = dataset.getPropertyDisplayableValue(NXL_LAST_MOD_USER).toString();
			String strNxlLastModDate = dataset.getPropertyDisplayableValue(NXL_LAST_MOD_DATE).toString();
			strNxlLastModUser = StringUtils.stripStart(strNxlLastModUser, "[");
			strNxlLastModUser = StringUtils.stripEnd(strNxlLastModUser, "]");
			strNxlLastModDate = StringUtils.stripStart(strNxlLastModDate, "[");
			strNxlLastModDate = StringUtils.stripEnd(strNxlLastModDate, "]");
			
			String strLastModUser = dataset.getPropertyDisplayableValues(LAST_MOD_USER).toString();
			String strLastModDate = dataset.getPropertyDisplayableValues(LAST_MOD_DATE).toString();
			strLastModUser = StringUtils.stripStart(strLastModUser, "[");
			strLastModUser = StringUtils.stripEnd(strLastModUser, "]");
			strLastModDate = StringUtils.stripStart(strLastModDate, "[");
			strLastModDate = StringUtils.stripEnd(strLastModDate, "]");
			
			if (LOGGER.isDebugEnabled()) {
				LOGGER.debug("	dc uid: " + dcUid);
				LOGGER.debug("	last_mod_user: " + dataset.get_last_mod_user().getUid());
				LOGGER.debug("	last_mod_date: " + Property.toDateString(dataset.get_last_mod_date()));
				LOGGER.debug("	nxl3_last_mod_user: " + strNxlLastModUser);
				LOGGER.debug("	nxl3_last_mod_date: " + strNxlLastModDate);
			}
			
			if (dataset.get_last_mod_user().getUid().equals(dcUid) && 
					!strNxlLastModUser.trim().equals("") && !strNxlLastModDate.trim().equals("")) {
				LOGGER.debug("	used nxl last mod fields");
				
				POM_system_class lastModUser = (POM_system_class) dataset.getPropertyObject(NXL_LAST_MOD_USER).getModelObjectValue();
				Calendar lastModDate = dataset.getPropertyObject(NXL_LAST_MOD_DATE).getCalendarValue();
				
				transDataset.setNxlLastModUser(lastModUser.getUid());
				transDataset.setNxlLastModDate(Property.toDateString(lastModDate));
			} else {
				LOGGER.debug("	used last mod fields");
				
				transDataset.setNxlLastModUser(dataset.get_last_mod_user().getUid());
				transDataset.setNxlLastModDate(Property.toDateString(dataset.get_last_mod_date()));
			}
		} catch (NotLoadedException ex) {
			LOGGER.error("FileProcessTask.readLastModNXL() caught NotLoadedException: " + ex.getMessage(), ex);
		} catch (ServiceException e) {
			LOGGER.error("FileProcessTask.readLastModNXL() caught ServiceException: " + e.getMessage(), e);
		}
	}
	
	private void setLastModNXL() {
		// read from file
		String last_mod_user = transDataset.getNxlLastModUser();
		String last_mod_date = transDataset.getNxlLastModDate();
		
		if (last_mod_user == null || last_mod_date == null)
			LOGGER.error("null value for last mod user/last mod date");
		
		if (LOGGER.isDebugEnabled()) {
			LOGGER.debug("      last_mod_user: " + last_mod_user);
			LOGGER.debug("      last_mod_date: " + last_mod_date);
		}
		
		// set data to dataset		
		Map<String, VecStruct> propertiesToUpdate = new HashMap<>();
		VecStruct vec1 = new VecStruct();
		VecStruct vec2 = new VecStruct();
		
		try {
			vec1.stringVec = new String[] {last_mod_user};
			vec2.stringVec = new String[] {last_mod_date};
			
			propertiesToUpdate.put(NXL_LAST_MOD_USER, vec1);
			propertiesToUpdate.put(NXL_LAST_MOD_DATE, vec2);
			
			ServiceData servData = dmService.setProperties(new ModelObject[] {dataset}, propertiesToUpdate);
			if (LOGGER.isTraceEnabled())
				BatchProcessUtility.logServiceData(servData);
		} catch (Exception ex) {
			LOGGER.error("      FileProcessTask.setLastModNXL() caught Exception: " + 
					ex.getMessage(), ex);
		}
	}
	
	private void setIsNXLProtected() {
		Map<String, VecStruct> propertiesToUpdate = new HashMap<>();
		VecStruct vec1 = new VecStruct();
		
		if (!isNXLProtected())
			vec1.stringVec = new String[] {"false"};
		else
			vec1.stringVec = new String[] {"true"};
		
		try {
			propertiesToUpdate.put(NXL_IS_NXL_PROTECTED, vec1);
			
			ServiceData servData = dmService.setProperties(new ModelObject[] {dataset}, propertiesToUpdate);
			if (LOGGER.isTraceEnabled())
				BatchProcessUtility.logServiceData(servData);
		} catch (Exception ex) {
			LOGGER.error("      FileProcessTask.setIsNXLProtected() caught Exception: " + 
					ex.getMessage(), ex);
		}
	}

}
