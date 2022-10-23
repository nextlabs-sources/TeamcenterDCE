package com.nextlabs.drm.tonxlfile;

/**
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectInputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.nextlabs.drm.tonxlfile.configuration.NextLabsConfigInterface;
import com.nextlabs.drm.tonxlfile.configuration.NextLabsConfigManager;
import com.nextlabs.drm.tonxlfile.configuration.NextLabsConstants;
import com.nextlabs.drm.tonxlfile.dto.IntegrationDatasetDTO;
import com.teamcenter.ets.soa.ConnectionManager;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.core._2007_06.DataManagement.PurgeSequencesInfo;
import com.teamcenter.services.strong.core._2007_09.DataManagement.NamedReferenceInfo;
import com.teamcenter.services.strong.core._2007_09.DataManagement.RemoveNamedReferenceFromDatasetInfo;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.Dataset;
import com.teamcenter.soa.client.model.strong.ImanFile;
import com.teamcenter.soa.client.model.strong.ItemRevision;
import com.teamcenter.soa.exceptions.NotLoadedException;
import static com.nextlabs.drm.tonxlfile.configuration.NextLabsConstants.*;

public class DatabaseOperation extends com.teamcenter.ets.load.DefaultDatabaseOperation {
	
	private static NextLabsConfigInterface nxlConf;
	
	private Dataset locSourceDataset = null;
	private ItemRevision locSourceItemRev = null;
	
	private DataManagementService dmService;
	private ModelObjectUtil moUtil;
	
	private static Map<String, IntegrationDatasetDTO> integrationLookup = null;
	
	public void query() throws Exception {
		try {
			super.query();
			
			locSourceItemRev = (ItemRevision) secondaryObj;
		} catch (Exception ex) {
			if (ex.getMessage().contains("Secondary Object is not set.")) {
				// ignore
				// accept secondary object is not set, for protection on dataset without Item Revision
			} else {
				throw ex;
			}
		}
		
		locSourceDataset = (Dataset) primaryObj;
	}
	
	@SuppressWarnings("unchecked")
	private void loadLookupTable() throws Exception {		
		// setup only when the table is null or empty
		if (integrationLookup == null || integrationLookup.isEmpty()) {
			this.m_zTaskLogger.info("load integrationLookup from file");
			
			File jarFile = new File(this.getClass().getProtectionDomain().getCodeSource().getLocation().getPath());
			File binFile = new File(jarFile.getParentFile().getAbsolutePath() + File.separator + TONXLFILEDATA);
			
			this.m_zTaskLogger.debug("jarFile: " + jarFile.getAbsolutePath());
			this.m_zTaskLogger.debug("binFile: " + binFile.getAbsolutePath());
			
			try (FileInputStream fis = new FileInputStream(binFile);
					ObjectInputStream ois = new ObjectInputStream(fis)
						) {
				integrationLookup = (HashMap<String, IntegrationDatasetDTO>) ois.readObject();
			} catch (IOException ioe) {
				throw ioe;
			} catch (ClassNotFoundException e) {
				throw e;
			}
		}
		
		if (this.m_zTaskLogger.isDebugEnabled()) {
			this.m_zTaskLogger.debug("integrationLookup table:");
			for (String name: integrationLookup.keySet()) {
				IntegrationDatasetDTO data =  integrationLookup.get(name);
				this.m_zTaskLogger.debug(data.toString());
			}
		}
		
		this.m_zTaskLogger.info("load integrationLookup from file done");
	}
	
	public void load() throws Exception {
		this.m_zTaskLogger.info("ToNxlFile entering DatabaseOperation->load()");
		
		// load lookup table from data file
		loadLookupTable();
		
		// fixing the connection lost issues after long-time running
		try {
			NextLabsConfigManager nxlConfigMgr = new NextLabsConfigManager();			
			nxlConf = nxlConfigMgr.getService(ConnectionManager.getActiveConnection());
		} catch (Exception ex) {
			nxlConf = null;
		}
		
		try {
			moUtil = new ModelObjectUtil(this.m_zTaskLogger);
			
			if (nxlConf == null) {
				this.m_zTaskLogger.error("ToNxlFile failed to instantiate the config service");
				
				throw new Exception("ToNxlFile failed to instantiate the config service");
			}
			
			List<String> namedRefList = null;
			List<String> fileList = null;
			NXLResultFolderFilter localNXLFilter = new NXLResultFolderFilter();
			File resultFolder = new File(this.m_scResultDir);
			
			// test UGPART: moved out from the condition body, so we can check the original named reference from the list
			this.m_zTaskLogger.debug("File list: " + resultFolder.getParent() + File.separator + SRC_LIST_FILENAME);
			List<String> lstNamedRefFilename = readNamedRefFilename(resultFolder.getParent() + File.separator + SRC_LIST_FILENAME);

			if (resultFolder.isDirectory()) {
				String[] arrayOfString2 = resultFolder.list(localNXLFilter);
				
				if (arrayOfString2 != null) {
					fileList = new ArrayList<>(arrayOfString2.length);
					namedRefList = new ArrayList<>(arrayOfString2.length);
					
					for (int k = 0; k < arrayOfString2.length; k++) {
						String localObject3 = this.m_scResultDir + File.separator + arrayOfString2[k];

						File localFile2 = new File((String)localObject3);
						this.m_zTaskLogger.debug("NXLFullPath[" + k + "]:" + (String)localObject3);
						
						if (localFile2.exists()) {
							this.m_zTaskLogger.debug("NXLFullPath[" + k + "]:" + (String)localObject3 + 
									" is added to List3");
							
							namedRefList.add(getNamedRef(lstNamedRefFilename, arrayOfString2[k]));
							fileList.add(arrayOfString2[k]);
						}
					}
				}
			}			
			
			if (fileList == null || fileList.size() <= 0) {
				return;
			}
			
			dmService = DataManagementService.getService(ConnectionManager.getActiveConnection());
			
			/*
			 * fix bug 38158
			 * Check the dataset status before process, this is because TC addNRFile does not catch
			 * exception when uploading the file to a checked-out dataset. And it shows updated in
			 * service data, but there is not file added to the checked-out dataset.
			 */
			dmService.getProperties(new ModelObject[] { locSourceDataset }, new String[] { PROP_CHECKED_OUT, PROP_CHECKED_OUT_CHANGE_ID });
			
			int dsCheckoutRetryCount = nxlConf.getDSCheckoutRetryCount();
			int dsCheckoutWaitTime = nxlConf.getDSCheckoutWaitTime();
			
			while (dsCheckoutRetryCount > -1) {
				if (!moUtil.isValidDatasetStatus(locSourceDataset)) {
					this.m_zTaskLogger.info("Dataset has been checked-out and can't be edited");
					
					if (dsCheckoutRetryCount <= 0)
						throw new Exception ("Dataset has been checked-out and can't be edited");
				} else {
					break;
				}
				
				Thread.sleep(dsCheckoutWaitTime);
				dsCheckoutRetryCount -= 1;
			}
			
			
			// initiate nextlabs datasethelper
			NextLabsDataSetHelper nxlDataSetHelper = new NextLabsDataSetHelper(this.m_zTaskLogger,
					true, this.softFailureHandlingEnabled, moUtil);
			
			nxlDataSetHelper.cacheSourceVolume(locSourceDataset);
			
			// early state feature
			// validate whether the file can be stored back to the dataset before removing/adding files
			// this prevents data loss when model does not match (i.e. nextlabs template not present, or foundation template is not customized)
			if (!nxlDataSetHelper.isAllNRFileTypeValid(locSourceDataset, namedRefList, this.m_scResultDir, fileList)) {
				throw new Exception ("Input named reference and file type does not match the model");
			}
			
			// remove unprotected src file
			// need to remove before upload, this is because the upload will check is there other type file
			// before change the tool to nxl file handler
			if (nxlConf.getRemoveDRMSrcFile()) {
				this.m_zTaskLogger.debug("Removing unprotected files from the source dataset");
				
				try {
					moUtil.setBypass();
					removeSrcUnprotectedFile(lstNamedRefFilename);
				} catch (Exception ex) {
					throw ex;
				} finally {
					moUtil.unsetBypass();
				}
				this.m_zTaskLogger.debug("Completed removing unprotected file from the source dataset");
			}
			
			// add protected file into the source dataset
			this.m_zTaskLogger.debug("Adding translated file to the source dataset");
			// fix bug 45118
			//nxlDataSetHelper.addNRFiles(sourceDataset, namedRefList, this.m_scResultDir, fileList, this.sourceDataset, nxlConf.getRemoveDRMSrcFile());
			nxlDataSetHelper.addNRFiles(locSourceDataset, namedRefList, this.m_scResultDir, fileList, this.locSourceDataset, nxlConf.getRemoveDRMSrcFile());

			// log the uploaded file information
			// for debug purpose in the case that the protected file is missing
			if (this.m_zTaskLogger.isDebugEnabled()) {
				dmService.refreshObjects(new ModelObject[]{ locSourceDataset });
				ModelObject[] modelObjs = locSourceDataset.get_ref_list();
				
				dmService.getProperties(modelObjs, new String[] {
						"file_ext", "file_name", "original_file_name"});
				
				for (int i = 0; i < modelObjs.length; i++) {
					if (!(modelObjs[i] instanceof ImanFile)) {
		        		continue;
		        	}
								
					ImanFile imanFile = (ImanFile) modelObjs[i];
	
					this.m_zTaskLogger.info("    --> " + imanFile.get_file_name() + ", " + imanFile.get_file_ext() + ", " 
							+ imanFile.get_original_file_name() + ", " + imanFile.getUid());
				}
			}
			
			this.m_zTaskLogger.debug("Completed adding translated file to the source dataset");
			this.m_zTaskLogger.info("ToNxlFile exits DatabaseOperation->load()");
		} catch (Throwable localThrowable) {
			this.m_zTaskLogger.error("ToNxlFile caught exception: " + localThrowable.getMessage());
			throw new Exception(localThrowable.getMessage());
		} finally {
			// cancel explicit checkout for dataset
	    	// fix bug 45340
			this.m_zTaskLogger.info("ToNxlFile starts cancelCheckout for [" + locSourceDataset.getUid() + "]");
			moUtil.cancelCheckout(locSourceDataset);
			this.m_zTaskLogger.info("ToNxlFile ends cancelCheckout for [" + locSourceDataset.getUid() + "]");
		}
	}
	
	/**
	 * filter out NEXTLABS_tonxlfile.log & output.xml which are generated by framework in result folder
	 * @author clow
	 *
	 */
	class NXLResultFolderFilter implements FilenameFilter {
			
		NXLResultFolderFilter() {
		}

		public boolean accept(File paramFile, String paramString) {
			// fixes bug 38084, filter out the Temp folder during decryption
			// update and make it case sensitive
			
			if (paramString.equals("NEXTLABS_tonxlfile.log") || paramString.equals("output.xml") || 
					paramString.equals("Temp")) {
				return false;
			} else {
				return true;
			}
		}
		
	} // end of class NXLResultFolderFilter
	
	private List<String> readNamedRefFilename(String inputFilePath) {
		List<String> lstNamedRefFilename = new ArrayList<>();
		
		try (
				InputStreamReader isr = new InputStreamReader(new FileInputStream(inputFilePath), NextLabsConstants.CHARSET);
				BufferedReader br = new BufferedReader(isr);) {
			// line format: staging file name (*), named ref, iman original file name (*), iman file name (scrambled file name)
			// * same file name
			String readLine = br.readLine();
			while (readLine != null) {
				// fix bug 38138
				String[] fileInfos = readLine.split(SRC_LIST_DELIMITER);
				lstNamedRefFilename.add(fileInfos[1] + SRC_LIST_DELIMITER + fileInfos[2]);
				
				readLine = br.readLine();
			}
		} catch (IOException e) {
			this.m_zTaskLogger.error("ToNxlFile caught exception: " + e.getMessage(), e);
		}
		
		return lstNamedRefFilename;
	}
	
	// test UGPART
	private String getNamedRef(List<String> lstNamedRefFilename, String filename) {
		boolean scfBBSolution = false;
		
		if (nxlConf.getInstalledFeatures().containsKey(SUPPLIER_COLLABORATION_FOUNDATION) &&
				nxlConf.getInstalledFeatures().containsKey(BRIEFCASE_BROWSER)) {
			// removed Text
			scfBBSolution = true;
		}
		
		for (String namedRefFilename : lstNamedRefFilename) {
			// fix bug 38138
			String[] infos = namedRefFilename.split(SRC_LIST_DELIMITER);
			
			// to support update on nxl protected file
			String infosFilename = infos[1];
			if (!infos[1].endsWith(".nxl")) {
				infosFilename = infos[1] + ".nxl";
				this.m_zTaskLogger.debug("ToNxlFile getNamedRef() comparing " + infos[1] + "[.nxl] with " + filename);
			} else {
				this.m_zTaskLogger.debug("ToNxlFile getNamedRef() comparing " + infos[1] + " with " + filename);
			}
			
			if (infosFilename.equals(filename)) {
				// 30/03/2020, fixed bug 61076
				String object_type = null;
				try {
					object_type = locSourceDataset.get_object_type();
				} catch (NotLoadedException e) {
					this.m_zTaskLogger.error("ToNxlFile getNamedRef() caught exception: " + e.getMessage(), e);
					object_type = null;
				}
				
				if (object_type != null && integrationLookup.containsKey(object_type.toLowerCase())) {
					if (integrationLookup.get(object_type.toLowerCase()).containsNamedReference(infos[0])) {
						return infos[0];
					}
				}
				// end of 30/03/2020, fixed bug 61076
				
				if (integrationLookup.get(DSTYPE_ALL).containsNamedReference(infos[0])) {
					this.m_zTaskLogger.debug("ToNxlFile getNamedRef() Integration Named Reference List contains " + infos[0]);
					
					// handle the Dataset Text and NR Text separately
					// this section will be improved in 3.0
					if (infos[0].equals("Text")) {
						if (scfBBSolution)
							return NXL_NAMED_REFERENCE;
						
						if (object_type.equals("Text")) {
							return infos[0];
						}
						
						// else return nxl_named_reference
					} else {
						return infos[0];
					}
				} else {
					this.m_zTaskLogger.debug("ToNxlFile getNamedRef() Integration Named Reference List does not contain " + infos[0]);
				}
			}
		}
		
		return NXL_NAMED_REFERENCE;
	}
	
	private void removeSrcUnprotectedFile(List<String> lstNamedRefFilename) throws Exception {	
		dmService.getProperties(new ModelObject[] {locSourceDataset}, PROPS_DATASET_REV);
		
		this.m_zTaskLogger.debug("Source Dataset rev no. [" + locSourceDataset.get_revision_number() + "]");
		this.m_zTaskLogger.debug("Removing imanfile from source dataset");
		removeIMANFile(locSourceDataset, lstNamedRefFilename);
		this.m_zTaskLogger.debug("Completed removing imanfile from source dataset");
		
		ModelObject[] objs = locSourceDataset.get_revisions_prop();
		dmService.getProperties(objs, PROPS_DATASET);
		
		for (ModelObject obj : objs) {
			Dataset datasetRev = (Dataset) obj;
			
			this.m_zTaskLogger.debug("Removing imanfile from source dataset rev no. [" + datasetRev.get_revision_number() + "]");
			removeIMANFile(datasetRev, lstNamedRefFilename);
		}
	}
	
	private void removeIMANFile(Dataset dataset, List<String> lstNamedRefFilename) throws Exception {		
		dmService.getProperties(new ModelObject[] {dataset}, PROPS_DATASET);
		
		String[] lstRefName = dataset.get_ref_names();
		ModelObject[] lstRef = dataset.get_ref_list();
		dmService.getProperties(lstRef, PROPS_NAMED_REFERENCES);
		
		for (int i = 0; i < lstRef.length; i++) {
			if (!(lstRef[i] instanceof ImanFile)) {
        		continue;
        	}
						
			ImanFile imanFile = (ImanFile) lstRef[i];

			// fix bug 38138
			if (lstNamedRefFilename.contains(lstRefName[i] + SRC_LIST_DELIMITER + imanFile.get_original_file_name())) {
				this.m_zTaskLogger.debug("Removing namedreference [" + lstRefName[i] + ", " + 
						imanFile.get_original_file_name() + "] from source dataset rev");
				removeNamedReference(dataset, imanFile, lstRefName[i]);
				
				if (nxlConf.getPurgeRevSeq() && locSourceItemRev != null && 
						!imanFile.get_original_file_name().contains("." + NXL_FILE_EXT)) {
					// Purge
					this.m_zTaskLogger.debug("Purging source item rev starts");
					PurgeSequencesInfo purgeSeqInfo = new PurgeSequencesInfo();
					purgeSeqInfo.inputObject = locSourceItemRev;
					purgeSeqInfo.validateLatestFlag = true;
					
					ServiceData purgeSD = dmService.purgeSequences(new PurgeSequencesInfo[] {purgeSeqInfo});
					moUtil.logServiceData(purgeSD);
					
					this.m_zTaskLogger.debug("Purging source item rev completed");
				}
				
				this.m_zTaskLogger.debug("Deleting imanfile from source dataset rev");
				
				ServiceData serviceData = dmService.deleteObjects(new ModelObject[] {imanFile});
				moUtil.logServiceData(serviceData);
			}
		}
	}
	
	private void removeNamedReference(Dataset dataset, ImanFile imanFile, String nameRef) throws Exception {
		RemoveNamedReferenceFromDatasetInfo[] remNRFrmInfos = new RemoveNamedReferenceFromDatasetInfo[1];
		remNRFrmInfos[0] = new RemoveNamedReferenceFromDatasetInfo();
		remNRFrmInfos[0].dataset = dataset;
		
		try {
			NamedReferenceInfo namedRefInfo = new NamedReferenceInfo();
			namedRefInfo.type = nameRef;
			namedRefInfo.deleteTarget = true;
			namedRefInfo.targetObject = (ModelObject) imanFile;
			
			this.m_zTaskLogger.debug("Going to remove named reference: " + nameRef + ", " + imanFile.get_original_file_name());
			
			remNRFrmInfos[0].nrInfo = new NamedReferenceInfo[] {namedRefInfo};
		} catch (NotLoadedException nle) {
			this.m_zTaskLogger.error(nle.getMessage(), nle);
		}
		
		this.m_zTaskLogger.debug("Remove namedreference based on RemNRFrmInfos[0]");
		
		ServiceData serviceData = dmService.removeNamedReferenceFromDataset(remNRFrmInfos);
		moUtil.logServiceData(serviceData);
	}
	
}
