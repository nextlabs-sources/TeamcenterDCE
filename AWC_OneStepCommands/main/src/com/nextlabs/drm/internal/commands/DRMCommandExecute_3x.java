package com.nextlabs.drm.internal.commands;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import com.google.gwt.core.shared.GWT;
import com.google.gwt.user.client.rpc.AsyncCallback;
import com.nextlabs.drm.internal.config.DRMCommands;
import com.nextlabs.drm.internal.config.NextLabsConstants;
import com.nextlabs.drm.server.policy.NextLabsPolicyConstants;
import com.siemens.splm.clientfx.base.published.IMessageService;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IJSO;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IJsArray;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IModelObject;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IServiceData;
import com.siemens.splm.clientfx.kernel.clientmodel.published.JSOFactory;
import com.siemens.splm.clientfx.kernel.clientmodel.published.PreferenceService;
import com.siemens.splm.clientfx.kernel.soa.published.SOA;
import com.siemens.splm.clientfx.tcui.utils.published.DataManagementService;
import com.siemens.splm.clientfx.tcui.utils.published.DataManagementService.ExpandGRMRelationsData2;
import com.siemens.splm.clientfx.tcui.utils.published.DataManagementService.ExpandGRMRelationsOutput2;
import com.siemens.splm.clientfx.tcui.utils.published.DataManagementService.ExpandGRMRelationsPref2;
import com.siemens.splm.clientfx.tcui.utils.published.DataManagementService.ExpandGRMRelationsResponse2;
import com.siemens.splm.clientfx.tcui.utils.published.DataManagementService.ExpandGRMRelationship;
import com.siemens.splm.clientfx.tcui.utils.published.DataManagementService.RelationAndTypesFilter;

public class DRMCommandExecute implements NextLabsConstants {

	private IMessageService msgService;

	DRMCommandUtilityServiceAsync drmService = (DRMCommandUtilityServiceAsync) GWT.create(DRMCommandUtilityService.class);

	public DRMCommandExecute(IMessageService m_messageService) {
		msgService = m_messageService;
	}

