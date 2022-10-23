package com.nextlabs.drm.rmx.batchprocess;

import static com.nextlabs.drm.rmx.batchprocess.configuration.BatchProcessConstants.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.nextlabs.drm.rmx.batchprocess.configuration.SizeFormat;
import com.nextlabs.drm.rmx.batchprocess.dto.ConfigPropertiesDTO;
import com.nextlabs.drm.rmx.batchprocess.dto.DatasetDTO;
import com.nextlabs.drm.rmx.batchprocess.dto.InputObjectDTO;
import com.nextlabs.nxl.NxlException;
import com.nextlabs.nxl.RightsManager;
import com.teamcenter.services.loose.administration.PreferenceManagementService;
import com.teamcenter.services.loose.administration._2012_09.PreferenceManagement.GetPreferencesResponse;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.core.ReservationService;
import com.teamcenter.services.strong.core.SessionService;
import com.teamcenter.services.strong.core._2007_12.Session.StateNameValue;
import com.teamcenter.soa.client.Connection;
import com.teamcenter.soa.client.FileManagementUtility;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.Property;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.Dataset;
import com.teamcenter.soa.exceptions.NotLoadedException;

public class BatchEncrypt implements BatchProcess {
	
	private final static Logger LOGGER = LogManager.getLogger("BPLOGGER");
	private final static Logger REPORTLOGGER = LogManager.getLogger("REPORTLOGGER");
	
	private final String tcUserName;
	private final String stagingFoldPath;
	private final int numOfThreads;
	
	// Tc services
	private final PreferenceManagementService prefService;
	private final DataManagementService dmService;
	private final ReservationService rmService;
	private final SessionService sesService;
	private final Connection connection;

	// internal lookup service
	private final RefInfoService refInfoService;
	private final VolInfoService volInfoService;

	private final RightsManager rightsManager;
	
	// files' size accumulator
	private final AtomicLong totalFilesSize;
	
	public BatchEncrypt(String tcUserName, String stagingFoldPath, int numOfThreads, 
			Connection connection, ConfigPropertiesDTO configProp) throws NxlException {
		this.tcUserName = tcUserName;
		this.stagingFoldPath = stagingFoldPath;
		this.numOfThreads = numOfThreads;
		this.totalFilesSize = new AtomicLong(0);
		
		dmService = DataManagementService.getService(connection);
		prefService = PreferenceManagementService.getService(connection);
		rmService = ReservationService.getService(connection);
		sesService = SessionService.getService(connection);
		this.connection = connection;
		
		refInfoService = new RefInfoService(dmService);
		volInfoService = new VolInfoService(dmService);
		
		rightsManager = new RightsManager(configProp.getRouterURL(), configProp.getAppID(), configProp.getAppKey());
	}
	
	@Override
	public void execute(List<InputObjectDTO> lstObjects) throws Exception {
		String[] uids = new String[lstObjects.size()];
		int dtoIndex = 0;
		
		for (InputObjectDTO dto : lstObjects)
			uids[dtoIndex++] = ((DatasetDTO) dto).getUid();
		
		ServiceData serviceData = dmService.loadObjects(uids);
		LOGGER.info("login as: " + tcUserName);
		LOGGER.info("loaded/input MO size: " + serviceData.sizeOfPlainObjects() + 
				"/" + lstObjects.size());
		
		ModelObject[] modelObjects = new ModelObject[serviceData.sizeOfPlainObjects()];
		// handle uids > sizeOfPlainObjects
		for (int i = 0; i < serviceData.sizeOfPlainObjects(); i++) {
			modelObjects[i] = serviceData.getPlainObject(i);
		}
		
		List<String> lstUidNotLoaded = BatchProcessUtility.findUidsNotLoaded(uids, modelObjects);
		if (lstUidNotLoaded.size() > 0) {
			StringBuilder sb = new StringBuilder("");
			for (String uidNotLoaded : lstUidNotLoaded)
				sb.append(uidNotLoaded + " ");
			
			REPORTLOGGER.info("failed to load: " + sb.toString());
		}
		
		LOGGER.debug("getting properties for loaded MO");
		serviceData = dmService.getProperties(modelObjects, PROPS_MODELOBJS);
		if (LOGGER.isTraceEnabled())
			BatchProcessUtility.logServiceData(serviceData);
		
		int nThreads;
		if (numOfThreads == 0)
			nThreads = Runtime.getRuntime().availableProcessors();
		else
			nThreads = numOfThreads;
		
		ExecutorService exeService = Executors.newFixedThreadPool(nThreads);
		LOGGER.info("threadPool size: " + nThreads);
		
		setByPass(true);
		long startTime = System.nanoTime();
		List<String> lstNonDataset = new ArrayList<>();
		List<String> lstCheckoutDataset = new ArrayList<>();
		
		List<String> lstNRExclude = Arrays.asList(getNRExclusionList());
		Map<String, String> lstAttribute = getTransferredAttributes(); 
		
		for (ModelObject mo : modelObjects) {
			if (mo instanceof Dataset) {
				try {
					String checkoutStatus = getCheckoutStatus(mo);
					LOGGER.info("[" + mo.getUid() + "] " + mo.getPropertyDisplayableValue("object_name") + 
							" checkout[" + checkoutStatus + "]");
					
					if (checkoutStatus.equalsIgnoreCase(" ")) {
						EncryptFilesTask encryptFilesTask = 
								new EncryptFilesTask((Dataset) mo, dmService, rmService, sesService, 
										new FileManagementUtility(connection), refInfoService, volInfoService, rightsManager, 
										stagingFoldPath, totalFilesSize, lstNRExclude, lstAttribute);
						try {
							exeService.execute(encryptFilesTask);
						} catch (Exception ex) {
							LOGGER.error("error: " + ex.getMessage(), ex);
						}
					} else {
						LOGGER.warn("cannot process checkout object " + mo.getPropertyDisplayableValue("object_name") + 
								" [" + mo.getUid() + "]");
						lstCheckoutDataset.add("[" + mo.getUid() + "] " + mo.getPropertyDisplayableValue("object_name"));
					}
				} catch (NotLoadedException e) {
					LOGGER.error("error: " + e.getMessage(), e);
				}
			} else {
				LOGGER.warn("cannot process non-dataset object " + mo.getPropertyDisplayableValue("object_name") + 
						" [" + mo.getUid() + "]");
				lstNonDataset.add("[" + mo.getUid() + "] " + mo.getPropertyDisplayableValue("object_name"));
			}
		}
		
		
		exeService.shutdown();
		// await indefinitely, for all the tasks to complete
		exeService.awaitTermination(Long.MAX_VALUE, TimeUnit.HOURS);
		
		LOGGER.info("batch encrypt completed in " + 
				BatchProcessUtility.getTimeFormat((System.nanoTime() - startTime)));
		REPORTLOGGER.info("batch encrypt completed in " + 
				BatchProcessUtility.getTimeFormat((System.nanoTime() - startTime)));
		REPORTLOGGER.info("processed " + 
				BatchProcessUtility.getSizeFormat(totalFilesSize.get(), SizeFormat.MEGABYTES) + " data");
		
		if (lstNonDataset.size() > 0)
			REPORTLOGGER.info("cannot process non-dataset object: " + lstNonDataset.toString());
		
		if (lstCheckoutDataset.size() > 0)
			REPORTLOGGER.info("cannot process checkout dataset: " + lstCheckoutDataset.toString());
	}
	
