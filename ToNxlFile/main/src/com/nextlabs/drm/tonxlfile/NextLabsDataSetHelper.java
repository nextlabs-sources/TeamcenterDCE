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
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import com.nextlabs.drm.tonxlfile.configuration.NextLabsConstants;
import com.teamcenter.ets.soa.SoaHelper;
import com.teamcenter.ets.soa.TeamcenterServerProxy;
import com.teamcenter.ets.util.DataSetHelper;
import com.teamcenter.services.loose.core._2006_03.FileManagement.DatasetFileInfo;
import com.teamcenter.services.loose.core._2006_03.FileManagement.GetDatasetWriteTicketsInputData;
import com.teamcenter.services.strong.core._2007_01.DataManagement.VecStruct;
import com.teamcenter.services.strong.core._2007_06.DataManagement.DatasetTypeInfo;
import com.teamcenter.services.strong.core._2007_06.DataManagement.ReferenceInfo;
import com.teamcenter.soa.client.FileManagementUtility;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.Dataset;
import com.teamcenter.soa.client.model.strong.DatasetType;
import com.teamcenter.soa.client.model.strong.ImanFile;
import com.teamcenter.soa.client.model.strong.ImanVolume;
import com.teamcenter.tstk.util.log.ITaskLogger;

public class NextLabsDataSetHelper extends DataSetHelper {
	
	private static final boolean IS_NXL_PROTECTED_SET = true;
	
	private ImanVolume srcDatasetVol = null;
	private ModelObjectUtil moUtil;
	private boolean removeSrcFlag = true;

	public NextLabsDataSetHelper(ITaskLogger zTaskLogger, boolean updateExistingVisData, 
			boolean qSoftFailureHandlingEnabled, ModelObjectUtil moUtil) {
		super(zTaskLogger, updateExistingVisData, qSoftFailureHandlingEnabled);
		setMoUtil(moUtil);
	}
	
	public void setMoUtil(ModelObjectUtil moUtil) {
		this.moUtil = moUtil;
	}
	
	public void cacheSourceVolume(Dataset sourceDataset) throws Exception {
    	sourceDataset = (Dataset) SoaHelper.getProperties(sourceDataset, "ref_list");
    	ModelObject[] files = sourceDataset.get_ref_list();
    	
    	for (ModelObject modelObject : files) {
    		if (modelObject instanceof ImanFile) {
    			modelObject = SoaHelper.getProperties(modelObject, "volume_tag");
    			ImanVolume zSrcDatasetVol = ((ImanFile) modelObject).get_volume_tag();
    			zSrcDatasetVol = (ImanVolume) SoaHelper.getProperties(zSrcDatasetVol, "volume_name");
    			
    			srcDatasetVol = zSrcDatasetVol;
    			
    			this.m_zTaskLogger.debug("\tset srcDatasetVol=" + srcDatasetVol.get_volume_name());
    			break;
    		}
    	} // end of the loop for files
	}
	
