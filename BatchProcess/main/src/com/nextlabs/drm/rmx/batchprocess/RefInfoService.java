package com.nextlabs.drm.rmx.batchprocess;

import static com.nextlabs.drm.rmx.batchprocess.configuration.BatchProcessConstants.DOT_NXL_FILE_EXT;
import static com.nextlabs.drm.rmx.batchprocess.configuration.BatchProcessConstants.INTEGRATION_NR_LOOKUP;

import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.core._2007_06.DataManagement.DatasetTypeInfoResponse;
import com.teamcenter.services.strong.core._2007_06.DataManagement.ReferenceInfo;
import com.teamcenter.soa.client.Connection;

public class RefInfoService {
	
	private final DataManagementService dmService;
	private final ConcurrentHashMap<String, 
		ConcurrentHashMap<String, CopyOnWriteArrayList<ReferenceInfo>>> refInfoLookup;	// dataset type as key
	
	private final static Logger LOGGER = LogManager.getLogger("RISVRLOGGER");
	
	public RefInfoService(DataManagementService dmService) {
		this.dmService = dmService;
		refInfoLookup = new ConcurrentHashMap<>();
	}
	
	public RefInfoService(Connection connection) {
		this(DataManagementService.getService(connection));
	}
	
	/**
	 * To do: need to consider file format
	 * @param datasetType
	 * @param fileExtension
	 * @return
	 */
	public ReferenceInfo getNamedReference(String datasetType, String fileExtension) {
		long startTime = System.nanoTime();
		
		ConcurrentHashMap<String, CopyOnWriteArrayList<ReferenceInfo>> lookup;
		List<ReferenceInfo> lstRefInfo = null;
		
		if (refInfoLookup.containsKey(datasetType))
			lookup = refInfoLookup.get(datasetType);
		else {
			LOGGER.info("lookup missed [" + datasetType + "]");
			lookup = getDatasetTypeInfo(datasetType);
			refInfoLookup.putIfAbsent(datasetType, lookup);
		}
		
		// actual match
		if (lookup.containsKey(fileExtension.toLowerCase()))
			lstRefInfo = lookup.get(fileExtension.toLowerCase());
		// search for *
		else if (lookup.containsKey(".*"))
			lstRefInfo = lookup.get(".*");
		
		long duration = System.nanoTime() - startTime;
		LOGGER.info("getNamedReference[" + datasetType + ", " + fileExtension +"] completed in " + BatchProcessUtility.getTimeFormat(duration));

		if (lstRefInfo != null && lstRefInfo.size() > 0) {
			if (fileExtension.equalsIgnoreCase(DOT_NXL_FILE_EXT)) {
				// need to check the integration named reference
				for (ReferenceInfo refInfo : lstRefInfo) {
					String refName = refInfo.referenceName;
					LOGGER.info(">> " + refName);
					
					if (INTEGRATION_NR_LOOKUP.contains(refName))
						return refInfo;
				}
			} 
			
			// always return first named reference
			return lstRefInfo.get(0);
		} else
			return null;
	}
	
	private ConcurrentHashMap<String, CopyOnWriteArrayList<ReferenceInfo>> getDatasetTypeInfo(String datasetType) {
		long startTime = System.nanoTime();
		LOGGER.info("retrieving from server [" + datasetType + "]");
		
		ConcurrentHashMap<String, CopyOnWriteArrayList<ReferenceInfo>> lookup = new ConcurrentHashMap<>();
		
		// retrieve refInfo from server
		DatasetTypeInfoResponse dsTypeInfoResp = dmService.getDatasetTypeInfo(new String[] {datasetType});
		
		ReferenceInfo[] namedRefInfos = null;
	    if (dsTypeInfoResp.infos.length > 0)
	    	namedRefInfos = dsTypeInfoResp.infos[0].refInfos;
	    
	    if (namedRefInfos == null || namedRefInfos.length <= 0) {
	    	long duration = System.nanoTime() - startTime;
	    	LOGGER.warn("no named reference info for [" + datasetType + "]");
	    	LOGGER.info("completed in " + BatchProcessUtility.getTimeFormat(duration) + " [" + datasetType +"]");
	    	
	    	return lookup;
	    }
	    
	    // setting up the lookup table for ref info
	    for (ReferenceInfo namedRefInfo : namedRefInfos) {
	    	CopyOnWriteArrayList<ReferenceInfo> innerLstRefInfo;
	    	
	    	String fileExt = namedRefInfo.fileExtension;
	    	int iIndex = fileExt.lastIndexOf(".");
	    	if (iIndex > 0)
	    		fileExt = fileExt.substring(iIndex);
	    	
	    	// standardize * (Text) and .* (Image) to .*
	    	if (fileExt.equalsIgnoreCase("*"))
	    		fileExt = ".*";
	    	
	    	if (lookup.containsKey(fileExt.toLowerCase()))
	    		innerLstRefInfo = lookup.get(fileExt.toLowerCase());
	    	else
	    		innerLstRefInfo = new CopyOnWriteArrayList<>();
	    		
	    	innerLstRefInfo.add(namedRefInfo);
	    	lookup.put(fileExt.toLowerCase(), innerLstRefInfo);
	    }
    	
    	if (LOGGER.isDebugEnabled()) {
    		LOGGER.debug("debug lookup content ");
	    	Iterator<Entry<String, CopyOnWriteArrayList<ReferenceInfo>>> ite = lookup.entrySet().iterator();
	    	
	    	while (ite.hasNext()) {
	    		Entry<String, CopyOnWriteArrayList<ReferenceInfo>> entry = ite.next();
	    		CopyOnWriteArrayList<ReferenceInfo> data = entry.getValue();
	    		
	    		LOGGER.debug(entry.getKey() + " : " + data.size());
	    		for (int z = 0; z < data.size(); z++) {
	    			LOGGER.debug(entry.getKey() + " => " + data.get(z).referenceName);
	    		}
	    	}
    	}
    	
    	long duration = System.nanoTime() - startTime;
    	LOGGER.info("completed in " + BatchProcessUtility.getTimeFormat(duration) + " [" + datasetType +"]");
    	return lookup;
	}
	
	public boolean isBinary(String fileFormat) {
		if (fileFormat.equalsIgnoreCase("BINARY"))
			return true;
		
		return false;
	}

}
