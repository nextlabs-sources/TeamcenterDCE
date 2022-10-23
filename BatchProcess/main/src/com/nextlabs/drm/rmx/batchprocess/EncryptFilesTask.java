package com.nextlabs.drm.rmx.batchprocess;

import static com.nextlabs.drm.rmx.batchprocess.configuration.BatchProcessConstants.*;

import java.io.File;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

import org.apache.commons.lang.StringUtils;

import com.nextlabs.drm.rmx.batchprocess.dto.TaskFileDTO;

import com.nextlabs.common.shared.Constants.TokenGroupType;
import com.nextlabs.nxl.RightsManager;

import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.core.ReservationService;
import com.teamcenter.services.strong.core.SessionService;
import com.teamcenter.soa.client.FileManagementUtility;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.Property;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.Dataset;
import com.teamcenter.soa.exceptions.NotLoadedException;
import com.teamcenter.soa.internal.client.model.PropertyBoolImpl;

public class EncryptFilesTask extends FileProcessTask {
	
	private final RightsManager rightsManager;
	
	private List<String> lstNRExclude;
	private Map<String, String> lstAttribute; // attribute name & default value
	
	public EncryptFilesTask(Dataset dataset, DataManagementService dmService, 
			ReservationService rsvService, SessionService sesService, 
			FileManagementUtility fmUtil, RefInfoService refInfoService, 
			VolInfoService volInfoService, RightsManager rightsManager, 
			String stagingFoldPath, AtomicLong totalFilesSize, 
			List<String> lstNRExclude, Map<String, String> lstAttribute) throws Exception {
		super(dataset, dmService, rsvService, sesService, 
				fmUtil, refInfoService, volInfoService, 
				stagingFoldPath, totalFilesSize);
		
		this.rightsManager = rightsManager;
		this.lstNRExclude = lstNRExclude;
		this.lstAttribute = lstAttribute;
	}
	
	@Override
	protected boolean filterFileToDownload(String fileExtension, String namedReference) {
		if (!fileExtension.equalsIgnoreCase(NXL_FILE_EXT) &&
				!lstNRExclude.contains(namedReference))
			return true;
		
		return false;
	}
	
	
	@Override
	protected void processFiles() {
		long startTime = System.nanoTime();
		
		LOGGER.info(prefix + " encryptFiles()");
		uploadQueue = new LinkedList<>();
		File outputFolder = new File(stagingFoldPath + File.separator + dataset.getUid() + "\\out");
		
		try {
			if (!outputFolder.mkdirs() && !outputFolder.exists()) {
				LOGGER.error(prefix + " encryptFiles() failed to create folders: " + outputFolder.getAbsolutePath());
				return;
			}
			
			if (!outputFolder.canRead() || !outputFolder.canWrite()) {
				LOGGER.error(prefix + " encryptFiles() failed to read/write folders: " + outputFolder.getAbsolutePath());
				return;
			}
		} catch (SecurityException se) {
			LOGGER.error(prefix + " encryptFiles() create folders caught exception: " + se.getMessage(), se);
			return;
		}
		
		// setup once and apply to all files of the dataset
		// load metadata		
		Map<String, String[]> tagMap = new HashMap<>();
		
		LOGGER.debug(prefix + "loading metadata...");
		String[] attributesName = lstAttribute.keySet().toArray(new String[lstAttribute.keySet().size()]);
		ServiceData servData = dmService.getProperties(new ModelObject[] {dataset}, attributesName);
		
		if (LOGGER.isTraceEnabled())
			BatchProcessUtility.logServiceData(servData);
		
    	for (String attributeName : attributesName) {
    		String value = "";
    		try {
    			Property property = dataset.getPropertyObject(attributeName);
    			List<String> values = property.getDisplayableValues();
    			
    			value = values.toString();
    			value = StringUtils.stripStart(value, "[");
    			value = StringUtils.stripEnd(value, "]");
    			    			
    			if (property instanceof PropertyBoolImpl) {
    				value = String.valueOf(property.getBoolValue());
    			}
    			
    			// 2.7 Bajaj requirement in short deliver time
    			if (value.trim().equals("") && lstAttribute.get(attributeName) != null) {
    				LOGGER.debug(prefix + " loadmetadata override with default value");
    				value = lstAttribute.get(attributeName);
    			}
    			
    			if (attributeName.equals(NXL_LAST_MOD_USER)) {
    				if (transDataset.getNxlLastModUser() != null && 
    						transDataset.getNxlLastModUser().length() > 0)
    					value = transDataset.getNxlLastModUser();
    			} else if (attributeName.equals(NXL_LAST_MOD_DATE)) {
    				if (transDataset.getNxlLastModDate() != null && 
    						transDataset.getNxlLastModDate().length() > 0) {
    					value = transDataset.getNxlLastModDate();
    				}
    			}
    			// end of Bajaj requirement
    			
    			LOGGER.debug(prefix + " loadmetadata " + attributeName + "=" + value);
    			
    		} catch (NotLoadedException nle) {
    			LOGGER.error(prefix + " loadmetadata caught NotLoadedException for " + attributeName);
    			
    			// if the attribute is not loaded, flag not loaded
    			value = "[NOT LOADED]";
    		} catch (IllegalArgumentException iae) {
    			LOGGER.error(prefix + " loadmetadata caught IllegalArgumentException for " + attributeName);
    			
    			// if the attribute is not exists, don't added the attribute
    			value = "[ILLEGAL ARGS]";
    		}
    		
    		// filter out empty string as RPM cannot handle this properly
    		if (!value.trim().equals("")) {
	    		String[] values = value.split(",");
	    		tagMap.put(attributeName, values);
    		}
    	}
		// end of load metadata
		
		
		while (!processQueue.isEmpty()) {
			TaskFileDTO fileDTO = processQueue.remove();
			File outputFile = new File(outputFolder.getAbsolutePath() + 
					File.separator + appendExtension(fileDTO.getLocalFile().getName()));
			
			try {
				rightsManager.encrypt(fileDTO.getLocalFile().getAbsolutePath(), outputFile.getAbsolutePath(), null, null, tagMap, null, TokenGroupType.TOKENGROUP_SYSTEMBUCKET);
				fileDTO.setLocalFile(outputFile);
				uploadQueue.add(fileDTO);
			} catch (Exception e) {
				LOGGER.error(prefix + " encryptFiles() caught exception: " + e.getMessage(), e);
			}
		}
		
		LOGGER.debug(prefix + " upload queue has " + processQueue.size() + " tasks");
		long duration = System.nanoTime() - startTime;
		reportMsgBuilder.append("encrypt[" + BatchProcessUtility.getTimeFormat(duration) + "] ");
	}
	
	@Override
	protected boolean isNXLProtected() {
		return true;
	}

}