	public boolean isAllNRFileTypeValid(Dataset dataset, List<String> namedReferenceList, 
			String resultsDir, List<String> fileList) throws Exception {
		this.m_zTaskLogger.debug("  isAllNRFileTypeValid() begin...");
		
		boolean isAllNRFileTypeValid = true;
	    
	    // setting up the lookup table for reference info
		Map<String, List<ReferenceInfo>> refInfoLookup = new HashMap<>();
	    int refInfoCount = setupRefInfoLookup(dataset, refInfoLookup);
	    
	    int iCount = fileList.size();
	    String[] filePathnames = new String[iCount];
	    
	    // looping through file list
	    for (int i = 0; i < iCount; i++) {
	    	String visFileName = fileList.get(i);
	    	String inNR = namedReferenceList.get(i);
	    	
	    	// only when incoming namedRef is 
	    	List<String> inFileNRList = new ArrayList<>();
	    	
	    	// using this variable to differentiate action unprotect from protect & update
	    	boolean isNXLFile = false;
	    	
	    	if (visFileName.regionMatches(0, File.separator, 0, 1)) {
	    		filePathnames[i] = (resultsDir + visFileName);
	    	} else {
	    		filePathnames[i] = (resultsDir + File.separator + visFileName);
	    	}
	    	
	    	this.m_zTaskLogger.trace("\tfilePathnames[" + i + "]=" + filePathnames[i], 0);
	    	
	    	// fix bug 48714
	    	String inExtension = ""; // handle the case that file has no extension
	    	if (visFileName.lastIndexOf('.') > 0)
	    		inExtension = visFileName.substring(visFileName.lastIndexOf('.'));
	    		    	
	    	if (inExtension.equalsIgnoreCase("." + NextLabsConstants.NXL_FILE_EXT)) {
	    		isNXLFile = true;
	    	}

	    	// does not need to compare with incoming named ref if the file is not NXL protected
			// for unprotect case
	    	if (isNXLFile && !refInfoLookup.isEmpty()) {
				this.m_zTaskLogger.trace("\tfilePathnames[" + i + "] refInfoCount=" + refInfoCount, 0);
				
				// search for actual match
				if (refInfoLookup.containsKey(inExtension.toLowerCase())) {
					List<ReferenceInfo> refInfos = refInfoLookup.get(inExtension.toLowerCase());
					
					for (ReferenceInfo refInfo : refInfos) {
						this.m_zTaskLogger.trace("\tfilePathnames[" + i + "] inExt=" + inExtension + "; refExt=" + 
								refInfo.fileExtension + "; inNR=" + inNR + "; refNR=" + refInfo.referenceName, 0);
						
						inFileNRList.add(refInfo.referenceName);
					}
				}
				
				// if there is no actual match
				// search for *
				// NR Text always look for *
				if (refInfoLookup.containsKey(".*")) {
					List<ReferenceInfo> refInfos = refInfoLookup.get(".*");
					
					for (ReferenceInfo refInfo : refInfos) {
						this.m_zTaskLogger.trace("\t(.*) filePathnames[" + i + "] inExt=" + inExtension + "; refExt=" + 
								refInfo.fileExtension + "; inNR=" + inNR + "; refNR=" + refInfo.referenceName, 0);
						
						if (inExtension.equalsIgnoreCase("." + NextLabsConstants.NXL_FILE_EXT) &&
								inNR.equalsIgnoreCase("Text") &&
								refInfo.referenceName.equalsIgnoreCase("Text") && 
								!refInfo.fileFormat.equalsIgnoreCase("BINARY")) {
							// skip
							// translation will be terminal
						} else {
							inFileNRList.add(refInfo.referenceName);
						}
					}
				}
	    	}
	    	
    		if (isNXLFile) {
    			// protected file, the named ref shall be same with the named ref fetched from the model
    			if (inFileNRList.contains(inNR)) {
    				isAllNRFileTypeValid = true;
    			} else {
    				isAllNRFileTypeValid = false;
    				break;
    			}
    		} else {
    			// does not need to compare with incoming named ref if the file is not NXL protected
    			// for unprotect case
    			isAllNRFileTypeValid = true;
    		}
	    } // end of the loop for filelist
	    
	    this.m_zTaskLogger.debug("  isAllNRFileTypeValid() end");
	    return isAllNRFileTypeValid;
	}
	
	// fix bug 45118
	public void addNRFiles(Dataset dataset, List<String> namedReferenceList, String resultsDir, 
			List<String> fileList, Dataset sourceDataset, boolean removeSrcFlag) throws Exception {
		this.removeSrcFlag = removeSrcFlag;
		addNRFiles(dataset, namedReferenceList, resultsDir, fileList, sourceDataset);
	}
	