	private String getCheckoutStatus(ModelObject modelObject) throws NotLoadedException {
		if (modelObject.getPropertyDisplayableValues(PROP_CHECKED_OUT).size() > 0)
			return modelObject.getPropertyDisplayableValues(PROP_CHECKED_OUT).get(0);
		else
			LOGGER.warn(modelObject.getPropertyDisplayableValue("object_name") + 
					" [" + modelObject.getUid() + "] has " + PROP_CHECKED_OUT + " < 0");

		return DEFAULT_CHECKOUT_STATUS;
	}
		
	private boolean setByPass(boolean flag) {
		LOGGER.trace("setBypass starts... [" + flag + "]");
		
		StateNameValue[] stateNameVals = new StateNameValue[1];
	    stateNameVals[0] = new StateNameValue();
	    stateNameVals[0].name = "bypassFlag";
	    stateNameVals[0].value = Property.toBooleanString(flag);
	    
	    try {
	    	// can be improved to check user is System Administrator group before setting bypass
	    	// normal user will get msg that cannot set bypass.
		    ServiceData serviceData = sesService.setUserSessionState(stateNameVals);
		    
		    if (LOGGER.isTraceEnabled()) {
		    	BatchProcessUtility.logServiceData(serviceData);
		    	LOGGER.trace("bypassFlag :" + sesService.getTCSessionInfo().bypass, 0);
		    }
		    
		    LOGGER.trace("setBypass ends... [" + flag + "]");
		    
		    return true;
		} catch (Exception ex) {
			LOGGER.error("BatchEncrypt.setBypass() caught exception: " + ex.getMessage(), ex);
			ex.printStackTrace(System.out);
			
			return false;
		}
	}
	
	private String[] readTcPreference(String pref_name) {
		String[] pref_values = new String[] {};
			
		try {
			// improvement 18 Oct 2016: refresh TC preferences
			prefService.refreshPreferences();
			
			GetPreferencesResponse prefResponse = prefService.getPreferences(
					new String[] {pref_name}, false);
			
			if (prefResponse.response != null &&
					prefResponse.response.length >= 1) {
				
				if (prefResponse.response[0].values != null && 
						prefResponse.response[0].values.values != null) {
					
					pref_values = prefResponse.response[0].values.values;
				}
			}
		} catch (Exception ex) {
			ex.printStackTrace(System.out);
		}
		
		return pref_values;
	}
	
	private String[] getNRExclusionList() {
		return readTcPreference(NXL_NR_BLACKLIST);
	}
	
	private Map<String, String> getTransferredAttributes() {
		Map<String, String> lstAttributes = new HashMap<>();
		
    	String[] attributes_defaultVal = readTcPreference(NXL_TRANSFERRED_ATTRIBUTES);
    	for (String attrDefVal : attributes_defaultVal) {
    		String[] keyValuePair = attrDefVal.split(":");
    		String key = keyValuePair[0].trim();
    		String defVal;
    		
    		if (keyValuePair.length >= 2) {
    			// trim spaces
    			defVal = keyValuePair[1].trim();
    			
    			LOGGER.debug("BatchEncrypt.getTransferredAttributes() found default value for " + 
    					key + "=" + defVal);
    		} else
    			defVal = null;
    		
    		lstAttributes.put(key, defVal);
    	}
    	
		return lstAttributes;
	}
	
	

}
