package com.nextlabs.drm.tonxlfile;

/**
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;

import com.nextlabs.drm.tonxlfile.configuration.NextLabsConfigInterface;
import com.nextlabs.drm.tonxlfile.configuration.NextLabsConfigManager;
import com.nextlabs.drm.tonxlfile.policy.NextLabsAuthorizationAgent;
import com.nextlabs.drm.tonxlfile.policy.NextLabsPolicyConstants;
import com.teamcenter.ets.extract.DefaultTaskPrep;
import com.teamcenter.ets.request.TranslationRequest;
import com.teamcenter.ets.soa.ConnectionManager;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.Property;
import com.teamcenter.soa.client.model.strong.Dataset;
import com.teamcenter.soa.client.model.strong.ImanFile;
import com.teamcenter.soa.client.model.strong.ItemRevision;
import com.teamcenter.soa.client.model.strong.POM_system_class;
import com.teamcenter.soa.client.model.strong.WorkspaceObject;
import com.teamcenter.soa.exceptions.NotLoadedException;
import com.teamcenter.soa.internal.client.model.PropertyBoolImpl;
import com.teamcenter.translationservice.task.Option;
import com.teamcenter.translationservice.task.TranslationDBMapInfo;
import com.teamcenter.translationservice.task.TranslationTask;
import com.teamcenter.translationservice.task.TranslationTaskUtil;
import com.teamcenter.translationservice.task.TranslatorOptions;
import static com.nextlabs.drm.tonxlfile.configuration.NextLabsConstants.*;

public class TaskPrep extends DefaultTaskPrep {
	
	private static NextLabsConfigInterface nxlConf;
	private static boolean isWindows;
	private static NextLabsAuthorizationAgent agent;
	
	private DataManagementService dmService;
	private ModelObjectUtil moUtil;
	
	static {
		String osname = System.getProperty("os.name");
		
		if (osname != null && osname.toLowerCase().indexOf("win") < 0) {
			isWindows = false;
		} else {
			isWindows = true;
		}

		// fixing the connection lost issues after long-time running
		try {
			NextLabsConfigManager nxlConfigMgr = new NextLabsConfigManager();
			
			nxlConf = nxlConfigMgr.getService(ConnectionManager.getActiveConnection());
			
			agent = NextLabsAuthorizationAgent.getInstance(nxlConf.getPdpHost(), nxlConf.getPdpPort(), 
						nxlConf.getPdpHttps(), nxlConf.getPdpIgnoreHttps(), nxlConf.getOAuth2Host(),
						nxlConf.getOAuth2Port(), nxlConf.getOAuth2Https(), nxlConf.getOAuth2ClientID(),
						nxlConf.getOAuth2ClientSecret(),
						nxlConf.getPdpDefaultAction(), nxlConf.getPdpDefaultMessage());
		} catch (Exception ex) {
			nxlConf = null;
			agent = null;
		}
	}
	
	public TranslationTask prepareTask() throws Exception {
		this.m_zTaskLogger.info("ToNxlFile entering TaskPrep->prepareTask()");
		
		dmService = DataManagementService.getService(ConnectionManager.getActiveConnection());
		moUtil = new ModelObjectUtil(this.m_zTaskLogger);
		
		boolean isCheckout = false;
		
		// fixing the connection lost issues after long-time running
		try {
			NextLabsConfigManager nxlConfigMgr = new NextLabsConfigManager();
			nxlConf = nxlConfigMgr.getService(ConnectionManager.getActiveConnection());
		} catch (Exception ex) {
			nxlConf = null;
		}
		
		if (nxlConf == null) {
			this.m_zTaskLogger.error("ToNxlFile failed to instantiate the config service");
			
			throw new Exception("ToNxlFile failed to instantiate the config service");
		}
		
		String scDatasetString = null;
    	String scItemRevString = null;
        TranslationDBMapInfo zDbMapInfo = new TranslationDBMapInfo();
        
    	ModelObject primary_objs[] = 
    			request.getPropertyObject(PRIMARYOBJECTS).getModelObjectArrayValue();
    	ModelObject secondary_objs[] = 
    			request.getPropertyObject(SECONDARYOBJECTS).getModelObjectArrayValue();
    	
    	List<String> lstArgumentKeys = request.getPropertyDisplayableValues(ARGUMENTKEYS);
    	List<String> lstArgumentData = request.getPropertyDisplayableValues(ARGUMENTDATA);
    	
    	// retrieving revision's properties
    	ItemRevision itemRev = (secondary_objs.length <= 0 || secondary_objs[0] == null) ? null : (ItemRevision) secondary_objs[0];    	
    	Dataset dataset = (Dataset) primary_objs[0];
    	
    	// if catches any exception, cancel the checkout
    	try {
	    	scItemRevString = (itemRev == null) ? "" : itemRev.getUid();
	    	scDatasetString = dataset.getUid();
	    	
	    	// DEBUG TEST
			dmService.refreshObjects(new ModelObject[] {dataset});
	    	
	    	// retrieving dataset's properties
	    	// fix the case where metadata from checkin handler, last mod fields are outdated
	    	dmService.getProperties(new ModelObject[] {dataset}, PROPS_DATASET_TASKPREP);
	    	String[] lstRefName = dataset.get_ref_names();
	        ModelObject[] lstRef = dataset.get_ref_list();
	        
	        // explicit checkout for dataset
	    	// fix bug 45340
	    	this.m_zTaskLogger.info("ToNxlFile starts checkout for [" + dataset.getUid() + "]");
	    	isCheckout = moUtil.checkout(dataset, NXL_CHECKOUT_MSG, NXL_CHANGEID); 
	    	if (!isCheckout) {
	    		throw new Exception("ToNxlFile failed to perform " + NXL_CHANGEID + " explicitly checkout on the [" + dataset.getUid() + "]");
	    	} else {
	    		this.m_zTaskLogger.info("ToNxlFile checkout the [" + dataset.getUid() + "]");
	    	}
	    	this.m_zTaskLogger.info("ToNxlFile ends checkout for [" + dataset.getUid() + "]");
	        
	        // retrieving references's properties
			dmService.getProperties(lstRef, PROPS_NAMED_REFERENCES);
			
			// setup the nxlaction, default is protect
			// nxlaction is optional, this is to support backward compatible
			String nxlAction = ACT_PROTECT;	// default action is protect
			if (lstArgumentKeys.contains(ARG_NXL_ACTION)) {			
				int index = lstArgumentKeys.indexOf(ARG_NXL_ACTION);
				
				if (index > -1) {
					nxlAction = lstArgumentData.get(index);
					
					// remove the nxlaction from the argument list
					lstArgumentKeys.remove(index);
					lstArgumentData.remove(index);
				}
			}
			
			// 3.5 feature, read the client's request context and log it
			if (lstArgumentKeys.contains(ARG_NXL_REMARK)) {
				int index = lstArgumentKeys.indexOf(ARG_NXL_REMARK);
				
				if (index > -1) {
					this.m_zTaskLogger.debug("ToNxlFile client's request context: " + lstArgumentData.get(index));
					
					lstArgumentKeys.remove(index);
					lstArgumentData.remove(index);
				}
			}
			
			// evaluate access for ACT_UNPROTECT
			if (nxlAction.equalsIgnoreCase(ACT_REMOVEPROTECTION)) {
				this.m_zTaskLogger.info("*** POLICY EVALUATION ***");
				String[] evaluationAttrs = nxlConf.getEvaluationAttr();
				dmService.getProperties(new ModelObject[] {dataset}, evaluationAttrs);
				String pc_owning_user = "N/A";
				if (request.getPropertyObject(NextLabsPolicyConstants.PC_OWNING_USER) != null) {
					ModelObject moUser = request.getPropertyObject(NextLabsPolicyConstants.PC_OWNING_USER).getModelObjectValue();
					dmService.getProperties(new ModelObject[] {moUser}, new String[] {"userid"});
					pc_owning_user = moUser.getPropertyDisplayableValue("userid");
				}
				
				HashMap<String, Object> attributes = new HashMap<>();
				
				for (String attr: evaluationAttrs) {
					try {
						Property property = dataset.getPropertyObject(attr);
						// need to handle multiple values
						if (property != null && property.getDisplayableValues() != null && 
								property.getDisplayableValues().size() > 0) {
							if (property.getDisplayableValues().size() == 1)
								attributes.put(attr, property.getDisplayableValues().get(0));
							else if (property.getDisplayableValues().size() > 1)
								attributes.put(attr, property.getDisplayableValues());
						}
					} catch (NotLoadedException nle) {
						// nop
					}
				}
								
				if (itemRev != null) {
					// fix bug 65218
					dmService.getProperties(new ModelObject[] {itemRev}, new String[] {"object_name"});
					
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_REV_TYPE, itemRev.getTypeObject().getName());
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_REV_NAME, itemRev.get_object_name());
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_REV_UID, scItemRevString);
				}
				
				if (dataset != null) {
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_DS_TYPE, dataset.getTypeObject().getName());
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_DS_NAME, dataset.get_object_name());
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_DS_UID, scDatasetString);
				}
				
				HashMap<String, Object> hasAccessResult = agent.hasAccess(pc_owning_user, 
						NextLabsPolicyConstants.RIGHT_EXECUTE_REMOVEPROTECTION, attributes, 
						new HashMap<String, Object>(), new HashMap<String, Object>());
				
				String response = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_KEY);
				String sError = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_ERROR_KEY);
				
				// Checking to see if there is SDK error response
				if (null != sError) {
					String sDefaultMsg = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_MESSAGE_KEY);
					
					if (null != sDefaultMsg) {
						this.m_zTaskLogger.info("CE SDK error: " + sDefaultMsg);
					}
				}
				
				// fix bug 46950
				Integer obligationCount = (Integer) hasAccessResult.get(NextLabsPolicyConstants.CE_ATTR_OBLIGATION_COUNT);
				boolean obligationRemoveProtection = false;
				
				if (obligationCount != null) {
					int count = obligationCount.intValue();
					
					for (int i = 1; i <= count; i++) {
						String obligationName = (String) 
								hasAccessResult.get(NextLabsPolicyConstants.CE_ATTR_OBLIGATION_NAME + ":" + i);
						
						if (obligationName != null && 
								obligationName.equalsIgnoreCase(NextLabsPolicyConstants.OBLIGATION_REMOVEPROTECTION)) {
							obligationRemoveProtection = true;
							break;
						}
					}
				}
				
				// Change to display message even if allow obligation	
				this.m_zTaskLogger.info("Response is " + response + " for request from user " + pc_owning_user + 
						" on uid " + dataset.getUid());
				
				if (!(NextLabsPolicyConstants.RESPONSE_ALLOW_VALUE).equalsIgnoreCase(response) || 
						!obligationRemoveProtection)
					throw new Exception("Remove protection is denied for " + pc_owning_user + " on uid " + dataset.getUid());
			}
			
			// fix the case where metadata from checkin handler, last mod fields are outdated
			// update the metadata fields to the latest last mod user & date
			if (lstArgumentKeys.contains(NXL_LAST_MOD_DATE) || 
					lstArgumentKeys.contains(NXL_LAST_MOD_USER)) {
				int indexDate = lstArgumentKeys.indexOf(NXL_LAST_MOD_DATE);
				int indexUser = lstArgumentKeys.indexOf(NXL_LAST_MOD_USER);
				String dcUid = ConnectionManager.getLoggedInUser().getUid();
				
				if (indexDate > -1) {
					try {
						List<String> values = dataset.getPropertyDisplayableValues("last_mod_date");
		    			
		    			String value = values.toString();
		    			value = StringUtils.stripStart(value, "[");
		    			value = StringUtils.stripEnd(value, "]");
		    			
		    			if (!dataset.get_last_mod_user().getUid().equals(dcUid) || lstArgumentKeys.get(indexDate).trim().equals("")) {
		    				lstArgumentData.set(indexDate, value);
		    			}
					} catch (NotLoadedException nle) {
						this.m_zTaskLogger.error("ToNxlFile caught NotLoadedException for last_mod_date");
					}
				}
				
				if (indexUser > -1) {
					try {
						List<String> values = dataset.getPropertyDisplayableValues("last_mod_user");
		    			
		    			String value = values.toString();
		    			value = StringUtils.stripStart(value, "[");
		    			value = StringUtils.stripEnd(value, "]");
						
		    			if (!dataset.get_last_mod_user().getUid().equals(dcUid) || lstArgumentKeys.get(indexUser).trim().equals("")) {
		    				lstArgumentData.set(indexUser, value);
		    			}
					} catch (NotLoadedException nle) {
						this.m_zTaskLogger.error("ToNxlFile caught NotLoadedException for last_mod_user");
					}
				}
			}
			
	        
			// prepare the file to be translated
			// move the file to staging
	        StringBuffer filesToTranslate = prepareFileToTranslate(lstRefName, lstRef, nxlAction);        
	        
	        // Write the file list to file
	        File fileList = createInputFile(filesToTranslate, SRC_LIST_FILENAME);
	        
	        TranslationTaskUtil.setUserAttrValue(zDbMapInfo, "priobjuid", scDatasetString);
	        if (!scItemRevString.trim().equals("")) {
	        	TranslationTaskUtil.setUserAttrValue(zDbMapInfo, "secobjuid", scItemRevString);
	        }
	        
	        TranslationTask translationtask = addRefIdToTask(
	        		prepTransTask(null, null, null, fileList.getName(), false, false, null, 0, null), 0);
	        
	        for (int i = 0; i < translationtask.getTranslationData().length; i++) {
	        	this.m_zTaskLogger.info("ToNxlFile Setting zDbMapInfo on TranslationData");
	        	translationtask.getTranslationData()[i].setTranslationDBMapInfo(zDbMapInfo);
	        }
	        
	        // updated to comply with 2.0 translator
	        this.m_zTaskLogger.debug(INPUT_PATH + ": " + fileList.getParent());
	        this.m_zTaskLogger.debug(INPUT_FILENAME + ": " + fileList.getName());
	        this.m_zTaskLogger.debug(OUTPUT_PATH + ": " + fileList.getParent() + File.separator + "result");
	        
	        // set translation task option
	        // basic parameters for translation task: input path, input file name, output path
	        Option optInputPath = new Option();
	        Option optInputFilename = new Option();
	        Option optOutputPath = new Option();
	        Option optNxlAction = new Option();
	        Option optArgsFilename = new Option();
	        
	        // updated to comply with 2.0 translator
	        optInputPath.setName(INPUT_PATH);
	        optInputPath.setValue(fileList.getParent());
	        
	        optInputFilename.setName(INPUT_FILENAME);
	        optInputFilename.setValue(fileList.getName());
	        
	        optOutputPath.setName(OUTPUT_PATH);
	        optOutputPath.setValue(fileList.getParent() + File.separator + "result");
	        
	        optNxlAction.setName(NXL_ACTION);
	        optNxlAction.setValue(nxlAction);
	        
	        optArgsFilename.setName(INPUT_ARGS_FILENAME);
			if (!isWindows) {
        		optArgsFilename.setValue("argsFile=");
			} else {
				optArgsFilename.setValue("\"argsFile=\"");
			}
	        
	        // setting up translation options which includes: file name, file path, tag name and value
	        TranslatorOptions translationOptions = new TranslatorOptions();
	        translationOptions.addOption(optInputPath);
	        translationOptions.addOption(optInputFilename);
	        translationOptions.addOption(optOutputPath);
	        translationOptions.addOption(optNxlAction);
	        
	        
	        // parameters from client, i.e. RAC
	        Option[] req_tags = new Option[lstArgumentKeys.size()];
	        
	        // start from 2, 1 is reserved for default tag nxl_encrypted
	        int tagIndex = 1;
	        for (int i = 0; i < lstArgumentKeys.size(); i++) {
	        	req_tags[i] = new Option();
	        	req_tags[i].setName(TAG_PREFIX + tagIndex);
				if (!isWindows) {
	        		req_tags[i].setValue(lstArgumentKeys.get(i) + "=" + lstArgumentData.get(i));
    	    	} else {
	        		req_tags[i].setValue("\"" + lstArgumentKeys.get(i) + "=" + lstArgumentData.get(i) + "\"");
    	    	}
	        	
	        	tagIndex++;
	        }
	        
	        this.m_zTaskLogger.info("ToNxlFile save last modified user & date");
	        Map<String, String> transientData = null;
	        try {
	        	transientData = saveLastModNXL(dataset);
	        } catch (Exception ex) {
	        	this.m_zTaskLogger.error("TaskPrep.saveLastModNXL() caught exception: " + ex.getMessage(), ex);
	        }
	        
	        // 1.5
	        // if req_tags contains data, ignore runtime_tags
	        // for the case the tags are coming from workflow obligation
	        StringBuffer sbOptions;
	    	File argsFile;
	    	
	        if (req_tags.length > 0) {
	        	// 10 metadata tags is configured in the translator.xml
	        	if (req_tags.length < TRANSLATOR_NOOFTAGS_CONFIGURED) {
	        		addOptions(translationOptions, req_tags);
	        	} else {
	        		sbOptions = buildSBFromOptions(req_tags);
	        		
	        		argsFile = createInputFile(sbOptions, ARGS_FILENAME);
					if (!isWindows) {
                		optArgsFilename.setValue("argsFile=" + argsFile.getName());
	        		} else {
    	    			optArgsFilename.setValue("\"argsFile=" + argsFile.getName() + "\"");
        			}
	        	}        	
	        } else {
	        	// parameters from runtime configuration
	        	Option[] runtime_tags = loadTransferredMetadata(dataset, tagIndex, transientData);
	            
	            if (runtime_tags.length < TRANSLATOR_NOOFTAGS_CONFIGURED) {
	            	addOptions(translationOptions, runtime_tags);
	        	} else {
	        		sbOptions = buildSBFromOptions(runtime_tags);
	        		
	        		argsFile = createInputFile(sbOptions, ARGS_FILENAME);
					if (!isWindows) {
        				optArgsFilename.setValue("argsFile=" + argsFile.getName());
	        		} else {
    	    			optArgsFilename.setValue("\"argsFile=" + argsFile.getName() + "\"");
        			}
	        	}
	        }
	        
	        translationOptions.addOption(optArgsFilename);        
	        translationtask.setTranslatorOptions(translationOptions);
	        
	        this.m_zTaskLogger.info("ToNxlFile set property values on new zDBMapInfo");        
	        this.m_zTaskLogger.info("ToNxlFile exits TaskPrep->prepareTask()");
	        
	        return translationtask;
    	} catch (Exception ex) {
    		
    		if (isCheckout) {
	    		// cancel explicit checkout for dataset
		    	// fix bug 45340
				this.m_zTaskLogger.info("ToNxlFile starts cancelCheckout for [" + dataset.getUid() + "]");
				moUtil.cancelCheckout(dataset);
				this.m_zTaskLogger.info("ToNxlFile ends cancelCheckout for [" + dataset.getUid() + "]");
    		}
			
    		throw ex;
    	}
	}
	
	private File createInputFile(StringBuffer content, String fileName) throws IOException {
		File fileList = new File(stagingLoc + File.separator + fileName);
		
		try (
				OutputStreamWriter osw = new OutputStreamWriter(
						new FileOutputStream(fileList), CHARSET);
				BufferedWriter bw = new BufferedWriter(osw);) {
			bw.append(content);
		}
		
        return fileList;
	}
	
	// check is actual file is nxl protected
	private boolean isNXLProtected(ImanFile zlFile) throws NotLoadedException {
		if (zlFile.get_file_ext().equalsIgnoreCase(NXL_FILE_EXT)) { 
			return true;
		} else {
			return false;
		}
	}
	
	// check is file is labeled as nxl protected
	private boolean isLabeledAsNXLProtected(ImanFile zlFile) throws NotLoadedException {
		if (zlFile.get_original_file_name().endsWith(NXL_FILE_EXT)) {
			return true;
		} else {
			return false;
		}
	}
		
	private StringBuffer prepareFileToTranslate(String[] lstRefName, ModelObject[] lstRef, 
			String nxlAction) throws Exception {
		ImanFile zlFile = null;
        File m_InputFile = null;
        StringBuffer filesToTranslate = new StringBuffer("");
        
        // 3.0 feature: NR exclusion list, named reference that won't be protected
        List<String> nrExclusiongList = new ArrayList<>(
        		Arrays.asList(nxlConf.getNRExclusionList()));
        
        boolean isActProtect = (nxlAction.equals(ACT_PROTECT)) ? true : false;
        boolean isActUpdateTag = (nxlAction.equals(ACT_UPDATETAG)) ? true : false;
        
        for (int i = 0; i < lstRef.length; i++) {
        	this.m_zTaskLogger.info("ToNxlFile (" + i + ") " + lstRef[i].getTypeObject().getName());
        	
        	// 3.0 feature
        	// exclude NR in the configured list by skipping it
        	if (!(lstRef[i] instanceof ImanFile)
        			|| nrExclusiongList.contains(lstRefName[i])) {
        		continue;
        	}

        	zlFile = (ImanFile) lstRef[i];
        	
        	// DEBUG TEST
        	dmService.refreshObjects(new ModelObject[] {zlFile});

        	// TAGSYNC & PROTECT
			if (isActUpdateTag && 
        			(!isNXLProtected(zlFile) && !isLabeledAsNXLProtected(zlFile))) {
        		continue;
        	}
        	
        	m_InputFile = TranslationRequest.getFileToStaging(zlFile, stagingLoc);
        	
        	// check is the file is 0kb which will not be processed by RMS
        	// fix bug 27562
        	// fix bug 38138
        	if (m_InputFile.length() > 0) {
        		this.m_zTaskLogger.debug("ToNxlFile (" + i + ") " + nxlAction + " " + lstRefName[i] + 
        				SRC_LIST_DELIMITER + " " + zlFile.get_original_file_name() + 
        				SRC_LIST_DELIMITER + " " + zlFile.get_file_name());

        		filesToTranslate.append(m_InputFile.getName() + SRC_LIST_DELIMITER + lstRefName[i] + 
        				SRC_LIST_DELIMITER + zlFile.get_original_file_name() + SRC_LIST_DELIMITER + 
        				zlFile.get_file_name() + System.getProperty(LINE_SEPARATOR));
        	}
        }
        
        return filesToTranslate;
	}
	
	private StringBuffer buildSBFromOptions(Option[] inputs) {
		StringBuffer sbContent = new StringBuffer("");
		
		if (!isWindows) {
			for (Option input: inputs) {
				String strValue = input.getValue();
				
				sbContent.append(strValue);
				sbContent.append(System.getProperty(LINE_SEPARATOR));
			}
		} else {
			for (Option input: inputs) {
				String strValue = input.getValue();
				
				// truncate the \" and "\
				sbContent.append(strValue.substring(1, strValue.length() - 1));
				sbContent.append(System.getProperty(LINE_SEPARATOR));
			}
		}
		
		return sbContent;
	}
	
	private Option[] loadTransferredMetadata(WorkspaceObject wsObj, int tagIndex, Map<String, String> transientData) {
		// retrieving meta data
    	String[] attributes_name = nxlConf.getTransferredAttr();
    	
    	// 2.7 Bajaj requirement in short deliver time
    	String[] attributes_defaultVal = new String[attributes_name.length];
    	int bIndex = 0;
    	for (String attribute_name : attributes_name) {
    		String[] keyValuePair = attribute_name.split(":");
    		
    		if (keyValuePair.length >= 2) {
    			// trim spaces
    			attributes_defaultVal[bIndex] = keyValuePair[1].trim();
    			attributes_name[bIndex] = keyValuePair[0].trim();
    			
    			this.m_zTaskLogger.debug("ToNxlFile loadTransferredMetadata found default value for " + 
    					attributes_name[bIndex] + "=" + attributes_defaultVal[bIndex]);
    		} else {
    			attributes_defaultVal[bIndex] = null;
    		}
    		
    		bIndex++;
    	}
    	// end of Bajaj requirement
    	
    	this.m_zTaskLogger.debug("ToNxlFile loadTransferredMetadata length of attributes_name=" + attributes_name.length);
    	
    	// retrieving meta data for business object from TC
    	dmService.getProperties(new ModelObject[] {wsObj}, attributes_name);
    	
    	// populate meta data to Options
    	String[] attributes_val = new String[attributes_name.length];
    	Option[] options = new Option[attributes_name.length];
    	
    	int index = 0;
    	for (String attribute_name : attributes_name) {
    		options[index] = new Option();
			options[index].setName(TAG_PREFIX + tagIndex++);
    		
    		try {
    			Property property = wsObj.getPropertyObject(attribute_name);
    			List<String> values = property.getDisplayableValues();
    			
    			String value = values.toString();
    			value = StringUtils.stripStart(value, "[");
    			value = StringUtils.stripEnd(value, "]");
    			    			
    			if (property instanceof PropertyBoolImpl) {
    				value = String.valueOf(property.getBoolValue());
    			}
    			
    			// 2.7 Bajaj requirement in short deliver time
    			if (value.trim().equals("") && attributes_defaultVal[index] != null) {
    				this.m_zTaskLogger.debug("ToNxlFile loadTransferredMetadata override with default value");
    				value = attributes_defaultVal[index];
    			}
    			
    			if (attribute_name.equals(NXL_LAST_MOD_USER)) {
    				if (transientData != null && transientData.containsKey(NXL_LAST_MOD_USER)) {
    					value = transientData.get(NXL_LAST_MOD_USER);
    				}
    			} else if (attribute_name.equals(NXL_LAST_MOD_DATE)) {
    				if (transientData != null && transientData.containsKey(NXL_LAST_MOD_DATE)) {
    					value = transientData.get(NXL_LAST_MOD_DATE);
    				}
    			}
    			// end of Bajaj requirement
    			
    			this.m_zTaskLogger.debug("ToNxlFile loadTransferredMetadata " + attribute_name + "=" + value);
    			
    			attributes_val[index] = value;
    		} catch (NotLoadedException nle) {
    			this.m_zTaskLogger.error("ToNxlFile caught NotLoadedException for " + attribute_name);
    			
    			// if the attribute is not loaded, flag not loaded
    			attributes_val[index] = "[NOT LOADED]";
    		} catch (IllegalArgumentException iae) {
    			this.m_zTaskLogger.error("ToNxlFile caught IllegalArgumentException for " + attribute_name);
    			
    			// if the attribute is not exists, don't added the attribute
    			attributes_val[index] = "[ILLEGAL ARGS]";
    		}
    		
			if (!isWindows) {
    			options[index].setValue(attributes_name[index] + "=" + attributes_val[index]);
    		} else {
    			options[index].setValue("\"" + attributes_name[index] + "=" + attributes_val[index] + "\"");
    		}
    		
    		index++;
    	}
        
    	return options;
	}
	
	private Map<String, String> saveLastModNXL(Dataset sourceDataset) throws Exception {
		StringBuffer content = new StringBuffer("");
		Map<String, String> transientData = new HashMap<>();
				
		DataManagementService dmService = DataManagementService.getService(ConnectionManager.getActiveConnection());
		// DEBUG TEST
		//dmService.refreshObjects(new ModelObject[] {sourceDataset});
		dmService.getProperties(new ModelObject[] {sourceDataset}, new String[] {
				"last_mod_user", "last_mod_date", NXL_LAST_MOD_USER, NXL_LAST_MOD_DATE});
				
		try {
			String dcUid = ConnectionManager.getLoggedInUser().getUid();
									
			String strNxlLastModUser = sourceDataset.getPropertyDisplayableValue(NXL_LAST_MOD_USER).toString();
			String strNxlLastModDate = sourceDataset.getPropertyDisplayableValue(NXL_LAST_MOD_DATE).toString();
			strNxlLastModUser = StringUtils.stripStart(strNxlLastModUser, "[");
			strNxlLastModUser = StringUtils.stripEnd(strNxlLastModUser, "]");
			strNxlLastModDate = StringUtils.stripStart(strNxlLastModDate, "[");
			strNxlLastModDate = StringUtils.stripEnd(strNxlLastModDate, "]");
			
			String strLastModUser = sourceDataset.getPropertyDisplayableValues("last_mod_user").toString();
			String strLastModDate = sourceDataset.getPropertyDisplayableValues("last_mod_date").toString();
			strLastModUser = StringUtils.stripStart(strLastModUser, "[");
			strLastModUser = StringUtils.stripEnd(strLastModUser, "]");
			strLastModDate = StringUtils.stripStart(strLastModDate, "[");
			strLastModDate = StringUtils.stripEnd(strLastModDate, "]");
			
			this.m_zTaskLogger.debug("\tdc uid: " + dcUid);
			this.m_zTaskLogger.debug("\tlast_mod_user: " + sourceDataset.get_last_mod_user().getUid());
			this.m_zTaskLogger.debug("\tlast_mod_date: " + Property.toDateString(sourceDataset.get_last_mod_date()));
			this.m_zTaskLogger.debug("\tnxl3_last_mod_user: " + strNxlLastModUser);
			this.m_zTaskLogger.debug("\tnxl3_last_mod_date: " + strNxlLastModDate);
			
			if (sourceDataset.get_last_mod_user().getUid().equals(dcUid) && 
					!strNxlLastModUser.trim().equals("") && !strNxlLastModDate.trim().equals("")) {
				this.m_zTaskLogger.debug("\tused nxl last mod fields");
				
				POM_system_class lastModUser = (POM_system_class) sourceDataset.getPropertyObject(NXL_LAST_MOD_USER).getModelObjectValue();
				Calendar lastModDate = sourceDataset.getPropertyObject(NXL_LAST_MOD_DATE).getCalendarValue();
				
				content.append(lastModUser.getUid() + System.getProperty(LINE_SEPARATOR));
				content.append(Property.toDateString(lastModDate));
				
				transientData.put(NXL_LAST_MOD_USER, strNxlLastModUser);
				transientData.put(NXL_LAST_MOD_DATE, strNxlLastModDate);
			} else {
				this.m_zTaskLogger.debug("\tused last mod fields");
				
				content.append(sourceDataset.get_last_mod_user().getUid() + System.getProperty(LINE_SEPARATOR));
				content.append(Property.toDateString(sourceDataset.get_last_mod_date()));
				
				transientData.put(NXL_LAST_MOD_USER, strLastModUser);
				transientData.put(NXL_LAST_MOD_DATE, strLastModDate);
			}
						
			createInputFile(content, TRANSIENT_FILENAME);
		} catch (NotLoadedException ex) {
			this.m_zTaskLogger.error("TaskPrep.saveLastModNXL() caught NotLoadedException: " + ex.getMessage(), ex);
		} catch (Exception ex) {
			this.m_zTaskLogger.error("TaskPrep.saveLastModNXL() caught Exception: " + ex.getMessage(), ex);
		}
		
		return transientData;
	}
	
	private void addOptions(TranslatorOptions translationOptions, Option[] options) {
		for (Option option: options) {
        	translationOptions.addOption(option);
        }
	}
	
}