	// override the original addNRFiles
	// - set the value for clientId to solve the error when there is multiple files uploaded at one time.
	public void addNRFiles(Dataset dataset, List<String> namedReferenceList, String resultsDir, 
			List<String> fileList, Dataset sourceDataset) throws Exception {
		this.m_zTaskLogger.trace("  addNRFiles() begin...", 0);
		
		// setting up the lookup table for reference info
		Map<String, List<ReferenceInfo>> refInfoLookup = new HashMap<>();
	    setupRefInfoLookup(dataset, refInfoLookup);
	    
	    int iCount = fileList.size();
	    TranslatedFile[] translatedFiles = new TranslatedFile[iCount];
	    	    
	    boolean isAllNonNXLProtected = true;
	    
	    for (int i = 0; i < iCount; i++) {
	    	String visFileName = fileList.get(i);
	    	String inNR = namedReferenceList.get(i);
	    	
	    	String outFileName;
	    	String outFileFormat;
	    	String outNamedReference;
	    	
	    	String refFileFormat = "BINARY";
	    	String refNR = null;
	    	boolean isNXLFile = false;		// using this variable to differentiate action unprotect from protect & update
	    	
	    	if (visFileName.regionMatches(0, File.separator, 0, 1)) {
	    		outFileName = (resultsDir + visFileName);
	    	} else {
	    		outFileName = (resultsDir + File.separator + visFileName);
	    	}
	    	
	    	this.m_zTaskLogger.trace("\tfilePathnames[" + i + "]=" + outFileName, 0);
	    	
	    	// fix bug 48714
	    	String inExtension = "";
	    	if (visFileName.lastIndexOf('.') > 0)
	    		inExtension = visFileName.substring(visFileName.lastIndexOf('.'));
	    	
	    	if (inExtension.equalsIgnoreCase("." + NextLabsConstants.NXL_FILE_EXT)) {
	    		isAllNonNXLProtected = false;
	    		isNXLFile = true;
	    	}

			filextmatching:
	    	if (!refInfoLookup.isEmpty()) {
	    		// search for actual match
	    		if (refInfoLookup.containsKey(inExtension.toLowerCase())) {
					List<ReferenceInfo> refInfos = refInfoLookup.get(inExtension.toLowerCase());
					
					for (ReferenceInfo refInfo : refInfos) {
						this.m_zTaskLogger.trace("\tfilePathnames[" + i + "] inExt=" + inExtension + "; refExt=" + 
								refInfo.fileExtension + "; inNR=" + inNR + "; refNR=" + refInfo.referenceName, 0);
						
						refFileFormat = refInfo.fileFormat;
						refNR = refInfo.referenceName;
						
						break filextmatching;
					}
				}
				
				// if there is no actual match
				// search for *
				if (refInfoLookup.containsKey(".*")) {
					List<ReferenceInfo> refInfos = refInfoLookup.get(".*");
					
					for (ReferenceInfo refInfo : refInfos) {
						this.m_zTaskLogger.trace("\t(.*) filePathnames[" + i + "] inExt=" + inExtension + "; refExt=" + 
								refInfo.fileExtension + "; inNR=" + inNR + "; refNR=" + refInfo.referenceName, 0);
						
						if (inExtension.equalsIgnoreCase("." + NextLabsConstants.NXL_FILE_EXT) &&
								inNR.equalsIgnoreCase("Text") &&
								refInfo.referenceName.equalsIgnoreCase("Text") && 
								!refInfo.fileFormat.equalsIgnoreCase("BINARY")) {
							// skip
							// translation will be terminal
						} else {
							refFileFormat = refInfo.fileFormat;
							refNR = refInfo.referenceName;
							
							break filextmatching;
						}
					}
				}
	    	}
	    	
	    	outFileFormat = refFileFormat;
	    	if (!isNXLFile && inNR.equalsIgnoreCase(NextLabsConstants.NXL_NAMED_REFERENCE) && 
	    			refNR != null) {
	    		outNamedReference = refNR;
	    	} else {
	    		outNamedReference = inNR;
	    	}
	    	
	    	translatedFiles[i] = new TranslatedFile(outFileName, outFileFormat, outNamedReference, isNXLFile);
	    }
	    
	    // setting bypass
	    moUtil.setBypass();
	    
	    this.m_zTaskLogger.info("\tSet last modified user & date");
        try {
        	setLastModNXL(dataset, resultsDir);
        } catch (Exception ex) {
        	this.m_zTaskLogger.error("\tNextLabsDataSetHelper.setLastModNXL() caught exception: " + ex.getMessage(), ex);
        }
        
	    this.m_zTaskLogger.debug("\tStoring files into source data volume ==> " + 
	    		this.m_qStoreInSrcVol + "; " + (srcDatasetVol != null));
		if (this.m_qStoreInSrcVol && srcDatasetVol != null) {
			this.m_zTaskLogger.debug("\tStoring files into source data volume " + srcDatasetVol.get_volume_name());
			this.m_zTempVol = TeamcenterServerProxy.getInstance().getSessionVol();
			TeamcenterServerProxy.getInstance().setSessionVol(srcDatasetVol);
    	}
		
	    try {
	    	dataset = (Dataset) SoaHelper.getProperties(dataset, "revision_number");
	    	int version = dataset.get_revision_number();
	    	this.m_zTaskLogger.debug("\tsetting files on " + dataset.get_object_string() + ";" + version + " ...");
	    	DatasetFileInfo[] azDFI = new DatasetFileInfo[translatedFiles.length];
	    				
	    	for (int i = 0; i < translatedFiles.length; i++) {
	    		this.m_zTaskLogger.debug("   fileName[" + i + "]       " + translatedFiles[i].getFileName());
	    		this.m_zTaskLogger.debug("   fileFormat[" + i + "]     " + translatedFiles[i].getFileFormat());
	    		this.m_zTaskLogger.debug("   namedReference[" + i + "] " + translatedFiles[i].getNamedReference());
	    		
	    		azDFI[i] = new DatasetFileInfo();
	    		azDFI[i].allowReplace = this.m_qUpdateExistingVisData;
	    		azDFI[i].fileName = translatedFiles[i].getFileName();
	    		azDFI[i].clientId = "clientId" + i;
	    		
	    		if (translatedFiles[i].getFileFormat().equalsIgnoreCase("BINARY")) {
	    			azDFI[i].isText = false;
	    		} else {
	    			azDFI[i].isText = true;
	    		}
	    		
	    		azDFI[i].namedReferencedName = translatedFiles[i].getNamedReference();
	    		// fix bug 45118
	    		// fix bug 53542
	    		if (!removeSrcFlag && translatedFiles[i].isNXLFile()) {
	    			this.m_zTaskLogger.debug("   overwrites namedReferences[" + i + "]  " + NextLabsConstants.NXL_NAMED_REFERENCE);
	    			azDFI[i].namedReferencedName = NextLabsConstants.NXL_NAMED_REFERENCE;
	    		}
	    	}
	    	
	    	GetDatasetWriteTicketsInputData[] arg0 = new GetDatasetWriteTicketsInputData[1];
	    	arg0[0] = new GetDatasetWriteTicketsInputData();
	    	arg0[0].createNewVersion = false;
	    	arg0[0].dataset = dataset;
	    	arg0[0].datasetFileInfos = azDFI;

	    	FileManagementUtility fmUtility = new FileManagementUtility(SoaHelper.getSoaConnection());
	    	ServiceData serviceData = fmUtility.putFiles(arg0);
	    	moUtil.logServiceData(serviceData);

	    	// switch on the flag to indicated the dataset is protected by NXL			
			if (IS_NXL_PROTECTED_SET) {
				setIsNXLProtected(sourceDataset, isAllNonNXLProtected);
			}
			// end of the swithing on section

	    	this.m_zTaskLogger.debug("\tFiles attached to dataset through named references");
	    } catch (Exception e) {
	    	this.m_zTaskLogger.error("addNRFiles Exception - " + e.toString(), e);
	    	throw e;
	    } finally {
	    	this.m_zTaskLogger.debug("Restoring volume to default");
	    	restoreTCSessionVolume();
	    	
	    	moUtil.unsetBypass();
	    }

	    this.m_zTaskLogger.trace("  addNRFiles() end", 0);
	}
	
