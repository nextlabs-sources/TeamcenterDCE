package com.nextlabs.drm.rmx.batchprocess.tcsession;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.teamcenter.soa.client.model.ModelEventListener;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.exceptions.NotLoadedException;

public class TCModelEventListener extends ModelEventListener {
	
	private final static Logger LOGGER = LogManager.getLogger("BPLOGGER");
	
	@Override
	public void localObjectChange(ModelObject[] objects) {
		if (objects.length == 0) return;
		
		LOGGER.debug("TCModelEventListener.localObjectChange() caughts the following objects have been updated in the client data model:");
		for (ModelObject mo : objects) {
			String uid = mo.getUid();
			String type = mo.getTypeObject().getName();
			String name = "";
			
			if (mo.getTypeObject().isInstanceOf("WorkspaceObject")) {
				ModelObject wo = mo;
				try {
					name = wo.getPropertyObject("object_string").getStringValue();
				} catch (NotLoadedException e) {}
			}
			LOGGER.debug("  " + uid + " " + type + " " + name);
		}
	}
	
	@Override
	public void localObjectDelete(String[] uids) {
		if (uids.length == 0) return;
		
		LOGGER.debug("TCModelEventListener.localObjectDelete() caughts the following objects have been deleted from the server and removed from the client data model:");
		for (String uid : uids)
			LOGGER.debug("  " + uid);
	}

}