	public void execute(final String user_id, final String drmCommand, List<IModelObject> selectedObjs) {
		final List<IModelObject> supportedObjs = new ArrayList<IModelObject>();

		// this is additional check since commandContextChanged() shall disable
		// command option to unsupported types
		for (IModelObject selectedObj : selectedObjs) {
			try {
				if (selectedObj.getTypeObject().isInstanceOf(TYPE_ITEM_REVISION) || 
						selectedObj.getTypeObject().isInstanceOf(TYPE_DATASET)) {
					supportedObjs.add(selectedObj);
				}
			} catch (Exception ex) {
				msgService.error(ex.getMessage());
			}
		}
		
		// get preferences for relation filter
		AsyncCallback<Map<String, String[]>> prefsCallback = new AsyncCallback<Map<String, String[]>>() {
			
			@Override
			public void onSuccess(Map<String, String[]> response) {
				String[] relation_filters = new String[0];
				
				Iterator<Entry<String, String[]>> iterator = response.entrySet().iterator();
				while (iterator.hasNext()) {
					Entry<String, String[]> entry = iterator.next();
					String name = entry.getKey();
					
					if (name.equalsIgnoreCase(NXL_RELATION_FILTER))
						relation_filters = entry.getValue();
				}
				
				List<RelationAndTypesFilter> lstRelationAndTypeFilter = new ArrayList<RelationAndTypesFilter>();
				for (String relation_filter : relation_filters) {
					RelationAndTypesFilter filter = new RelationAndTypesFilter();
					filter.relationTypeName = relation_filter;
					lstRelationAndTypeFilter.add(filter);
				}
				
				if (lstRelationAndTypeFilter.size() <= 0) {
					// add a dummy filter to select all relationship
					lstRelationAndTypeFilter.add(new RelationAndTypesFilter());
				}

				// option 3
				// expand related dataset
				ExpandGRMRelationsPref2 expandPref = new ExpandGRMRelationsPref2();
		
				expandPref.expItemRev = false;
				expandPref.returnRelations = true;
				expandPref.info = lstRelationAndTypeFilter.toArray(new RelationAndTypesFilter[0]);
		
				for (IModelObject primaryObj : supportedObjs) {
					if (primaryObj.getTypeObject().isInstanceOf(TYPE_ITEM_REVISION)) {
						final String drmCommandInner = drmCommand;
						final String user_idInner = user_id;
						final IModelObject inner_primaryObj = primaryObj;
						final ExpandGRMRelationsPref2 innerExpandPref = expandPref;
						
						// fix bug 47810
						// awc 3.2 cannot retrieve "checked_out", will be null
						DataManagementService.getProperties(new IModelObject[] { inner_primaryObj }, new String[] { "checked_out" },
								new AsyncCallback<IServiceData>() {
									@Override
									public void onSuccess(IServiceData response) {
										// fix bug 35038
										// checking for "checked_out"
										if (inner_primaryObj.getPropertyObject("checked_out").getDisplayValue().equals("Y")) {
											msgService.notify("Invalid state: Selected Item Revision has been checked out.");
										} else {
											DataManagementService.expandGRMRelationsForPrimary(new IModelObject[] { inner_primaryObj },
													innerExpandPref, new AsyncCallback<ExpandGRMRelationsResponse2>() {
		
														@Override
														public void onSuccess(ExpandGRMRelationsResponse2 response) {
															for (ExpandGRMRelationsOutput2 output : response.output) {
																for (ExpandGRMRelationsData2 data : output.relationshipData) {
																	for (ExpandGRMRelationship relationship : data.relationshipObjects) {
																		IModelObject revisionItem = output.inputObject;
																		IModelObject otherSideObj = relationship.otherSideObject;
		
																		if (otherSideObj.getTypeObject().isInstanceOf(TYPE_DATASET)) {
																			processDataset(user_idInner, revisionItem, otherSideObj,
																					drmCommandInner);
		
																		}
																	}
																}
															}
														}
		
														@Override
														public void onFailure(Throwable caught) {
															msgService.notify("Failed to expand the related objects for selected item.");
															msgService.error("DRMCommandExecute.execute() caught exception: " + caught.getMessage());
														}
		
													}); // end of
														// expandGRMRelationsForPrimary
														// function
										}
									}
		
									@Override
									public void onFailure(Throwable caught) {
										msgService.notify("Failed to retrieve checked_out for NextLabs DRM.");
										msgService.error("DRMCommandExecute.execute() caught exception: " + caught.getMessage());
									}
								});
		
						// end of TYPE_ITEM_REVISION condition
					} else if (primaryObj.getTypeObject().isInstanceOf(TYPE_DATASET)) {
						processDataset(user_id, null, primaryObj, drmCommand);
					}
					// end of TYPE_DATASET condition
				} // end of looping supportedObjs
			}
			
			@Override
			public void onFailure(Throwable caught) {
				msgService.notify("Failed to retrieve preferences to instantiate NextLabs Relation Filter.");
				msgService.error("DRMCommandExecute.execute() caught exception: " + 
						caught.getMessage());
			}
			
		};
		
		// get preferences for relation_filter
		initPrefsForExp(prefsCallback);
	}