	private void setIsNXLProtected(Dataset sourceDataset, boolean allNxlProtected) throws Exception {
		Map<String, VecStruct> propertiesToUpdate = new HashMap<>();
		VecStruct vec1 = new VecStruct();
		
		if (allNxlProtected)
			vec1.stringVec = new String[] {"false"};
		else
			vec1.stringVec = new String[] {"true"};
		
		try {
			propertiesToUpdate.put(NextLabsConstants.NXL_IS_NXL_PROTECTED, vec1);
			//propertiesToUpdate.put(NextLabsConstants.FORMAT_USED, vec2);
			
			SoaHelper.setProperties(new ModelObject[] {sourceDataset}, propertiesToUpdate);
		} catch (Exception ex) {
			this.m_zTaskLogger.error("\t\tNextLabsDataSetHelper.setIsNXLProtected() caught Exception: " + 
					ex.getMessage(), ex);
		}
	}
	
	private void setLastModNXL(Dataset sourceDataset, String resultDir) throws Exception {
		// read from file
		String lastModUser = null;
		String lastModDate = null;
		
		File resultFolder = new File(resultDir);		
		List<String> content = new ArrayList<>();
		
		try (
				FileInputStream fis = new FileInputStream(
				resultFolder.getParent() + File.separator + NextLabsConstants.TRANSIENT_FILENAME); 
				InputStreamReader isr = new InputStreamReader(fis);
				BufferedReader br = new BufferedReader(isr);) {
			
			// last_mod_user, last_mod_date
			String readLine = br.readLine();
			while (readLine != null && content.size() < 2) {
				content.add(readLine);
				
				readLine = br.readLine();
			}
			
			if (content.size() >= 2) {
				lastModUser = content.get(0);
				lastModDate = content.get(1);
			}
		} catch (IOException e) {
			this.m_zTaskLogger.error("\t\tReading from transient data file caught exception: " + 
					e.getMessage(), e);
		}
		
		if (lastModUser == null || lastModDate == null)
			throw new Exception("Failed to read metadata from the transient data file");
		
		this.m_zTaskLogger.debug("\t\tlast_mod_user: " + lastModUser);
		this.m_zTaskLogger.debug("\t\tlast_mod_date: " + lastModDate);
		
		// set data to dataset		
		Map<String, VecStruct> propertiesToUpdate = new HashMap<>();
		VecStruct vec1 = new VecStruct();
		VecStruct vec2 = new VecStruct();
		
		try {
			vec1.stringVec = new String[] {lastModUser};
			vec2.stringVec = new String[] {lastModDate};
			
			propertiesToUpdate.put(NextLabsConstants.NXL_LAST_MOD_USER, vec1);
			propertiesToUpdate.put(NextLabsConstants.NXL_LAST_MOD_DATE, vec2);
			
			SoaHelper.setProperties(new ModelObject[] {sourceDataset}, propertiesToUpdate);
		} catch (Exception ex) {
			this.m_zTaskLogger.error("\t\tNextLabsDataSetHelper.setLastModNXL() caught Exception: " + 
					ex.getMessage(), ex);
		}
	}
	
