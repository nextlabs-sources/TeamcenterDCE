package com.nextlabs.drm.rmx.batchprotect.tcquery;

/**
 * Created on December 2, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.util.ArrayList;
import java.util.Map.Entry;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.nextlabs.drm.rmx.batchprotect.BatchProtectUtility;
import com.nextlabs.drm.rmx.batchprotect.configuration.BatchProtectConstants;
import com.nextlabs.drm.rmx.batchprotect.tcsession.TCSession;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;
import com.teamcenter.services.loose.query.SavedQueryService;
import com.teamcenter.services.loose.query._2006_03.SavedQuery.DescribeSavedQueriesResponse;
import com.teamcenter.services.loose.query._2006_03.SavedQuery.SavedQueryFieldListObject;
import com.teamcenter.services.loose.query._2006_03.SavedQuery.SavedQueryFieldObject;
import com.teamcenter.services.loose.query._2007_09.SavedQuery.QueryResults;
import com.teamcenter.services.loose.query._2007_09.SavedQuery.SavedQueriesResponse;
import com.teamcenter.services.loose.query._2008_06.SavedQuery.QueryInput;
import com.teamcenter.services.loose.query._2010_04.SavedQuery.FindSavedQueriesCriteriaInput;
import com.teamcenter.services.loose.query._2010_04.SavedQuery.FindSavedQueriesResponse;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.Dataset;
import com.teamcenter.soa.client.model.strong.ItemRevision;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.soa.exceptions.NotLoadedException;

public class QueryManager {
	
	private final static Logger logger = LogManager.getLogger("BPLOGGER");
	private TCSession tcSession;
	
	public QueryManager(TCSession sessionIn) {
		this.tcSession = sessionIn;
	}
	
	public ModelObject[] query(String queryName, Properties queryProps) {
		ArrayList<ModelObject> lstModelObject = new ArrayList<ModelObject>();
		ModelObject[] modelObjects = null;
		ModelObject query = null;
		SavedQueryFieldObject[] defaultFieldObjs = null;

		// Get the service stub
		SavedQueryService queryService = SavedQueryService.getService(tcSession.getConnection());
		DataManagementService dmService = DataManagementService.getService(tcSession.getConnection());
		
		try {
			// Search the saved query
			logger.info("Searching saved query [{}] ...", queryName);
			
			FindSavedQueriesCriteriaInput queryCriteriaInput = new FindSavedQueriesCriteriaInput();
			queryCriteriaInput.queryNames = new String[] { queryName };
			FindSavedQueriesResponse queryResponse = queryService.findSavedQueries(
					new FindSavedQueriesCriteriaInput[] { queryCriteriaInput } );
			
			if (queryResponse.savedQueries.length <= 0) {
				logger.info("Cannot find saved query [{}] in the system.", queryName);
				
				return null;
			}
			
			// Found the saved query
			logger.info("Found {} saved query [{}] ...", queryResponse.savedQueries.length, queryName);
			
			// Take the first one from the list, if there is multiple
			query = queryResponse.savedQueries[0];
			try {
				dmService.getProperties(new ModelObject[] {query}, new String[] {"query_name"});
				
				if (!query.getPropertyDisplayableValue("query_name").equals(queryName)) {
					logger.info("Cannot find saved query [{}] in the system.", queryName);
					
					return null;
				}
			} catch (NotLoadedException e) {
				logger.error("Failed to load the query_name for the searched query");
				
				return null;
			} catch (Exception e) {
				logger.error("Failed to load the query_name for the searched query: {}", e.getMessage());
				logger.error("Stack trace: ", e);
				
				return null;
			}
			
			// Fix bug 38338
			// force refresh on the query object
			dmService.refreshObjects(new ModelObject[] {query});
						
			// Fix bug 38244
			// load default search parameter and value
			logger.debug("Describe saved query [{}] ...", queryName);
			DescribeSavedQueriesResponse dsqResponse = queryService.describeSavedQueries(new ModelObject[] {query});
			SavedQueryFieldListObject[] defaultFieldListObjs = dsqResponse.fieldLists;
			
			logger.debug("  describe saved query has {} result", defaultFieldListObjs.length);
			if (defaultFieldListObjs.length > 0) {
				defaultFieldObjs = defaultFieldListObjs[0].fields;
				logger.debug("  Saved query has {} field object", defaultFieldObjs.length);
				
				if (logger.isDebugEnabled()) {
					for (SavedQueryFieldObject fieldObj : defaultFieldObjs) {
						logger.debug("    > {} {}{}{}", fieldObj.logicalOperation, 
								fieldObj.entryName, fieldObj.mathOperation, fieldObj.value);
						
					}
				}
			}
		} catch (ServiceException e) {
			logger.error("QueryManager.query() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			return null;
		} catch (Exception e) {
			logger.error("QueryManager.query() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			return null;
		}

		if (defaultFieldObjs == null || defaultFieldObjs.length <= 0) {
			logger.info("There is no search field for saved query [{}] in the system.", queryName);
			
			return null;
		}

		try {
			// Search for all Items, returning a maximum of ? objects
			QueryInput savedQueryInput[] = new QueryInput[1];
			savedQueryInput[0] = new QueryInput();
			savedQueryInput[0].query = query;
			savedQueryInput[0].maxNumToReturn = BatchProtectConstants.DEFAULT_MAX_NUM_TO_RETURN;
			savedQueryInput[0].limitList = new ModelObject[0];
			
			if (queryProps.containsKey("maxNumToReturn")) {
				logger.debug("Setting maxNumToReturn to {}", queryProps.getProperty("maxNumToReturn"));
				savedQueryInput[0].maxNumToReturn = Integer.parseInt(
						queryProps.getProperty("maxNumToReturn"));
				queryProps.remove("maxNumToReturn");
			}
			
			logger.debug("Size of properties loaded from file: {}", queryProps.size());
			if (logger.isDebugEnabled()) {
				for (Entry<Object, Object> x : queryProps.entrySet()) {
					String key = String.valueOf(x.getKey());
					String value = String.valueOf(x.getValue());
					
					logger.debug("    > {}={}", key, value);
				}
			}
			
			ArrayList<String> lstEntry = new ArrayList<String>();
			ArrayList<String> lstValue = new ArrayList<String>();
			
			logger.debug("Overwriting the default value of query fields based on the input from the properties file");
			for (int i = 0; i < defaultFieldObjs.length; i++) {
				String fieldEntryName = defaultFieldObjs[i].entryName;
				
				// searching does the field is set in properties file
				if (queryProps.containsKey(fieldEntryName)) {
					// if properties file set the field value to empty, remove that field from query
					// to handle date issue and also follow RAC way to handle empty value
					if (queryProps.getProperty(fieldEntryName).trim().equals(""))
						continue;
					
					// overwrite the default value
					defaultFieldObjs[i].value = queryProps.getProperty(fieldEntryName);
					logger.debug("  Overwriting the default value of {} with {}", 
							fieldEntryName, queryProps.getProperty(fieldEntryName));
				}
				
				lstEntry.add(fieldEntryName);
				lstValue.add(String.valueOf(defaultFieldObjs[i].value));
				
				logger.info("    Query properties [{}] {}={}", 
						i, fieldEntryName, String.valueOf(defaultFieldObjs[i].value));
				// for those properties in properties file that does not match search file are ignore
			}
			
			savedQueryInput[0].entries = lstEntry.toArray(new String[0]);
			savedQueryInput[0].values = lstValue.toArray(new String[0]);

			// Execute the service operation
			logger.info("Executing saved query [{}] ...", queryName);
			SavedQueriesResponse savedQueryResult = queryService
					.executeSavedQueries(savedQueryInput);
						
			QueryResults found = savedQueryResult.arrayOfResults[0];

			logger.info("Saved query [{}] found {} items.", 
					queryName, found.objectUIDS.length);

			// Page through the results 10 at a time
			for (int i = 0; i < found.objectUIDS.length; i += 10) {
				int pageSize = (i + 10 < found.objectUIDS.length) ? 10
						: found.objectUIDS.length - i;

				String[] uids = new String[pageSize];
				for (int j = 0; j < pageSize; j++) {
					uids[j] = found.objectUIDS[i + j];
					logger.debug("  Loop [{}] going to load object for {}", i, uids[j]);
				}

				ServiceData sd = dmService.loadObjects(uids);
				
				for (int k = 0; k < sd.sizeOfPlainObjects(); k++) {
					ModelObject modObj = sd.getPlainObject(k);
					
					logger.info("className: {} {} {}", 
							modObj.getTypeObject().getParent().getClassName(),
							modObj.getTypeObject().getClassName(),
							(modObj instanceof ItemRevision));
					if (modObj instanceof Dataset || 
							modObj instanceof ItemRevision) {
						lstModelObject.add(modObj);
					}
				}
			}
			
			modelObjects = new ModelObject[lstModelObject.size()];
			modelObjects = lstModelObject.toArray(modelObjects);
			
			// Loading properties for found model objects 
			logger.debug("Loading model object's properties for query reponse objects ...");
			ServiceData serviceData = dmService.getProperties(modelObjects, 
					BatchProtectConstants.PROPS_MODELOBJS);
			logger.debug("Loading model object's properties for query reponse objects is complete");
			
			if (logger.isTraceEnabled())
				BatchProtectUtility.logServiceData(serviceData);
		} catch (Exception e) {
			logger.error("QueryManager.query() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			return null;
		}

		return modelObjects;
	}
	
}
