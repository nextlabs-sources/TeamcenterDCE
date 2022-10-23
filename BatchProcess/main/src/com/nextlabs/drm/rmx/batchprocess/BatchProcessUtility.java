package com.nextlabs.drm.rmx.batchprocess;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.nextlabs.drm.rmx.batchprocess.configuration.SizeFormat;
import com.nextlabs.drm.rmx.batchprocess.configuration.TimeFormat;
import com.nextlabs.drm.rmx.batchprocess.dto.DatasetDTO;
import com.nextlabs.drm.rmx.batchprocess.dto.InputObjectDTO;
import com.teamcenter.soa.client.model.ErrorStack;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.ServiceData;

public class BatchProcessUtility {
	
	private final static Logger LOGGER = LogManager.getLogger("BPLOGGER");
	
	/**
	 * log the service data returned from TC service
	 */
	public static void logServiceData(ServiceData serviceData) {
		LOGGER.info("    Size of created objects: " + serviceData.sizeOfCreatedObjects());
		LOGGER.info("    Size of partial errors: " + serviceData.sizeOfPartialErrors());
		LOGGER.info("    Size of plain objects: " + serviceData.sizeOfPlainObjects());
		
		for (int i = 0; i < serviceData.sizeOfPartialErrors(); i++) {
			ErrorStack errStack = serviceData.getPartialError(i);
			int[] errCodes = errStack.getCodes();
			
			for (String errMsg : errStack.getMessages()) {
				LOGGER.info("      Error message: " + errMsg);
			}
			LOGGER.info("      Error codes: " + Arrays.toString(errCodes));
		}
	}
	
	/**
	 * load the input objects from file, exclude duplicate items
	 */
	public static List<InputObjectDTO> loadInputObjectsFromFile(String inputFilePath) {
		Set<InputObjectDTO> setObjects = new HashSet<>();
		
		try (BufferedReader br = new BufferedReader(new FileReader(inputFilePath));) {
			String input = br.readLine();
			while (input != null) {
				if (input.trim().length() > 0) {
					DatasetDTO dto = new DatasetDTO.Builder(input).build();
					
					if (!setObjects.add(dto))
						LOGGER.warn("BatchProcessUtil.loadInputObjectsFromFile() caught" + 
								" duplicated input which will be ignored: " + dto.getUid());
				}
				
				input = br.readLine();
			}
		} catch (Exception e) {
			LOGGER.error("BatchProcessUtil.loadInputObjectsFromFile() caught exception: " + 
					e.getMessage(), e);
			return null;
		}
				
		return new ArrayList<InputObjectDTO>(setObjects);
	}
	
	public static List<String> findUidsNotLoaded(String[] uids, ModelObject[] modelObjects) {
		// if size of uids (unique) and modelObjects (unique) are equal
		// return an empty list
		if (uids.length == modelObjects.length)
			return new ArrayList<String>();
		
		Set<String> setToLoad = new HashSet<>(Arrays.asList(uids));
		Set<String> setLoaded = new HashSet<>();
		
		for (ModelObject mo : modelObjects) {
			setLoaded.add(mo.getUid());
		}
		
		setToLoad.removeAll(setLoaded);
		return new ArrayList<String>(setToLoad);
	}
	
	public static String getTimeFormat(long durationInNS) {
		return getTimeFormat(durationInNS, TimeFormat.MILI_SECONDS);
	}
	
	public static String getSizeFormat(long bytes) {
		return getSizeFormat(bytes, SizeFormat.KILOBYTES);
	}
	
	public static String getTimeFormat(long durationInNS, TimeFormat format) {
		return (durationInNS / format.getDivisor()) + format.getUnit();
	}
	
	public static String getSizeFormat(long bytes, SizeFormat format) {
		return (bytes / format.getDivisor()) + format.getUnit();
	}
	
}
