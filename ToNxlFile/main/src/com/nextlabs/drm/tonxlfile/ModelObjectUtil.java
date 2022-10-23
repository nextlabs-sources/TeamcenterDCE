package com.nextlabs.drm.tonxlfile;

/**
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

import java.util.concurrent.locks.ReentrantLock;

import com.teamcenter.ets.soa.ConnectionManager;
import com.teamcenter.services.strong.core.SessionService;
import com.teamcenter.services.strong.core.ReservationService;
import com.teamcenter.services.strong.core._2007_12.Session.StateNameValue;
import com.teamcenter.services.strong.core._2011_06.Reservation.OkToCheckoutResponse;
import com.teamcenter.soa.client.model.ErrorStack;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.Dataset;
import com.teamcenter.soa.exceptions.NotLoadedException;
import com.teamcenter.tstk.util.log.ITaskLogger;
import static com.nextlabs.drm.tonxlfile.configuration.NextLabsConstants.*;

public class ModelObjectUtil {
	
	private ITaskLogger taskLogger;
	private static final ReentrantLock lock = new ReentrantLock();
	
	public ModelObjectUtil(ITaskLogger logger) {
		taskLogger = logger;
	}
	
	public void setBypass() throws Exception {
		taskLogger.trace("setBypass starts...", 0);
		
		StateNameValue[] stateNameVals = new StateNameValue[1];
	    stateNameVals[0] = new StateNameValue();
	    stateNameVals[0].name = "bypassFlag";
	    stateNameVals[0].value = "true";
		
		SessionService sessionService = SessionService.getService(ConnectionManager.getActiveConnection());
		
		// obtain lock
		taskLogger.trace("obtaining lock, queue length: " + lock.getQueueLength() + 
				", threadid: " + Thread.currentThread().getId(), 0);
		lock.lock();
		taskLogger.trace("obtained lock, queue length: " + lock.getQueueLength() + 
				", threadid: " + Thread.currentThread().getId(), 0);
		
		try {
		    ServiceData serviceData = sessionService.setUserSessionState(stateNameVals);
		    
		    if (taskLogger.isTraceEnabled()) {
		    	logServiceData(serviceData);
		    	taskLogger.trace("bypassFlag :" + sessionService.getTCSessionInfo().bypass, 0);
		    }
		    
		    taskLogger.trace("setBypass ends...", 0);
		} catch (Exception ex) {
			taskLogger.error("ModelObjectUtil.setBypass() caught exception: " + ex.getMessage(), ex);
		}
	}
	
	public void unsetBypass() throws Exception {
		taskLogger.trace("unsetBypass starts...", 0);		
		
	    StateNameValue[] stateNameVals = new StateNameValue[1];
	    stateNameVals[0] = new StateNameValue();
	    stateNameVals[0].name = "bypassFlag";
	    stateNameVals[0].value = "false";
	    
	    SessionService sessionService = null;
	    ServiceData serviceData = null;

	    try {
	    	sessionService = SessionService.getService(ConnectionManager.getActiveConnection());
	    	serviceData = sessionService.setUserSessionState(stateNameVals);
	    } catch (Exception ex) {
	    	taskLogger.error("ModelObjectUtil.unsetBypass() caught exception: " + ex.getMessage(), ex);
	    }
	    
	    // release lock
	    taskLogger.trace("releasing lock, qlength: " + lock.getQueueLength() + 
				", threadid: " + Thread.currentThread().getId(), 0);
	    lock.unlock();
	    taskLogger.trace("released lock, qlength: " + lock.getQueueLength() + 
				", threadid: " + Thread.currentThread().getId(), 0);
	    
	    if (taskLogger.isTraceEnabled()) {
	    	if (serviceData != null)
	    		logServiceData(serviceData);
	    	
	    	if (sessionService != null)
	    		taskLogger.trace("bypassFlag :" + sessionService.getTCSessionInfo().bypass, 0);
	    }
	    
	    taskLogger.trace("unsetBypass ends...", 0);
	}
	
	public boolean checkout(ModelObject modelObject, String message, String changeId) throws Exception {
		boolean isCheckout = false;
		boolean okToCheckout = false;
		
		try {			
	    	ReservationService rService = ReservationService.getService(ConnectionManager.getActiveConnection());
	    	
	    	this.setBypass();
	    	
	    	// workaround 08072017
	    	int RETRY_COUNT = 3;
	    	boolean workaroundRetry = false;
	    	//OkToCheckoutResponse okToCheckoutResponse = rService.okToCheckout(new ModelObject[] { modelObject });
	    	OkToCheckoutResponse okToCheckoutResponse;
	    	
	    	do {
	    		okToCheckoutResponse = rService.okToCheckout(new ModelObject[] {modelObject});
	    		    	
		    	if (taskLogger.isTraceEnabled()) {
		    		taskLogger.trace(" okToCheckoutResponse serviceData: ", 0);
			    	logServiceData(okToCheckoutResponse.serviceData);
			    	
			    	taskLogger.trace(" okToCheckoutResponse.verdict length: " + okToCheckoutResponse.verdict.length, 0);
			    	for (boolean okTo : okToCheckoutResponse.verdict) {
			    		taskLogger.trace("   verdict > " + okTo, 0);
			    	}
		    	}
		    	
		    	workaroundRetry = isWorkaround08072017Data(okToCheckoutResponse.serviceData);
		    	if (workaroundRetry) {
		    		try {
		    			taskLogger.trace(" WORKAROUND RETRY: " + RETRY_COUNT--, 0);
		    			Thread.sleep(1000);
		    		} catch (InterruptedException ie) {
		    			Thread.currentThread().interrupt();
		    		}
		    	}
	    	} while (workaroundRetry && RETRY_COUNT > 0);
	    	
	    	if (okToCheckoutResponse.verdict.length == 1) {
	    		okToCheckout = okToCheckoutResponse.verdict[0];
	    		
	    		if (okToCheckout) {
	    			ServiceData serviceData = rService.checkout(new ModelObject[] {modelObject}, message, changeId);
	    			
	    			// log service data
	    	    	logServiceData(serviceData);
	    	    	
	    	    	if (serviceData.sizeOfPartialErrors() == 0 && serviceData.sizeOfUpdatedObjects() >= 1) {
	    	    		isCheckout = true;
	    	    	}
	    		} else {
	    			String moChangeId = "";
	    			int changeidSize = modelObject.getPropertyDisplayableValues(PROP_CHECKED_OUT_CHANGE_ID).size();
	    			
	    			taskLogger.trace(" [" + modelObject.getUid() + "] changeid size: " + changeidSize, 0);
	    			
	    			if (changeidSize > 0) {
	    				changeId = modelObject.getPropertyDisplayableValues(
	    						PROP_CHECKED_OUT_CHANGE_ID).get(0);
	    				
	    				taskLogger.trace(" [" + modelObject.getUid() + "] changeid: " + changeId, 0);
	    			}
	    			
	    			if (moChangeId != null && changeId.equals(NXL_CHANGEID)) {
	    				isCheckout = true;
	    			}
	    		}
	    	}
		} catch (Exception ex) {
			taskLogger.error("ModelObjectUtil.checkout() caught exception: " + ex.getMessage(), ex);
		} finally {
			//if (okToCheckout)
			this.unsetBypass();
		}
		
		return isCheckout;
	}
	
	public boolean cancelCheckout(ModelObject modelObject) throws Exception {
		boolean isCheckoutCancel = false;
		
		try {			
			ReservationService rService = ReservationService.getService(ConnectionManager.getActiveConnection());
			
			this.setBypass();
			ServiceData serviceData = rService.cancelCheckout(new ModelObject[] { modelObject });
	    	
			if (taskLogger.isTraceEnabled()) {
				// log service data
				logServiceData(serviceData);
			}
	    	
	    	if (serviceData.sizeOfPartialErrors() == 0) {
	    		isCheckoutCancel = true;
	    	}
		} catch (Exception ex) {
			taskLogger.error("ModelObjectUtil.cancelCheckout() caught exception: " + ex.getMessage(), ex);
		} finally {
			this.unsetBypass();
		}
		
		return isCheckoutCancel;
	}
	
	public void logServiceData(ServiceData serviceData) {
		taskLogger.info("Size of partial errors: " + serviceData.sizeOfPartialErrors());
		taskLogger.info("Size of deleted objects: " + serviceData.sizeOfDeletedObjects());
		taskLogger.info("Size of updated objects: " + serviceData.sizeOfUpdatedObjects());
		
		if (serviceData.sizeOfPartialErrors() > 0) {
			for (int i = 0; i < serviceData.sizeOfPartialErrors(); i++) {
				ErrorStack errStack = serviceData.getPartialError(i);
				
				for (String errMsg : errStack.getMessages()) {
					taskLogger.info("Error message[" + i + "]: " + errMsg);
				}
			}
		}
		
		if (taskLogger.isDebugEnabled()) {			
			for (int i = 0; i < serviceData.sizeOfDeletedObjects(); i++) {
				String deletedObject = serviceData.getDeletedObject(i);
				
				taskLogger.debug("deletedObject[" + i + "]: " + deletedObject);
			}
			
			for (int i = 0; i < serviceData.sizeOfUpdatedObjects(); i++) {
				ModelObject modObj = serviceData.getUpdatedObject(i);
				
				taskLogger.debug("updatedObject[" + i + "]: " + 
						modObj.getTypeObject().getName() + "; " + modObj.getUid());
			}
		}
	}
	
	// workaround 08072017
	private boolean isWorkaround08072017Data(ServiceData serviceData) {
		String MATCHES_MSG = "The object is locked for modify in another session";
		
		if (serviceData.sizeOfPartialErrors() > 0) {
			for (int i = 0; i < serviceData.sizeOfPartialErrors(); i++) {
				ErrorStack errStack = serviceData.getPartialError(i);
				
				for (String errMsg : errStack.getMessages()) {
					if (errMsg != null && errMsg.contains(MATCHES_MSG))
						return true;
				}
			}
		}
		
		return false;
	}
	
	// fix bug 38158
	public boolean isValidDatasetStatus(Dataset dataset) {
		String checkoutStatus = DEFAULT_CHECKOUT_STATUS;
		String changeId = "";
		
		try {
			if (dataset.getPropertyDisplayableValues(
					PROP_CHECKED_OUT).size() > 0) {
				checkoutStatus = dataset.getPropertyDisplayableValues(
						PROP_CHECKED_OUT).get(0);
			}
			
			if (dataset.getPropertyDisplayableValues(
					PROP_CHECKED_OUT_CHANGE_ID).size() > 0) {
				changeId = dataset.getPropertyDisplayableValues(
						PROP_CHECKED_OUT_CHANGE_ID).get(0);
			}
		} catch (NotLoadedException e) {
			taskLogger.error("ToNxlFile caught exception in isValidDatasetStatus(): " + e.getMessage(), e);
		}
		
		if (checkoutStatus.equals("Y") && !changeId.equals(NXL_CHANGEID)) {
			return false;
		} else {
			return true;
		}
	}

}
