package com.nextlabs.drm.rmx.batchprocess;

import java.util.concurrent.ConcurrentHashMap;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.soa.client.Connection;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.strong.ImanVolume;
import com.teamcenter.soa.exceptions.NotLoadedException;

import static com.nextlabs.drm.rmx.batchprocess.configuration.BatchProcessConstants.*;

public class VolInfoService {
	
	private final DataManagementService dmService;
	private final ConcurrentHashMap<String, String> volInfoLookup;
	
	private static final Logger LOGGER = LogManager.getLogger("RISVRLOGGER");
	
	public VolInfoService(DataManagementService dmService) {
		this.dmService = dmService;
		volInfoLookup = new ConcurrentHashMap<>();
	}
	
	public VolInfoService(Connection connection) {
		this(DataManagementService.getService(connection));
	}
	
	public String getVolumeName(ImanVolume vol) {
		long startTime = System.nanoTime();
		String volName = "";
		
		if (volInfoLookup.contains(vol.getUid()))
			volName = volInfoLookup.get(vol.getUid());
		else {
			dmService.getProperties(new ModelObject[] {vol}, PROPS_IMANVOLUME);
			try {
				volName = vol.get_volume_name();
				volInfoLookup.putIfAbsent(vol.getUid(), volName);
			} catch (NotLoadedException e) {
				LOGGER.error(e.getMessage(), e);
			}
		}
		
		long duration = System.nanoTime() - startTime;
		LOGGER.info("getVolumeName[{}, {}] completed in {}", vol.getUid(), volName, BatchProcessUtility.getTimeFormat(duration));
		
		return volName;
	}

}
