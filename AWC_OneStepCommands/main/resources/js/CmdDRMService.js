/**
* Define DRM Command execution include: Apply Protection and Remove Protection for an Item Revision or a Dataset.
* On AWC 3.4, 4.0, 4.1: We use logger.js as logging library (prefer by Siemens)
* On AWC 3.3: Need to use logService.js (instead of logger.js).This file will be change during install AWC feature
* 
* @module js/CmdDRMService
*/

define(["app", "js/logger", "soa/kernel/soaService", "js/messagingService"], function (app, _logSvc) {
	'use strict';
	// Define object type constant
	var TYPE_ITEM_REVISION = "ItemRevision";
	var TYPE_DATASET = "Dataset";

	// key value argument for translation request
	var KEY_NXLACTION = "nxlaction";

	// Dispatcher Service Framework: Name of the service provider
	var DSF_PROVIDER = "NEXTLABS";

	// Dispatcher Service Framework: Name of the service
	var DSF_SERVICE = "tonxlfile";

	// Dispatcher Service Framework: Priority used in processing the request
	var DSF_PRIORITY = 2;

	// TC SOA REST API: Name of the SOA service
	var SOA_GET_PREF_SERVICE = "Administration-2012-09-PreferenceManagement";
	var SOA_GET_PROPERTY_SERVICE = "Core-2006-03-DataManagement";
	var SOA_EXPAND_RELATION_SERVICE = "Core-2007-09-DataManagement";
	var SOA_DISPATCHER_SERVICE = "Core-2008-06-DispatcherManagement";

	// TC SOA REST API: Name of the SOA service's operation
	var SOA_GET_PREF_OPERATION = "getPreferences";
	var SOA_GET_PROPERTY_OPERATION = "getProperties";
	var SOA_EXPAND_RELATION_OPERATION = "expandGRMRelationsForPrimary";
	var SOA_DISPATCHER_OPERATION = "createDispatcherRequest";

	// 3.5 feature: For Policy Evaluation
	// var NXL_EVALUATION_ATTRIBUTES = "NXL_Evaluation_Attributes";
	// var NXL_PDPHOST_SERVERAPP = "NXL_PDPHOST_SERVERAPP"; // format: hostname/ip:port
	// var NXL_PDP_DEFAULT_ACTION = "NXL_PDP_DEFAULT_ACTION";
	// var NXL_PDP_DEFAULT_MESSAGE = "NXL_PDP_DEFAULT_MESSAGE";
	// var DEFAULT_PDP_PORT = 1099;

	var exports = {};
	var _soaSvc;
	var _messagingSvc;

	exports.execute = function (user, drmCommand, selectedObjs) {
		var index;
		var userId = user.uid;

		_logSvc.info("Requested by User ID: " + userId);
		_logSvc.info("Executing DRMCommand: " + drmCommand);

		// Start DRM command
		var supportedObjs = [];
		for (index = 0; index < selectedObjs.length; index++) {
			try {
				// Bug 52446: Need to check type of SelectedObject of Document/Drawing Revision by modelType.parentTypeName as ItemRevision (its parent type is Item)
				if (selectedObjs[index].modelType.parentTypeName == TYPE_DATASET || selectedObjs[index].type == TYPE_ITEM_REVISION || selectedObjs[index].modelType.parentTypeName == TYPE_ITEM_REVISION) {
					supportedObjs.push(selectedObjs[index]);
				}
			} catch (err) {
				_logSvc.error(err.message);
			}
		}

		// Bug 52710: Only do command on specific relation with the ItemRevision. Get NXL_Relation_Filter preference
		var relationType;
		var typeFilter;
		var relationFilterList;
		_soaSvc.post(SOA_GET_PREF_SERVICE, SOA_GET_PREF_OPERATION, {
			preferenceNames: ["NXL_Relation_Filter"],
			includePreferenceDescriptions: !1
		}, {}).then(function (res) {
			// onSuccess of getPreferences for NXL_Relation_Filter
			if (res.response[0].values.values == undefined) {
				relationType = [];
			} else {
				relationType = res.response[0].values.values;
			}
			if (relationType.length == 0) { // NXL_Relation_Filter is not set
				_logSvc.info("NXL_Relation_Filter is not set. OneStepCommand apply for all relations");
				// expand related dataset
				typeFilter = {
					"relationTypeName": null,
					"otherSideObjectTypes": null
				};
				relationFilterList = [typeFilter];
			} else { // Get all the relation from NXL_Relation_Filter preference
				_logSvc.info("DRM command will apply with following relations: ");
				_logSvc.info(relationType);
				relationFilterList = [];
				for (index = 0; index < relationType.length; index++) {
					var relationName = relationType[index];
					typeFilter = {
						"relationTypeName": relationName,
						"otherSideObjectTypes": null
					};
					relationFilterList.push(typeFilter);
				}
			}

			var expandPref = {
				"expItemRev": false,
				"returnRelations": true,
				"info": relationFilterList
			};

			for (index = 0; index < supportedObjs.length; index++) {
				var primaryObj = supportedObjs[index];
				console.debug("Primary Object : " + primaryObj);
				console.debug(primaryObj);
				// Bug 52446: Addition check for Document Revision and Drawing Revision
				// Check type of this model object. If it is Item/Document/Drawing-Revision, we need to ExpandGRMRelations...
				if (primaryObj.type == TYPE_ITEM_REVISION || primaryObj.modelType.parentTypeName == TYPE_ITEM_REVISION) {
					processRevisionLevel(primaryObj, expandPref, drmCommand);
				}
				else if (primaryObj.modelType.parentTypeName == TYPE_DATASET) {
					processDatasetLevel(null, primaryObj, drmCommand);
				}
			}
		}, function (err) {
			// onFail to get NXL_Relation_Filter
			_messagingSvc.showError("Failed to get preference NXL_Relation_Filter");
			_logSvc.error("Failed to get preference NXL_Relation_Filter : " + err.message);
		}); // Wrapper of getting preference NXL_Relation_Filter
	};

	function processRevisionLevel(primaryObj, expandPref, drmCommand) {
		// Ask SoA Service about the checked_out properties of this ItemRevision ModelObj
		_soaSvc.post(SOA_GET_PROPERTY_SERVICE, SOA_GET_PROPERTY_OPERATION, {
			"objects": [primaryObj],
			"attributes": ["checked_out"]
		}, {}).then(function (res) {
			// onSucess of getProperties on checked_out

			// Checked_out == "Y"
			if (res.modelObjects[primaryObj.uid].props.checked_out.dbValues[0] == "Y") {
				var objectType = "Item Revision";
				if (res.modelObjects[primaryObj.uid].type == "DocumentRevision") {
					objectType = "Document Revision";
				} else if (res.modelObjects[primaryObj.uid].type == "Drawing Revision") {
					objectType = "Drawing Revision";
				}
				_messagingSvc.showError("Invalid state: Selected " + objectType + " has been checked out.");
			} else { // Checked_out == ""
				_soaSvc.post(SOA_EXPAND_RELATION_SERVICE, SOA_EXPAND_RELATION_OPERATION, {
					"primaryObjects": [primaryObj],
					"pref": expandPref
				}, {}).then(function (res) {
					// onSuccess of expandGRMRelationsForPrimary
					console.debug("Expand GRM Relationship : ");
					console.debug(res);

					// Bug 54840: Count number of dataset will process. 
					var datasetCount = 0;
					for (var outIndex = 0; outIndex < res.output.length; outIndex++) {
						for (var dataIndex = 0; dataIndex < res.output[outIndex].relationshipData.length; dataIndex++) {
							if (res.output[outIndex].relationshipData[dataIndex].relationshipObjects == undefined) {
								continue
							}
							for (var relIndex = 0; relIndex < res.output[outIndex].relationshipData[dataIndex].relationshipObjects.length; relIndex++) {
								var revisionItem = res.output[outIndex].inputObject;
								var otherSideObj = res.output[outIndex].relationshipData[dataIndex].relationshipObjects[relIndex].otherSideObject;

								// Bug 52550: Model Object on AWC 3.3 is different compare to AWC 3.4. Need to check if the property exist (modelType.parentTypeName or className) to use to check DATASET type.
								if ((otherSideObj.modelType != undefined && otherSideObj.modelType.parentTypeName == TYPE_DATASET) || (otherSideObj.className != undefined && otherSideObj.className == TYPE_DATASET)) {
									processDatasetLevel(revisionItem, otherSideObj, drmCommand);
									datasetCount = datasetCount + 1;
								}
							}
						}
					}
					// Bug 54840: If 0 after all, then pop message that "No dataset to be processed" (Even after NXL_Relation_Filter)
					if (datasetCount == 0) {
						_messagingSvc.showInfo("No dataset to be processed.");
					}
				}, function (err) {
					// onFailure of expandGRMRelationsForPrimary
					_messagingSvc.showError("Failed to expand the related objects for selected item.");
					_logSvc.error("Failed to expand the related objects for selected item : " + err.message);
				});
			}
		}, function (err) {
			_messagingSvc.showError("Failed to retrieve \"checked_out\" property of selected object.");
			_logSvc.error("Failed to retrieve \"checked_out\" property of selected object : " + err.message);
		});
	}

	function processDatasetLevel(revisionItem, datasetObj, drmCommand) {
		var datasetUID = datasetObj.uid;
		// Ask SoA Service about the checked_out properties of this Dataset ModelObj
		_soaSvc.post(SOA_GET_PROPERTY_SERVICE, SOA_GET_PROPERTY_OPERATION, {
			"objects": [datasetObj],
			"attributes": ["checked_out"]
		}, {}).then(function (res) {
			// onSuccess of getProperties for "checked_out"
			if (res.modelObjects[datasetObj.uid].props.checked_out.dbValues[0] == "Y") { // Checked_out == "Y"
				_messagingSvc.showError("Invalid state: Selected Dataset has been checked out.");
			} else {
				// Checked_out == ""
				// Without Policy Evaluation (Not apply for 3.3, 3.4, 4.0, 4.1)
				// Both Apply Protection and Remove Protection have same procedures
				_soaSvc.post(SOA_GET_PROPERTY_SERVICE, SOA_GET_PROPERTY_OPERATION, {
					"objects": [datasetObj],
					"attributes": ["ref_names", "object_name"]
				}, {}).then(function (res) {
					// onSucess of getProperties for "ref_name" and "object_name"
					var datasetObjectName = res.modelObjects[datasetUID].props.object_name.dbValues[0];
					// Bug 54840: If being empty dataset, pop message "No Translation will be create."
					if (res.modelObjects[datasetUID].props.ref_names.dbValues[0] == undefined) {
						// In this case, empty dataset
						_messagingSvc.showInfo(datasetObjectName + " is empty dataset, no translation will be create.");
					} else {
						var refNameSize = res.modelObjects[datasetUID].props.ref_names.dbValues[0].length;

						if (refNameSize > 0) {
							var revItems;
							if (revisionItem == null) {
								revItems = [];
							} else {
								revItems = [revisionItem];
							}

							createDispatcherRequest([datasetObj], revItems, drmCommand);
						} else {
							_logSvc.info("Dataset " + datasetObjectName + " has no file reference.");
						}
					}
				}, function (err) {
					// onFailure of getProperties for "ref_name" and "object_name"
					_messagingSvc.showError("Failed to retrieve \"ref_names\" and \"object_name\" properties for selected dataset.");
					_logSvc.error("Failed to retrieve \"ref_names\" and \"object_name\" properties for selected dataset : " + err.message);
				});
			}
		}, function (err) {
			// onFailure of getProperties for "checked_out"
			_messagingSvc.showError("Failed to retrieve \"checked_out\" properties for selected dataset.");
			_logSvc.error("Failed to retrieve \"checked_out\" properties for selected dataset : " + err.message);
		});
	}

	function createDispatcherRequest(primaryObjs, secondaryObjs, drmCommand) {
		// KeyValueArgs
		var keyValueArguments = {
			"key": KEY_NXLACTION,
			"value": drmCommand
		};
		// dataFiles
		var dataFiles = {}; 						// This field can be null

		// "CreateDispatcherRequestArgs"
		var requestArgs = {
			"providerName": DSF_PROVIDER,
			"serviceName": DSF_SERVICE,
			//"type": "String",           			// This field can be null
			"primaryObjects": primaryObjs,
			"secondaryObjects": secondaryObjs,
			"priority": DSF_PRIORITY,
			//"startTime": "String",      			// This field can be null
			//"endTime": "String",        			// This field can be null
			//"interval": "int",          			// This field can be null
			"keyValueArgs": [keyValueArguments],	// "KeyValueArgs[]""
			"dataFiles": [dataFiles] 				// "DataFiles[]"
		};

		var soaInput = {
			"inputs": [requestArgs]
		};

		console.debug("Dispatcher Request Input : ");
		console.debug(soaInput);

		var objectName = "dataset"; // Represent object name of selected Dataset/ItemRev/DocRev/DrawRev. Provide more descriptive message to user
		if (primaryObjs[0].cellHeader1 != undefined && primaryObjs[0].cellHeader2 != undefined) { // DispatcherRequest for Dataset level
			objectName = primaryObjs[0].cellHeader1 + " - " + primaryObjs[0].cellHeader2;
		} else if (primaryObjs[0].props.object_name.uiValues[0] != undefined && primaryObjs[0].type != undefined) { // DispatcherRequest for ItemRevision level
			objectName = primaryObjs[0].props.object_name.uiValues[0] + " - " + primaryObjs[0].type;
		}

		_soaSvc.post(SOA_DISPATCHER_SERVICE, SOA_DISPATCHER_OPERATION, soaInput, {}).then(function (res) {
			// onSuccess of Create Dispatcher Request
			_messagingSvc.showInfo("New translation request for " + objectName + " is created.");
		}, function (err) {
			// onFailure of Create Dispatcher Request
			_messagingSvc.showInfo("Failed to create translation request for " + objectName + ".");
			_logSvc.error("Failed to create dispatcher request for dataset : " + err.message);
		});
	}

	return exports, app.factory("CmdDRMService", ["soa_kernel_soaService", "messagingService", function (soaSvc, messagingSvc) {
		return _soaSvc = soaSvc, _messagingSvc = messagingSvc, exports;
	}]), {
			moduleServiceNameToInject: "CmdDRMService"
		};
});
