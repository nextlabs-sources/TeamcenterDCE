package com.nextlabs.drm.rmx.batchprocess;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.nextlabs.drm.rmx.batchprocess.configuration.SizeFormat;
import com.nextlabs.drm.rmx.batchprocess.dto.InputObjectDTO;
import com.nextlabs.drm.rmx.batchprocess.dto.DatasetDTO;
import com.nextlabs.nxl.crypt.RightsManager;
import com.nextlabs.nxl.exception.NXRTERROR;
import com.nextlabs.nxl.pojos.PolicyControllerDetails;

import static com.nextlabs.drm.rmx.batchprocess.configuration.BatchProcessConstants.*;

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

public class BatchDecrypt implements BatchProcess {
	
	private static final Logger LOGGER = LogManager.getLogger("BPLOGGER");
	private static final Logger REPORTLOGGER = LogManager.getLogger("REPORTLOGGER");
	
	private final String tcUserName;
	private final String stagingFoldPath;
	private final int numOfThreads;
	
	// Tc services
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
	
	public BatchDecrypt(String tcUserName, String stagingFoldPath, int numOfThreads, 
			Connection connection, PolicyControllerDetails pcDetails) throws NXRTERROR {
		this.tcUserName = tcUserName;
		this.stagingFoldPath = stagingFoldPath;
		this.numOfThreads = numOfThreads;
		this.totalFilesSize = new AtomicLong(0);
		
		dmService = DataManagementService.getService(connection);
		rmService = ReservationService.getService(connection);
		sesService = SessionService.getService(connection);
		this.connection = connection;
				
		refInfoService = new RefInfoService(dmService);
		volInfoService = new VolInfoService(dmService);
		
		rightsManager = new RightsManager(pcDetails);
	}
	
	@Override
	public void execute(List<InputObjectDTO> lstObjects) throws Exception {
		String[] uids = new String[lstObjects.size()];
		int dtoIndex = 0;
		
		for (InputObjectDTO dto : lstObjects)
			uids[dtoIndex++] = ((DatasetDTO) dto).getUid();
		
		ServiceData serviceData = dmService.loadObjects(uids);
		LOGGER.info("login as: {}", tcUserName);
		LOGGER.info("loaded/input MO size: {}/{}", serviceData.sizeOfPlainObjects(), lstObjects.size());
		
		ModelObject[] modelObjects = new ModelObject[serviceData.sizeOfPlainObjects()];
		// handle uids > sizeOfPlainObjects
		for (int i = 0; i < serviceData.sizeOfPlainObjects(); i++) {
			modelObjects[i] = serviceData.getPlainObject(i);
		}
		
		List<String> lstUidNotLoaded = BatchProcessUtility.findUidsNotLoaded(uids, modelObjects);
		if (!lstUidNotLoaded.isEmpty()) {
			StringBuilder sb = new StringBuilder("");
			for (String uidNotLoaded : lstUidNotLoaded)
				sb.append(uidNotLoaded + " ");
			
			REPORTLOGGER.info("failed to load: {}", sb);
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
		LOGGER.info("threadPool size: {}", nThreads);
		
		setByPass(true);
		long startTime = System.nanoTime();
		List<String> lstNonDataset = new ArrayList<>();
		List<String> lstCheckoutDataset = new ArrayList<>();
		
		for (ModelObject mo : modelObjects) {
			if (mo instanceof Dataset) {
				try {
					String checkoutStatus = getCheckoutStatus(mo);
					LOGGER.info("[{}] {} checkout[{}]", mo.getUid(), 
							mo.getPropertyDisplayableValue("object_name"), checkoutStatus);
					
					if (checkoutStatus.equalsIgnoreCase(" ")) {
						DecryptFilesTask decryptFilesTask = 
								new DecryptFilesTask((Dataset) mo, dmService, rmService, sesService, 
										new FileManagementUtility(connection), refInfoService, volInfoService, rightsManager, 
										stagingFoldPath, totalFilesSize);
						
						try {
							exeService.execute(decryptFilesTask);
						} catch (Exception ex) {
							LOGGER.error("error: {}", ex.getMessage(), ex);
						}
					} else {
						LOGGER.warn("cannot process checkout object {} [{}]", mo.getPropertyDisplayableValue("object_name"), mo.getUid());
						lstCheckoutDataset.add("[" + mo.getUid() + "] " + mo.getPropertyDisplayableValue("object_name"));
					}
				} catch (NotLoadedException e) {
					LOGGER.error("error: {}", e.getMessage(), e);
				}
			} else {
				LOGGER.warn("cannot process non-dataset object {} [{}]", mo.getPropertyDisplayableValue("object_name"), mo.getUid());
				lstNonDataset.add("[" + mo.getUid() + "] " + mo.getPropertyDisplayableValue("object_name"));
			}
		}
		
		
		exeService.shutdown();
		// await indefinitely, for all the tasks to complete
		exeService.awaitTermination(Long.MAX_VALUE, TimeUnit.HOURS);
		
		LOGGER.info("batch decrypt completed in {}", 
				BatchProcessUtility.getTimeFormat((System.nanoTime() - startTime)));
		REPORTLOGGER.info("batch decrypt completed in {}",  
				BatchProcessUtility.getTimeFormat((System.nanoTime() - startTime)));
		REPORTLOGGER.info("processed {}",  
				BatchProcessUtility.getSizeFormat(totalFilesSize.get(), SizeFormat.MEGABYTES) + " data");
		
		if (!lstNonDataset.isEmpty())
			REPORTLOGGER.info("cannot process non-dataset object: {}", lstNonDataset);
		
		if (!lstCheckoutDataset.isEmpty())
			REPORTLOGGER.info("cannot process checkout dataset: {}", lstCheckoutDataset);
	}
	
	private String getCheckoutStatus(ModelObject modelObject) throws NotLoadedException {
		if (modelObject.getPropertyDisplayableValues(PROP_CHECKED_OUT).size() > 0)
			return modelObject.getPropertyDisplayableValues(PROP_CHECKED_OUT).get(0);
		else
			LOGGER.warn("{} [{}] has {} < 0", modelObject.getPropertyDisplayableValue("object_name"), 
					modelObject.getUid(), PROP_CHECKED_OUT);

		return DEFAULT_CHECKOUT_STATUS;
	}
		
	private boolean setByPass(boolean flag) {
		LOGGER.trace("setBypass starts... [{}]", flag);
		
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
		    	LOGGER.trace("bypassFlag :{}", sesService.getTCSessionInfo().bypass);
		    }
		    
		    LOGGER.trace("setBypass ends... [{}]", flag);
		    
		    return true;
		} catch (Exception ex) {
			LOGGER.error("BatchDecrypt.setBypass() caught exception: {}", ex.getMessage(), ex);
			return false;
		}
	}

}