	private int setupRefInfoLookup(Dataset dataset, 
			Map<String, List<ReferenceInfo>> refInfoLookup) throws Exception {
		// retrieving dataset type information: named reference, file type
	    dataset = (Dataset) SoaHelper.getProperties(dataset, "dataset_type");
	    DatasetType zDsType = dataset.get_dataset_type();
	    
	    zDsType = (DatasetType) SoaHelper.getProperties(zDsType, "datasettype_name");
	    String srcDatasetTyp = zDsType.get_datasettype_name();
	    
	    DatasetTypeInfo[] zInfos = SoaHelper.getDatasetTypeInfo(srcDatasetTyp);
	    ReferenceInfo[] listOfNamedRefContext = null;
	    if (zInfos.length > 0) {
	    	listOfNamedRefContext = zInfos[0].refInfos;
	    }
	    
	    if (listOfNamedRefContext == null)
	    	return 0;
		
		// setting up the lookup table for reference info
    	for (int i = 0; i < listOfNamedRefContext.length; i++) {
    		ReferenceInfo refInfo = listOfNamedRefContext[i];
    		List<ReferenceInfo> refInfos;
    		
    		String fileExt = refInfo.fileExtension;
    		int iIndex = fileExt.lastIndexOf(".");
			if (iIndex > 0)
				fileExt = fileExt.substring(iIndex);
			
			// standardize * (Text) and .* (Image) to .*
			if (fileExt.equalsIgnoreCase("*"))
				fileExt = ".*";
    		
    		if (refInfoLookup.containsKey(fileExt.toLowerCase())) {
    			refInfos = refInfoLookup.get(fileExt.toLowerCase());
    		} else {
    			refInfos = new ArrayList<>();
    		}
    		
			refInfos.add(refInfo);	
    		refInfoLookup.put(fileExt.toLowerCase(), refInfos);
    	}
    	
    	if (this.m_zTaskLogger.isDebugEnabled()) {
	    	Iterator<Entry<String, List<ReferenceInfo>>> ite = refInfoLookup.entrySet().iterator();
	    	this.m_zTaskLogger.debug("debug refInfoLookup content ");
	    	while (ite.hasNext()) {
	    		Entry<String, List<ReferenceInfo>> entry = ite.next();
	    		List<ReferenceInfo> data = entry.getValue();
	    		this.m_zTaskLogger.debug(entry.getKey() + " : " + data.size());
	    		for (int z = 0; z < data.size(); z++) {
	    			this.m_zTaskLogger.debug(entry.getKey() + " => " + data.get(z).referenceName);
	    		}
	    	}
    	}
    	
    	return listOfNamedRefContext.length;
	}

}