	private void processDataset(String user_id, IModelObject revisionItem, IModelObject datasetObj, String drmCommand) {
		final IModelObject datasetObjInner = datasetObj;
		final IModelObject revisionItemInner = revisionItem;
		final String drmCommandInner = drmCommand;
		final String user_idInner = user_id;
		DataManagementService.getProperties(new IModelObject[] { datasetObjInner }, new String[] { "checked_out" },
				new AsyncCallback<IServiceData>() {

					@Override
					public void onSuccess(IServiceData response) {
						if (datasetObjInner.getPropertyObject("checked_out").getDisplayValue().equals("Y")) {
							msgService.notify("Invalid state: Selected dataset has been checked out.");
						} else {

							final String ds_uid = datasetObjInner.getUid();
							final IModelObject finalOtherSideObj = datasetObjInner;
							final IModelObject finalRevisionItem = revisionItemInner;

							if (drmCommandInner.equalsIgnoreCase(DRMCommands.UNPROTECT.toString())) {
								// remove protection

								// 1. get preferences to instantiate NextLabsAuthorizationAgent
								AsyncCallback<Map<String, String[]>> prefsCallback = new AsyncCallback<Map<String, String[]>>() {

									@Override
									public void onSuccess(Map<String, String[]> response) {
										String[] evaluationAttrs = new String[0];
										String pdpDefaultAction = "";
										String pdpDefaultMessage = "";
										String pdpHostServerApp = "";
										String pdpHost;
										int pdpPort;

										Iterator<Entry<String, String[]>> iterator = response.entrySet().iterator();
										while (iterator.hasNext()) {
											Entry<String, String[]> entry = iterator.next();
											String name = entry.getKey();

											if (name.equalsIgnoreCase(NXL_EVALUATION_ATTRIBUTES))
												evaluationAttrs = entry.getValue();
											else if (name.equalsIgnoreCase(NXL_PDP_DEFAULT_ACTION))
												pdpDefaultAction = entry.getValue()[0];
											else if (name.equalsIgnoreCase(NXL_PDP_DEFAULT_MESSAGE))
												pdpDefaultMessage = entry.getValue()[0];
											else if (name.equalsIgnoreCase(NXL_PDPHOST_SERVERAPP))
												pdpHostServerApp = entry.getValue()[0];
										}

										if (pdpHostServerApp.indexOf(":") > 0) {
											String[] values = pdpHostServerApp.split(":");

											pdpHost = values[0];
											try {
												pdpPort = Integer.parseInt(values[1]);
											} catch (NumberFormatException nfe) {
												pdpPort = DEFAULT_PDP_PORT;
											}
										} else {
											pdpHost = pdpHostServerApp;
											pdpPort = DEFAULT_PDP_PORT;
										}

										// stores the PDP connection details in data structure
										final PolicyEvaluationRequest request = new PolicyEvaluationRequest(pdpHost, pdpPort,
												pdpDefaultAction, pdpDefaultMessage);
										final String[] evaluationAttrsInner = evaluationAttrs;
										List<String> evaluationAttrsList = new ArrayList<String>();
										List<IModelObject> iModelObjectList = new ArrayList<IModelObject>();

										// adds revision item to the list for properties retrieve
										iModelObjectList.add(finalOtherSideObj);
										if (finalRevisionItem != null)
											iModelObjectList.add(finalRevisionItem);

										// setups the evaluation attribute list for policy use
										for (String arr : evaluationAttrs) {
											evaluationAttrsList.add(arr);
										}

										// adds additional two attributes for internal use
										evaluationAttrsList.add("object_name");
										evaluationAttrsList.add("ref_names");

										// 2. get properties for evaluation calls dataMngt to retrieve the metadata for evaluation
										DataManagementService.getProperties(
												iModelObjectList.toArray(new IModelObject[iModelObjectList.size()]),
												evaluationAttrsList.toArray(new String[evaluationAttrsList.size()]),
												new AsyncCallback<IServiceData>() {

													@Override
													public void onSuccess(IServiceData response) {
														try {
															Map<String, IModelObject> maps = response.getModelObjects();
															HashMap<String, Object> attributes = new HashMap<String, Object>();
															final String ds_object_name = maps.get(ds_uid).getPropertyObject("object_name")
																	.getDisplayValue();
															final int refNamesSize = maps.get(ds_uid).getPropertyObject("ref_names")
																	.getDisplayValues().size();

															// if the dataset has named reference(s)
															if (refNamesSize > 0) {
																for (String attr : evaluationAttrsInner) {
																	String value = maps.get(ds_uid).getPropertyObject(attr)
																			.getDisplayValue();

																	// need to handle multiple values
																	if (value != null && !value.equalsIgnoreCase("")) {
																		attributes.put(attr, value);
																	}
																}

																request.setUser_id(user_idInner);
																request.setAttributes(attributes);
																request.setUserAttributes(new HashMap<String, Object>());
																request.setAppAttributes(new HashMap<String, Object>());

																if (finalOtherSideObj != null) {
																	attributes.put(NextLabsPolicyConstants.ATTRIBUTE_DS_TYPE,
																			finalOtherSideObj.getType());
																	attributes.put(NextLabsPolicyConstants.ATTRIBUTE_DS_NAME,
																			ds_object_name);
																	attributes.put(NextLabsPolicyConstants.ATTRIBUTE_DS_UID,
																			finalOtherSideObj.getUid());
																}

																if (finalRevisionItem != null) {
																	String rev_object_name = maps.get(finalRevisionItem.getUid())
																			.getPropertyObject("object_name").getDisplayValue();

																	attributes.put(NextLabsPolicyConstants.ATTRIBUTE_REV_TYPE,
																			finalRevisionItem.getType());
																	attributes.put(NextLabsPolicyConstants.ATTRIBUTE_REV_NAME,
																			rev_object_name);
																	attributes.put(NextLabsPolicyConstants.ATTRIBUTE_REV_UID,
																			finalRevisionItem.getUid());
																}

																try {
																	// 3. call NextLabsAuthorizationAgent to evaluate
																	AsyncCallback<Boolean> evaResult = new AsyncCallback<Boolean>() {

																		@Override
																		public void onSuccess(Boolean result) {
																			IModelObject[] revItems;

																			if (finalRevisionItem == null) {
																				revItems = new IModelObject[0];
																			} else {
																				revItems = new IModelObject[] { finalRevisionItem };
																			}

																			if (result) {
																				createDispatcherRequest(new IModelObject[] { finalOtherSideObj }, revItems, drmCommandInner);
																			} else {
																				msgService.notify("Remove protection on dataset " + ds_object_name + " is denied.");
																			}
																		}

																		@Override
																		public void onFailure(Throwable caught) {
																			msgService.notify("Failed to evaluate right to remove protection.");
																			msgService.error("DRMCommandExecute.createDispatcherRequest() caught exception: " + 
																					caught.getMessage());
																		}

																	};

																	drmService.policyEvaluation(request, evaResult);
																} catch (Exception ex) {
																	ex.printStackTrace(System.out);
																	msgService.error("DRMCommandExecute.createDispatcherRequest() caught exception: " + 
																			ex.getMessage());
																}
															} else {
																msgService.notify("Dataset " + ds_object_name + " has no file reference.");
															}
														} catch (Exception ex) {
															ex.printStackTrace(System.out);
															msgService.error("DRMCommandExecute.createDispatcherRequest() caught exception: " + 
																			ex.getMessage());
														}
													}

													@Override
													public void onFailure(Throwable caught) {
														msgService.notify("Failed to retrieve properties for selected dataset.");
														msgService.error("DRMCommandExecute.createDispatcherRequest() caught exception: " + 
																caught.getMessage());
													}

												}); // end of getProperties
									}

									@Override
									public void onFailure(Throwable caught) {
										msgService.notify("Failed to retrieve preferences to instantiate NextLabs Authorization Agent.");
										msgService.error("DRMCommandExecute.createDispatcherRequest() caught exception: "
												+ caught.getMessage());
									}

								};

								// 1. get preferences to instantiate NextLabsAuthorizationAgent
								// 2. get properties for evaluation
								// 3. call NextLabsAuthorizationAgent to evaluate
								initPrefsForPDP(prefsCallback);
							} else { // end of remove protection
								// apply protection
								DataManagementService.getProperties(
										new IModelObject[] { datasetObjInner }, 
										new String[] { "ref_names", "object_name" }, 
										new AsyncCallback<IServiceData>() {

									@Override
									public void onSuccess(IServiceData response) {
										try {
											int refNamesSize = response.getModelObject(ds_uid).getPropertyObject("ref_names")
													.getDisplayValues().size();
											String ds_object_name = response.getModelObject(ds_uid).getPropertyObject("object_name")
													.getDisplayValue();

											if (refNamesSize > 0) {
												IModelObject[] revItems;

												if (finalRevisionItem == null) {
													revItems = new IModelObject[0];
												} else {
													revItems = new IModelObject[] { finalRevisionItem };
												}

												createDispatcherRequest(new IModelObject[] { finalOtherSideObj }, 
														revItems, drmCommandInner);
											} else {
												msgService.notify("Dataset " + ds_object_name + " has no file reference.");
											}
										} catch (Exception ex) {
											ex.printStackTrace(System.out);
											msgService.error("DRMCommandExecute.processDataset() caught exception: " + 
													ex.getMessage());
										}
									}

									@Override
									public void onFailure(Throwable caught) {
										msgService.notify("Failed to retrieve properties for selected dataset.");
										msgService.error("DRMCommandExecute.processDataset() caught exception: " + 
												caught.getMessage());
									}

								}); // end of getProperties
							} // end of apply protection
						}
					}

					@Override
					public void onFailure(Throwable caught) {
						msgService.notify("Failed to retrieve properties for selected dataset.");
						msgService.error("DRMCommandExecute.processDataset() caught exception: " + 
								caught.getMessage());
					}

				}); // end of getProperties

	}

