package com.nextlabs.drm.rmx.batchprotect;

/**
 * Created on December 2, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import static com.nextlabs.drm.rmx.batchprotect.configuration.BatchProtectConstants.*;
import com.nextlabs.drm.rmx.batchprotect.configuration.NextLabsAuthorizationAgent;
import com.nextlabs.drm.rmx.batchprotect.configuration.NextLabsPolicyConstants;
import com.nextlabs.drm.rmx.batchprotect.configuration.TeamcenterPreferences;
import com.nextlabs.drm.rmx.batchprotect.tcquery.QueryManager;
import com.nextlabs.drm.rmx.batchprotect.tcsession.TCSession;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;
import com.teamcenter.services.loose.core.DataManagementService;
import com.teamcenter.services.loose.core.DispatcherManagementService;
import com.teamcenter.services.loose.core.ReservationService;
import com.teamcenter.services.loose.core._2007_06.DataManagement.RelationAndTypesFilter;
import com.teamcenter.services.loose.core._2007_09.DataManagement.ExpandGRMRelationsData2;
import com.teamcenter.services.loose.core._2007_09.DataManagement.ExpandGRMRelationsOutput2;
import com.teamcenter.services.loose.core._2007_09.DataManagement.ExpandGRMRelationsPref2;
import com.teamcenter.services.loose.core._2007_09.DataManagement.ExpandGRMRelationsResponse2;
import com.teamcenter.services.loose.core._2007_09.DataManagement.ExpandGRMRelationship;
import com.teamcenter.services.loose.core._2008_06.DispatcherManagement.CreateDispatcherRequestArgs;
import com.teamcenter.services.loose.core._2008_06.DispatcherManagement.CreateDispatcherRequestResponse;
import com.teamcenter.services.loose.core._2008_06.DispatcherManagement.KeyValueArguments;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.Dataset;
import com.teamcenter.soa.client.model.strong.DocumentRevision;
import com.teamcenter.soa.client.model.strong.ImanFile;
import com.teamcenter.soa.client.model.strong.ItemRevision;
import com.teamcenter.soa.exceptions.NotLoadedException;

public class BatchProtect {
	
	private static final Logger logger = LogManager.getLogger("BPLOGGER");
	
	private NextLabsAuthorizationAgent agent;
	private TeamcenterPreferences tcPreferences;
	private String[] evaluationAttrs;
	private String tc_user;
	private TCSession session;
	
	public BatchProtect(String tc_user_name, TCSession tc_session) {
		try {
			tc_user = tc_user_name;
			session = tc_session;
			tcPreferences = new TeamcenterPreferences(session);
			logger.info("teamcenter preference object created");
			evaluationAttrs = tcPreferences.getEvaAttrArray();
			logger.info("NXL_Evaluation_Attributes: {}", Arrays.toString(evaluationAttrs));
			
			agent = NextLabsAuthorizationAgent.getInstance(tcPreferences.getPdpHost(), tcPreferences.getPdpPort(),
					tcPreferences.getPdpHttps(), tcPreferences.getPdpIgnoreHttps(),
					tcPreferences.getOAuth2Host(), tcPreferences.getOAuth2Port(),
					tcPreferences.getOAuth2Https(), tcPreferences.getOAuth2ClientID(),
					tcPreferences.getOAuth2ClientSecret(),
					tcPreferences.getPdpDefaultAction(), tcPreferences.getPdpDefaultMessage());
			logger.info("successfully loaded agent!");

		} catch (Exception ex) {
			logger.error("BatchProtect.BatchProtect() caught exception: {}", ex.getMessage(), ex);
			evaluationAttrs = new String[0];
		}
	}

	public void execute(TCSession session, String queryName, String queryPropFilepath, 
			boolean dryrun, boolean forceUncheckout, String nxlaction) throws Exception {
		Properties queryProps = BatchProtectUtility.loadPropertiesFromFile(queryPropFilepath);

		QueryManager queryManager = new QueryManager(session);
		ModelObject[] modelObjects = queryManager.query(queryName, queryProps);

		DispatcherManagementService dms = DispatcherManagementService.getService(session.getConnection());
		DataManagementService dmService = DataManagementService.getService(session.getConnection());
		ReservationService rService = ReservationService.getService(session.getConnection());

		ExpandGRMRelationsPref2 expandPref = setupExpandGRMRelationsPref2();

		if (modelObjects != null) {
			ArrayList<CreateDispatcherRequestArgs> lstRequestArg = new ArrayList<CreateDispatcherRequestArgs>();

			for (int i = 0; i < modelObjects.length; i++) {
				try {
					ModelObject modelObject = modelObjects[i];

					dmService.refreshObjects(new ModelObject[] { modelObject });
					String checkoutStatus = getCheckoutStatus(modelObject);
					String activeSeq = modelObject.getPropertyDisplayableValue("active_seq"); 
					
					if (activeSeq.equals("0")) {
						logger.info("Skip processing <{}> {} [{}] [{}] [{}] ({})", 
								modelObject.getTypeObject().getName(),
								modelObject.getPropertyDisplayableValue(PROP_OBJECT_NAME),
								modelObject.getPropertyDisplayableValue(PROP_OBJECT_TYPE),
								modelObject.getUid(), activeSeq, checkoutStatus);
						
						continue;
					}
					
					// Bajaj new requirement for 2.7.1
					if (forceUncheckout && checkoutStatus.equals("Y")) {
						/*
						 * document revision cannot be unchecked-out at the
						 * revision level first, it has to be done at the
						 * dataset level otherwise the dataset will be stuck at
						 * check-out state and cannot be cancel/check-in (it
						 * says dataset is not check-out, most likely this
						 * checking is on document revision) workaround: do a
						 * check-out at document revision again (it will say
						 * dataset have been check-out, ignore the msg) do a
						 * check-in again will resolve this state stuck problem.
						 */
						// DocumentRevision solution: don't do unchecked-out for
						// DocumentRevision, leave it do dataset level
						if (!(modelObject instanceof DocumentRevision)) {
							// refresh the parent of dataset
							// workaround for the 2nd cancelcheckout for dataset of DocumentRevision
							if (modelObject instanceof Dataset) {
								ModelObject[] parents = expandGRMRelationsForDataset(dmService, modelObject, expandPref);
								dmService.refreshObjects(parents);
							}
							
							forceUncheckout(rService, modelObject);

							dmService.refreshObjects(new ModelObject[] { modelObject });
							checkoutStatus = getCheckoutStatus(modelObject);
						}
					}

					// Support Item Revision and Dataset
					if (modelObject instanceof Dataset && 
							!checkoutStatus.equals("Y")) {
						if (!validDatasetToProcess(dmService, modelObject))
							continue;
						if (nxlaction.equalsIgnoreCase(ACT_UNPROTECT) && 
								!policyEvaluation(modelObject, nxlaction))
							continue;
						
						lstRequestArg.add(createDispatcherRequestArgs(modelObject, checkoutStatus, nxlaction));
						// fix bug 39910
					} else if ((modelObject instanceof DocumentRevision) || 
							(modelObject instanceof ItemRevision && !checkoutStatus.equals("Y"))) {						
						// expand the Item Revision to retrieve related dataset
						ModelObject[] moDatasets = expandGRMRelationsForItemRevision(dmService, modelObject, expandPref);
						dmService.refreshObjects(moDatasets);

						String checkoutStatusDataset;
						for (ModelObject moDataset : moDatasets) {
							checkoutStatusDataset = getCheckoutStatus(moDataset);

							// Bajaj new requirement for 2.7.1
							// DocumentRevision solution: uncheck-out at dataset
							// level, it will auto uncheck-out the
							// DocumentRevision
							if (forceUncheckout && checkoutStatusDataset.equals("Y")) {
								forceUncheckout(rService, moDataset);

								dmService.refreshObjects(new ModelObject[] { moDataset });
								checkoutStatusDataset = getCheckoutStatus(moDataset);
							}

							if (checkoutStatusDataset.equals("Y")) {
								// fixed bug 38498
								logger.info("Couldn't process <Dataset> {} [{}] [{}] [{}] ({})",
										moDataset.getPropertyDisplayableValue(PROP_OBJECT_NAME),
										moDataset.getPropertyDisplayableValue(PROP_OBJECT_TYPE),
										moDataset.getUid(), activeSeq, checkoutStatus);
							} else {
								if (!validDatasetToProcess(dmService, moDataset))
									continue;
								
								if (nxlaction.equalsIgnoreCase(ACT_UNPROTECT) && 
										!policyEvaluation(moDataset, nxlaction))
									continue;
								
								lstRequestArg.add(createDispatcherRequestArgs(moDataset, modelObject, checkoutStatus, nxlaction));
							}
						}
					} else {
						logger.info("Skip processing <{}> {} [{}] [{}] [{}] ({})", 
								modelObject.getTypeObject().getName(),
								modelObject.getPropertyDisplayableValue(PROP_OBJECT_NAME),
								modelObject.getPropertyDisplayableValue(PROP_OBJECT_TYPE),
								modelObject.getUid(), activeSeq, checkoutStatus);
					}
				} catch (NotLoadedException e) {
					logger.error("BatchProtect.protect() caught exception: {}", e.getMessage());
					logger.error("Stack trace: ", e);

					throw e;
				}
			} // end of loop for modelObjects

			// validate whether the request args is still valid after cancel
			// checkout
			if (forceUncheckout) {
				logger.info("");
				logger.info("Validate dataset after force uncheckout...");
				logger.info("Dispatcher request args size before validation is {}", lstRequestArg.size());

				Iterator<CreateDispatcherRequestArgs> iCDRArgs = lstRequestArg.iterator();
				while (iCDRArgs.hasNext()) {
					CreateDispatcherRequestArgs createDispatcherRequestArgs = iCDRArgs.next();
					ModelObject[] priObj = createDispatcherRequestArgs.primaryObjects;
					// need to load secondary obj as well, cause Vis_Session
					// does not create new version of the dataset
					// if we only check primary obj, will have duplicate request
					// for a dataset like Vis_Session
					// one of the request will point to invalid secondary obj
					ModelObject[] secObj = createDispatcherRequestArgs.secondaryObjects;

					ServiceData reloadDataPri = dmService.loadObjects(new String[] { priObj[0].getUid() });
					ServiceData reloadDataSec = null;

					if (reloadDataPri.sizeOfPartialErrors() > 0) {
						logger.debug("   Remove dispatcher request args for {}", priObj[0].getUid());
						iCDRArgs.remove();
						// if primary obj valid, test secondary obj
					} else if (secObj.length > 0 && secObj[0] != null) {
						reloadDataSec = dmService.loadObjects(new String[] { secObj[0].getUid() });

						if (reloadDataSec.sizeOfPartialErrors() > 0) {
							logger.debug("   Remove dispatcher request args for {}; {}", priObj[0].getUid(), secObj[0].getUid());
							iCDRArgs.remove();
						}
					}

					if (logger.isTraceEnabled()) {
						BatchProtectUtility.logServiceData(reloadDataPri);

						if (reloadDataSec != null)
							BatchProtectUtility.logServiceData(reloadDataSec);
					}
				}

				logger.info("Dispatcher request args size after validation is {}", lstRequestArg.size());
				logger.info("");
			}
			
			CreateDispatcherRequestArgs[] requestArgs = new CreateDispatcherRequestArgs[lstRequestArg.size()];
			requestArgs = lstRequestArg.toArray(requestArgs);
			
			if (!dryrun && requestArgs.length > 0) {
				try {
					logger.info("Creating translation requests ...");
					CreateDispatcherRequestResponse createDispReqResponse = dms.createDispatcherRequest(requestArgs);
					BatchProtectUtility.logServiceData(createDispReqResponse.svcData);
				} catch (ServiceException e) {
					logger.error("BatchProtect.protect() caught exception: {}", e.getMessage());
					logger.error("Stack trace: ", e);

					throw e;
				}
			} else if(dryrun) {
				logger.info("Dry run completed without creating translation request");
			}
		}
	}

	/*
	 * validate whether the dataset need to be processed 1. check whether the
	 * dataset has any file 2. check whether the file(s) is 0kb
	 */
	private boolean validDatasetToProcess(DataManagementService dmService, ModelObject dataset) {
		dmService.refreshObjects(new ModelObject[] { dataset });

		logger.trace("Retrieving dataset properties ...");
		ServiceData serviceData = dmService.getProperties(
				new ModelObject[] { dataset }, PROPS_DATASET);

		if (logger.isTraceEnabled())
			BatchProtectUtility.logServiceData(serviceData);

		ModelObject[] lstModelObject = null;
		try {
			lstModelObject = dataset.getPropertyObject("ref_list").getModelObjectArrayValue();
		} catch (NotLoadedException e) {
			logger.error("BatchProtect.validDatasetToProcess() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
		} catch (Exception e) {
			logger.error("BatchProtect.validDatasetToProcess() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
		}

		if (lstModelObject == null || lstModelObject.length <= 0) {
			return false;
		}

		boolean areAllFilesZeroSize = true;
		dmService.refreshObjects(lstModelObject);

		for (ModelObject modelObj : lstModelObject) {
			if (modelObj instanceof ImanFile) {
				serviceData = dmService.getProperties(
						new ModelObject[] { modelObj }, PROPS_IMANFILE);

				if (logger.isTraceEnabled())
					BatchProtectUtility.logServiceData(serviceData);

				try {
					logger.debug("  {} byte_size: {}", modelObj.getUid(), 
							modelObj.getPropertyDisplayableValue("byte_size"));
					if (Long.parseLong(modelObj.getPropertyDisplayableValue("byte_size")) > 0)
						areAllFilesZeroSize = false;
				} catch (NotLoadedException e) {
					logger.error("BatchProtect.validDatasetToProcess() caught exception: {}", e.getMessage());
					logger.error("Stack trace: ", e);
				} catch (NumberFormatException e) {
					// fix the bug when FSC cannot find the file
					// FSC proxy error: "FSC_Send error on getFileStat step
					// @/stat2/file/nvargs/ , <fsc_error> -9009 : NO_ROUTE_3
					logger.trace("BatchProtect.validDatasetToProcess(): Failed to retrieve byte_size for {}", modelObj.getUid());
					logger.trace("Stack trace: ", e);
				} catch (Exception e) {
					logger.trace("Stack trace: ", e);
				}
			}
		}
		
		if (areAllFilesZeroSize)
			return false;
		else
			return true;
	}

	private ExpandGRMRelationsPref2 setupExpandGRMRelationsPref2() {
		RelationAndTypesFilter typeFilter = new RelationAndTypesFilter();
		ExpandGRMRelationsPref2 expandPref = new ExpandGRMRelationsPref2();

		expandPref.expItemRev = false;
		expandPref.returnRelations = true;
		expandPref.info = new RelationAndTypesFilter[] { typeFilter };

		return expandPref;
	}

	private ModelObject[] expandGRMRelationsForItemRevision(DataManagementService dmService, 
			ModelObject itemRevision, ExpandGRMRelationsPref2 expandPref) {
		// add dataset to a list, then retrieve the properties for those
		// datasets
		// considering retrieving properties from TC is more expensive than
		// doing looping twice
		List<ModelObject> lstDataset = new ArrayList<>();

		try {
			logger.info("Expanding Item Reivsion {} ...", itemRevision.getPropertyDisplayableValue("object_string"));
		} catch (NotLoadedException e) {

		}
		
		ExpandGRMRelationsResponse2 response = dmService.expandGRMRelationsForPrimary(new ModelObject[] { itemRevision }, expandPref);
		for (ExpandGRMRelationsOutput2 output : response.output) {
			for (ExpandGRMRelationsData2 data : output.relationshipData) {
				for (ExpandGRMRelationship relationship : data.relationshipObjects) {
					// ModelObject revisionItem = output.inputObject;
					ModelObject otherSideObj = relationship.otherSideObject;
					
					if (otherSideObj instanceof Dataset) {
						lstDataset.add(otherSideObj);
					}
				}
			}
		}

		// retrieve the properties from TC for those datasets
		ModelObject[] moDatasets = new ModelObject[lstDataset.size()];
		moDatasets = lstDataset.toArray(moDatasets);

		logger.trace("Retrieving model objects properties ...");
		ServiceData sd = dmService.getProperties(moDatasets, PROPS_MODELOBJS);
		if (logger.isTraceEnabled())
			BatchProtectUtility.logServiceData(sd);

		return moDatasets;
	}
	
	private ModelObject[] expandGRMRelationsForDataset(DataManagementService dmService, 
			ModelObject dataset, ExpandGRMRelationsPref2 expandPref) {
		List<ModelObject> lstItemRevision = new ArrayList<>();
		
		try {
			logger.info("Expanding Dataset {} ...", dataset.getPropertyDisplayableValue("object_string"));
		} catch (NotLoadedException e) {
			
		}
		
		ExpandGRMRelationsResponse2 response = dmService.expandGRMRelationsForSecondary(new ModelObject[] { dataset }, expandPref);
		for (ExpandGRMRelationsOutput2 output : response.output) {
			for (ExpandGRMRelationsData2 data : output.relationshipData) {
				for (ExpandGRMRelationship relationship : data.relationshipObjects) {
					ModelObject otherSideObj = relationship.otherSideObject;
					
					if (otherSideObj instanceof DocumentRevision)
						lstItemRevision.add(otherSideObj);
				}
			}
		}
		
		ModelObject[] moItemRevisions = new ModelObject[lstItemRevision.size()];
		moItemRevisions = lstItemRevision.toArray(moItemRevisions);
		
		logger.trace("Retrieving model objects properties ...");
		ServiceData sd = dmService.getProperties(moItemRevisions, PROPS_MODELOBJS);		
		if (logger.isTraceEnabled())
			BatchProtectUtility.logServiceData(sd);
		
		return moItemRevisions;
	}

	private CreateDispatcherRequestArgs createDispatcherRequestArgs(ModelObject primaryObject, String checkoutStatus, String nxlaction)
			throws NotLoadedException {
		return createDispatcherRequestArgs(primaryObject, null, checkoutStatus, nxlaction);
	}

	private CreateDispatcherRequestArgs createDispatcherRequestArgs(ModelObject primaryObject, ModelObject secondaryObject,
			String checkoutStatus, String nxlaction) throws NotLoadedException {
		logger.info("Creating translation request arguments to {} <Dataset> {} [{}] [{}] ({})", 
				nxlaction, primaryObject.getPropertyDisplayableValue(PROP_OBJECT_NAME),
				primaryObject.getPropertyDisplayableValue(PROP_OBJECT_TYPE),
				primaryObject.getUid(), checkoutStatus);

		CreateDispatcherRequestArgs requestArgs = new CreateDispatcherRequestArgs();
		requestArgs.primaryObjects = new ModelObject[] { primaryObject };

		// support apply protection on dataset (with/without attached to Item
		// Revision)
		if (secondaryObject != null) {
			requestArgs.secondaryObjects = new ModelObject[] { secondaryObject };
		}

		requestArgs.keyValueArgs = new KeyValueArguments[1];
		requestArgs.keyValueArgs[0] = new KeyValueArguments();
		requestArgs.keyValueArgs[0].key = ARG_NXL_ACTION;
		requestArgs.keyValueArgs[0].value = nxlaction;

		requestArgs.providerName = DSF_PROVIDER;
		requestArgs.serviceName = DSF_SERVICE;
		requestArgs.priority = DSF_PRIORITY;

		return requestArgs;
	}

	private String getCheckoutStatus(ModelObject modelObject) throws NotLoadedException {
		String checkoutStatus = DEFAULT_CHECKOUT_STATUS;

		if (modelObject.getPropertyDisplayableValues(PROP_CHECKED_OUT).size() > 0) {
			checkoutStatus = modelObject.getPropertyDisplayableValues(PROP_CHECKED_OUT).get(0);
		}

		return checkoutStatus;
	}

	// Bajaj new requirement for 2.7.1
	// force uncheckout will cause issue to those users who are working on
	// checkout dataset
	// can be further improved to cancel modelobject in batch
	// require big changes to code flow, will be redesign in 3.0
	private void forceUncheckout(ReservationService rService, ModelObject modelObject) {
		try {
			logger.info("  Force uncheckout {} [{}] [{}]", 
					modelObject.getPropertyDisplayableValue(PROP_OBJECT_NAME), 
					modelObject.getPropertyDisplayableValue(PROP_OBJECT_TYPE),
					modelObject.getUid());
		} catch (NotLoadedException e) {
			logger.error("BatchProtect.forceUncheckout() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
		}

		ServiceData sd = rService.cancelCheckout(new ModelObject[] { modelObject });
		if (sd.sizeOfPartialErrors() > 0)
			BatchProtectUtility.logServiceData(sd);

		logger.info("  Force uncheckout is complete");
	}

	private boolean policyEvaluation(ModelObject primaryObject, String nxlaction) throws Exception {
		// policy evaluation for remove protection
		HashMap<String, Object> attributes = new HashMap<String, Object>();
		logger.debug("  Policy evaluation for remove protection of dataset {}", 
				primaryObject.getPropertyDisplayableValue(PROP_OBJECT_NAME));
		
		for (String evalAttribute : evaluationAttrs) {
			List<String> lstValue = primaryObject.getPropertyDisplayableValues(evalAttribute);
			if (lstValue != null && lstValue.size() > 0) {
				if (lstValue.size() == 1) {
					logger.debug("  evalAttribute=={} ;value=={}", evalAttribute, lstValue.get(0));
					attributes.put(evalAttribute, lstValue.get(0));
				} else if (lstValue.size() > 1) {
					if (logger.isDebugEnabled()) {
						StringBuilder sb = new StringBuilder();
						for (String str : lstValue)
							sb.append(str + " ");
						logger.debug("  evalAttribute=={} ;value=={}", evalAttribute, sb);
					}
					
					attributes.put(evalAttribute, lstValue);
				}
			}
		}

		HashMap<String, Object> hasAccessResult = agent.hasAccess(tc_user, NextLabsPolicyConstants.RIGHT_EXECUTE_REMOVEPROTECTION, attributes,
				new HashMap<String, Object>(), new HashMap<String, Object>());

		String response = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_KEY);
		String sError = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_ERROR_KEY);

		// Checking to see if there is SDK error response
		if (null != sError) {
			String sDefaultMsg = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_MESSAGE_KEY);
			
			if (null != sDefaultMsg) {
				logger.error("CE SDK error: {}", sDefaultMsg);
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
				logger.debug("  obligation name {} ={}", i, obligationName);
				
				if (obligationName != null && 
						obligationName.equalsIgnoreCase(NextLabsPolicyConstants.OBLIGATION_REMOVEPROTECTION)) {
					obligationRemoveProtection = true;
					break;
				}
			}
		}
		
		logger.debug("  obligationRemoveProtection==" + obligationRemoveProtection);
		if (!(NextLabsPolicyConstants.RESPONSE_ALLOW_VALUE).equalsIgnoreCase(response) || 
				!obligationRemoveProtection) {
			logger.info("  Remove protection is denied for {}", tc_user);
			return false;
		}

		return true;
	}
	
}