	private void createDispatcherRequest(IModelObject[] primaryObjs, IModelObject[] secondaryObjs, String drmCommand) {
		try {
			IJSO requestArgs = JSOFactory.createObject();
			requestArgs.setModelObjects("primaryObjects", primaryObjs);
			requestArgs.setModelObjects("secondaryObjects", secondaryObjs);
			requestArgs.setInteger("priority", DSF_PRIORITY);
			requestArgs.setString("providerName", DSF_PROVIDER);
			requestArgs.setString("serviceName", DSF_SERVICE);

			IJSO keyValueArguments = JSOFactory.createObject();
			keyValueArguments.setString("key", KEY_NXLACTION);
			keyValueArguments.setString("value", drmCommand);

			IJsArray keyValueArgumentsInput = JSOFactory.createArray();
			keyValueArgumentsInput.push(keyValueArguments);
			requestArgs.setArray("keyValueArgs", keyValueArgumentsInput);

			IJSO soaInput = JSOFactory.createObject();
			IJsArray inputs = JSOFactory.createArray();
			inputs.push(requestArgs);
			soaInput.setArray("inputs", inputs);

			SOA.executeOperation(SOA_SERVICENAME, SOA_OPERATIONNAME, soaInput, new AsyncCallback<IServiceData>() {

				@Override
				public void onSuccess(IServiceData result) {
					if (result.createException() != null) {
						msgService.error("createDispatcherRequest exception: " + result.createException().getMessage());
					}

					msgService.notify("New translation request is created.");
				}

				@Override
				public void onFailure(Throwable caught) {
					msgService.notify("Failed to create translation request.");
					msgService.error("executeOperation() caught exception: " + caught.getMessage());
				}

			}); // end of createDispatcherRequest function
		} catch (Exception ex) {
			ex.printStackTrace(System.out);
			msgService.error("DRMCommandExecute.createDispatcherRequest() caught exception: " + 
					ex.getMessage());
		}
	}

	private void initPrefsForPDP(AsyncCallback<Map<String, String[]>> prefsCallback) {
		PreferenceService.getStringValues(new String[] { NXL_EVALUATION_ATTRIBUTES, NXL_PDP_DEFAULT_ACTION, 
				NXL_PDP_DEFAULT_MESSAGE, NXL_PDPHOST_SERVERAPP }, prefsCallback);
	}
	
	private void initPrefsForExp(AsyncCallback<Map<String, String[]>> prefsCallback) {
		PreferenceService.getStringValues(new String[] { NXL_RELATION_FILTER }, prefsCallback);
	}

}
