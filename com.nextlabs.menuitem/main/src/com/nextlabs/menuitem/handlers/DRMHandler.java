package com.nextlabs.menuitem.handlers;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.common.NotDefinedException;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.handlers.HandlerUtil;
import org.eclipse.jface.dialogs.MessageDialog;

import com.nextlabs.menuitem.policy.NextLabsPolicyConstants;
import com.nextlabs.menuitem.policy.NextLabsAuthorizationAgent;
import com.nextlabs.menuitem.configuration.NextLabsConstants;
import com.nextlabs.menuitem.exception.InvalidAccessException;
import com.nextlabs.menuitem.exception.InvalidFileException;
import com.nextlabs.menuitem.exception.InvalidStateException;
import com.nextlabs.menuitem.exception.InvalidStructureException;
import com.teamcenter.rac.aif.kernel.AIFComponentContext;
import com.teamcenter.rac.aif.kernel.AbstractAIFSession;
import com.teamcenter.rac.aif.kernel.InterfaceAIFComponent;
import com.teamcenter.rac.aifrcp.AIFUtility;
import com.teamcenter.rac.ets.external.DispatcherRequestFactory;
import com.teamcenter.rac.kernel.TCComponent;
import com.teamcenter.rac.kernel.TCComponentContextList;
import com.teamcenter.rac.kernel.TCComponentDataset;
import com.teamcenter.rac.kernel.TCComponentItemRevision;
import com.teamcenter.rac.kernel.TCComponentBOMLine;
import com.teamcenter.rac.kernel.TCComponentTcFile;
import com.teamcenter.rac.kernel.TCException;
import com.teamcenter.rac.kernel.TCPreferenceService;
import com.teamcenter.rac.kernel.TCPropertyService;
import com.teamcenter.rac.kernel.TCSession;
import com.teamcenter.soa.client.model.Property;
import com.teamcenter.soa.exceptions.NotLoadedException;

import static com.nextlabs.menuitem.configuration.NextLabsConstants.*;

/**
 * Created on Aug 15, 2014
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2020 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

/**
 * Our sample handler extends AbstractHandler, an IHandler base class.
 * @see org.eclipse.core.commands.IHandler
 * @see org.eclipse.core.commands.AbstractHandler
 */
public class DRMHandler extends AbstractHandler {
	
	private TCPreferenceService tcPrefService;
	private TCPropertyService tcPropService;
	private TCSession session;
	private List<String> relationNames;
	private boolean useRelationFilter = false;
	
	private NextLabsAuthorizationAgent agent;
	private String[] evaluationAttrs;
	private String pdpDefaultAction;
	
	/**
	 * The constructor.
	 */
	public DRMHandler() {
		//get TC preference: NXL_Relation_Filter
		AbstractAIFSession localAbstractAIFSession = AIFUtility.getDefaultSession();
		if ((localAbstractAIFSession instanceof TCSession) && 
				(localAbstractAIFSession.isLoggedIn())) {
			session = (TCSession) localAbstractAIFSession;
		}
		
		tcPrefService = session.getPreferenceService();
		tcPropService = session.getPropertyService();
		
		// setting up NextLabsAuthorizationAgent by fetching connection details
		// from TC preferences
		try {
			evaluationAttrs = tcPrefService.getStringValues(NXL_EVALUATION_ATTRIBUTES);
					
			pdpDefaultAction = tcPrefService.getStringValue(NXL_PDP_DEFAULT_ACTION);
			final String pdpDefaultMessage = tcPrefService.getStringValue(NXL_PDP_DEFAULT_MESSAGE);
			
			// PDP
			final String pdpHostServerApp = tcPrefService.getStringValue(NXL_PDPHOST_SERVERAPP);
			final String pdpPortServerApp = tcPrefService.getStringValue(NXL_PDPPORT_SERVERAPP);
			final String pdpHttpsServerApp = tcPrefService.getStringValue(NXL_PDP_HTTPS_SERVERAPP);
			
			// OAuth2
			final String oauth2IgnoreHttps = tcPrefService.getStringValue(NXL_PDP_IGNOREHTTPS_SERVERAPP);
			final String oauth2Host = tcPrefService.getStringValue(NXL_OAUTH2HOST);
			final String oauth2Port = tcPrefService.getStringValue(NXL_OAUTH2PORT);
			final String oauth2Https= tcPrefService.getStringValue(NXL_OAUTH2_HTTPS);
			final String oauth2ClientID = tcPrefService.getStringValue(NXL_OAUTH2_CLIENTID);
			final String oauth2ClientSecret = tcPrefService.getStringValue(NXL_OAUTH2_CLIENTSECRET);	
						
			agent = NextLabsAuthorizationAgent.getInstance(
				pdpHostServerApp, pdpPortServerApp, pdpHttpsServerApp, oauth2IgnoreHttps,
				oauth2Host, oauth2Port, oauth2Https, oauth2ClientID, oauth2ClientSecret,
				pdpDefaultAction, pdpDefaultMessage);
		} catch (Exception ex) {
			System.out.println("DRMHandler.DRMHandler() caught exception: " + ex.getMessage());
			ex.printStackTrace(System.out);
			
			evaluationAttrs = new String[0];
			agent = null;
		}
	}
	
	/**
	 * the command has been executed, so extract extract the needed information
	 * from the application context.
	 */
	public Object execute(ExecutionEvent event) throws ExecutionException {
		
		IWorkbenchWindow window = HandlerUtil.getActiveWorkbenchWindowChecked(event);
		InterfaceAIFComponent[] selectComps = AIFUtility.getCurrentApplication()
				.getTargetComponents();
		
		
		tcPrefService = session.getPreferenceService();	
		relationNames = new ArrayList<String>();
		String[] tmpNames = tcPrefService.getStringValues(NXL_RELATION_FILTER);
		
		// if user has configured NXL_Relation_Filter
		if (tmpNames != null && tmpNames.length > 0) {
			for (String name:tmpNames) {
				name = name.trim();
				
				if (name.length() > 0) {
					relationNames.add(name);
				}
			}
			
			if (relationNames.size() > 0)
				useRelationFilter = true;
		}
		
		String msg = "";
		String warnMsg = "";
		String errMsg = "";
		
		// default action
		String drmCommand = CMD_PROTECT;
		try {
			drmCommand = event.getCommand().getName();
			
			if (drmCommand.equalsIgnoreCase(LBL_CMD_UNPROTECT)) {
				drmCommand = CMD_UNPROTECT;
			}
		} catch (NotDefinedException nde) {
			// rare case
		}
		drmCommand = drmCommand.toLowerCase();

		if (!isSelectedComponentsSupported(selectComps)) {
			MessageDialog.openInformation(
					window.getShell(),
					MSG_DIAG_TITLE,
					"Please select a dataset, an item revision, or a BOM line to secure.");
			
			return null;
		}
		
		StringBuffer dsNames = new StringBuffer("");
		StringBuffer dsNames_NoFile = new StringBuffer("");
		StringBuffer dsNames_InvalidAccess = new StringBuffer("");
		StringBuffer irNames_NoFile = new StringBuffer("");
		StringBuffer dsInvalidStateMsg = new StringBuffer("");
		
		for (InterfaceAIFComponent selectComp : selectComps) {
			// process all dataset of the selected Item Revision
			if (selectComp instanceof TCComponentItemRevision) {
				TCComponentItemRevision tcCompIR = (TCComponentItemRevision) selectComp;
				
				int iniSize1 = dsNames.length();
				int iniSize2 = dsNames_NoFile.length();
							
				try {
					// fix bug 27622: Checkout on Item Revision does not checkout the child items. Different from Document Revision.
					if (!tcCompIR.isCheckedOut()) {
						TCComponent[] tcComps;
						
						// fix bug 31676
						if (useRelationFilter) {
							TCComponentContextList tcCompContextList = tcCompIR.getRelatedList(relationNames.toArray(new String[0]));
							tcComps = filterDuplicateElement(tcCompContextList.toTCComponentArray());
						} else {
							tcComps = filterDuplicateElement(tcCompIR.getRelatedComponents());
						}
						
						for (TCComponent tcComp : tcComps) {
							// skip NextLabs Protected dataset
							// fix bug 27626: where MSWordX is TCComponentMSWordX instead of TCComponentDataset
							if (tcComp != null && 
									(tcComp instanceof TCComponentDataset)) {
								
								try {									
									dsNames.append(
										processDataset(window, (TCComponentDataset) tcComp, drmCommand) + ", ");
								} catch (InvalidFileException nfe) {
									dsNames_NoFile.append(nfe.getDsName() + ", ");
								} catch (InvalidAccessException iae) {
									dsNames_InvalidAccess.append(iae.getDsName() + ", ");
								} catch (InvalidStateException ise) {
									dsInvalidStateMsg.append(ise.getMessage() + "\n");
								} catch (InvalidStructureException istre) {
									// dataset will have item revision in this case.
								}
							}
						}
					} else {
						dsInvalidStateMsg.append("Invalid state: " + tcCompIR.getProperty("object_name") + " has been checked out.\n");
					}
					
					// delete last 2 chars that are ", "
					/*if (dsNames.length() > 2) {
						dsNames.setLength(dsNames.length() - 2);
						msg = "New translation requests for " + dsNames.toString() + " are created.";
					}
					
					if (dsNames_NoFile.length() > 2) {
						dsNames_NoFile.setLength(dsNames_NoFile.length() - 2);
						msg += "\n\n" + dsNames_NoFile.toString() + " have no file reference or empty file.";
					}*/
					
					// fix bug 27356
					if (iniSize1 == dsNames.length() && iniSize2 == dsNames_NoFile.length()) {
						irNames_NoFile.append(tcCompIR.getStringProperty("object_string") + ", ");
					}
					
					// fix bug 27622
					/*if (dsInvalidStateMsg.length() > 0) {
						msg += "\n\n" + dsInvalidStateMsg.toString();
					}*/
				} catch (TCException tce) {
					errMsg = "Exception: " + tce.getMessage();
				}
			} else if (selectComp instanceof TCComponentDataset) {
				TCComponentDataset tcCompDS = (TCComponentDataset) selectComp;
				
				try {
					dsNames.append(processDataset(window, tcCompDS, drmCommand) + ", ");
				} catch (InvalidFileException nfe) {
					dsNames_NoFile.append(nfe.getDsName() + ", ");
				} catch (InvalidAccessException nae) {
					dsNames_InvalidAccess.append(nae.getDsName() + ", ");
				} catch (InvalidStateException ise) {
					dsInvalidStateMsg.append(ise.getMessage() + "\n");
				} catch (InvalidStructureException istre) {
					msg = istre.getMessage();
				}
			} else if (selectComp instanceof TCComponentBOMLine) {
				TCComponentBOMLine tcCompBL = (TCComponentBOMLine) selectComp;
				
				try {
					msg = "Applying protection to the assemblies and parts.";
					
					expandBOM(tcCompBL, window, drmCommand);
				} catch (TCException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (InvalidFileException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (InvalidStateException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (InvalidStructureException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		} // end of for
		
		// delete last 2 chars that are ", "
		if (dsNames.length() > 2) {
			dsNames.setLength(dsNames.length() - 2);
			msg = "New translation requests for " + dsNames.toString() + " are created.";
		}
		
		if (dsNames_NoFile.length() > 2) {
			dsNames_NoFile.setLength(dsNames_NoFile.length() - 2);
			warnMsg += "Dataset " + dsNames_NoFile.toString() + " have no file reference or empty file.\n";
		}
		
		if (dsNames_InvalidAccess.length() > 2) {
			dsNames_InvalidAccess.setLength(dsNames_InvalidAccess.length() - 2);
			warnMsg += "Remove protection is denied for Dataset " + dsNames_InvalidAccess.toString() + ".\n";
		}
		
		if (irNames_NoFile.length() > 2) {
			irNames_NoFile.setLength(irNames_NoFile.length() - 2);
			warnMsg += "Item Revision " + irNames_NoFile.toString() + " has no dataset to be processed.\n";
		}
		
		if (dsInvalidStateMsg.length() > 0) {
			warnMsg += dsInvalidStateMsg.toString();
		}
		
		if (msg.length() > 0) {
			MessageDialog.openInformation(
					window.getShell(),
					MSG_DIAG_TITLE,
					msg);
		} else {
			MessageDialog.openInformation(
					window.getShell(),
					MSG_DIAG_TITLE,
					"No data to be processed.");
		}
		
		if (warnMsg.length() > 0) {
			MessageDialog.openWarning(
					window.getShell(), 
					MSG_DIAG_TITLE, 
					warnMsg);
		}
		
		if (errMsg.length() > 0) {
			MessageDialog.openError(
					window.getShell(),
					MSG_DIAG_TITLE,
					errMsg);
		}
		
		return null;
	}
	
	private TCComponent[] filterDuplicateElement(TCComponent[] input) {
		Set<TCComponent> setTC = new HashSet<TCComponent>(Arrays.asList(input));
		
		return setTC.toArray(new TCComponent[0]);
	}
	
	private boolean isSelectedComponentsSupported(InterfaceAIFComponent[] selectComps) {
		for (InterfaceAIFComponent selectComp : selectComps) {
			if (!(selectComp instanceof TCComponentDataset) && 
					!(selectComp instanceof TCComponentItemRevision) && 
					!(selectComp instanceof TCComponentBOMLine)) {
				return false;
			}
		}
		
		return true;
	}
	
	private void expandBOM(TCComponentBOMLine tcCompBL, IWorkbenchWindow window, String drmCommand) 
			throws TCException, InvalidFileException, InvalidStateException, InvalidStructureException {
		if (tcCompBL.hasChildren()) {
			processBOM(tcCompBL, window, drmCommand);
			
			// process direct children
			AIFComponentContext[] tcComps = tcCompBL.getChildren();
			
			for (AIFComponentContext tcC : tcComps) {
				TCComponentBOMLine tcCBL = (TCComponentBOMLine) tcC.getComponent();
			
				expandBOM(tcCBL, window, drmCommand);
			}
		} else {
			processBOM(tcCompBL, window, drmCommand);			
		}
	}
	
	private void processBOM(TCComponentBOMLine tcCompBL, IWorkbenchWindow window, String drmCommand) 
			throws TCException, InvalidFileException, InvalidStateException, InvalidStructureException {
		TCComponentItemRevision tcCompIR = (TCComponentItemRevision) tcCompBL.getItemRevision();
		
		if (!tcCompIR.isCheckedOut()) {
			TCComponent[] tcComps;
			
			if (useRelationFilter) {
				TCComponentContextList tcCompContextList = tcCompIR.getRelatedList(relationNames.toArray(new String[0]));
				tcComps = filterDuplicateElement(tcCompContextList.toTCComponentArray());
			} else {
				tcComps = tcCompIR.getRelatedComponents();
			}
			
			for (TCComponent tcComp : tcComps) {
				if (tcComp != null && 
						(tcComp instanceof TCComponentDataset)) {
					TCComponentDataset tcDS = (TCComponentDataset) tcComp;
					
					// fix bug 34766
					try {
						processDataset(window, tcDS, drmCommand);
					} catch (InvalidFileException ife) {
						ife.printStackTrace();
					} catch (InvalidAccessException iae) {
						iae.printStackTrace();
					} catch (InvalidStateException ise) {
						ise.printStackTrace();
					} catch (InvalidStructureException istruce) {
						istruce.printStackTrace();
					}
				}
			}
		} else {
			System.out.println("Invalid state: " + tcCompIR.getProperty("object_name") + " has been checked out.\n");
		}
	}
	
	private String processDataset(IWorkbenchWindow window, TCComponentDataset tcCompDS, String drmCommand) 
			throws InvalidFileException, InvalidAccessException, InvalidStateException, InvalidStructureException {
		String dsObjectName = "";
		
		TCComponent[] tcComps_Pri = new TCComponent[1];
		TCComponent[] tcComps_Sec = new TCComponent[1];
		
		// Arguments for dispatcher request
		HashMap<String, String> dispReqArgs = new HashMap<String, String>();

		tcComps_Pri[0] = tcCompDS;

		try {
			dsObjectName = tcCompDS.getProperty("object_name");
						
			TCComponent[] tcComps = tcCompDS.getNamedReferences();
									
			// dataset has no file
			if (tcComps.length <= 0) {
				throw new InvalidFileException(dsObjectName);
			}
			
			// dataset has been checkout
			// fix bug 27622
			if (tcCompDS.isCheckedOut()) {
				throw new InvalidStateException("Invalid state: " + dsObjectName + " has been checked out.");
			}
			
			// check is the file is 0kb which will not be processed by RMS
			// fix bug 27562 (removed, to improve performance on large amount of dataset or dataset with big file)
				// this does not violate the handling of 0kb file, Taskprep will handle correctly
			// fix bug 34860, force refresh to prevent "the given tag does not exist in database..."
			tcCompDS.refresh();		
			
			TCComponentDataset[] tcCompDSs = tcCompDS.getAllVersions();
			AIFComponentContext[] aifCompContexts = null;
			
			for (TCComponentDataset tcCompDS_Temp : tcCompDSs) {
				aifCompContexts = tcCompDS_Temp.getPrimary();
				
				if (aifCompContexts != null && aifCompContexts.length > 0) {
					break;
				}
			}
			
			if (aifCompContexts == null)
				return "";
			
			for (int i = aifCompContexts.length - 1; i >= 0; i--) {
				if (aifCompContexts[i].getComponent() instanceof TCComponentItemRevision) {
					tcComps_Sec[0] = (TCComponent) aifCompContexts[i].getComponent();
					break;
				}
			}
			
			if (tcComps_Sec[0] == null) {
				/// test
				tcComps_Sec = new TCComponent[0];
				
				// org
				///throw new InvalidStructureException("Invalid structure: " + dsObjectName + " has not been attached to Item Revision.");
			}
			
			// evaluate policy on remove protection
			if (drmCommand.equalsIgnoreCase(CMD_UNPROTECT)) {
				if (agent == null) {
					System.out.println("AGENT IS NULL");
					
					if (!NextLabsPolicyConstants.RESPONSE_ALLOW_VALUE.equalsIgnoreCase(pdpDefaultAction))
						throw new InvalidAccessException(dsObjectName);
				}
				
				String[] attrValues = tcPropService.getProperties(tcCompDS, evaluationAttrs);
				String user = session.getUser().getUserId();
				
				HashMap<String, Object> attributes = new HashMap<String, Object>();
				
				int indexAttrValues = 0;
				for (String attr: evaluationAttrs) {
					String value = attrValues[indexAttrValues++];
					System.out.println(" >> " + attr + " : " + value);
					
					// need to handle multiple values
					if (value != null && !value.equalsIgnoreCase("")) {
						if (value.contains(",")) {
							String[] values = value.split(",");
							List<String> lstValue = new ArrayList<String>();
							for (String val : values)
								lstValue.add(val.trim());
							
							attributes.put(attr, lstValue);
						} else
							attributes.put(attr, value);
					}
				}
			
				// fix bug 46986
				if (tcComps_Sec.length >= 1 && tcComps_Sec[0] != null) {
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_REV_TYPE, tcComps_Sec[0].getType());
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_REV_NAME, tcPropService.getProperty(tcComps_Sec[0], "object_name"));
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_REV_UID, tcComps_Sec[0].getUid());
				}
				
				if (tcCompDS != null) {
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_DS_TYPE, tcCompDS.getType());
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_DS_NAME, tcPropService.getProperty(tcCompDS, "object_name"));
					attributes.put(NextLabsPolicyConstants.ATTRIBUTE_DS_UID, tcCompDS.getUid());
				}
				
				try {
					HashMap<String, Object> hasAccessResult = agent.hasAccess(user, 
							NextLabsPolicyConstants.RIGHT_EXECUTE_REMOVEPROTECTION, attributes, 
							new HashMap<String, Object>(), new HashMap<String, Object>());
					
					String response = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_KEY);
					String sError = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_ERROR_KEY);
										
					// Checking to see if there is SDK error response
					if (null != sError) {
						String sDefaultMsg = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_MESSAGE_KEY);
						
						if (null != sDefaultMsg) {
							System.out.println("CE SDK error: " + sDefaultMsg);
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
							
							if (obligationName != null && 
									obligationName.equalsIgnoreCase(NextLabsPolicyConstants.OBLIGATION_REMOVEPROTECTION)) {
								obligationRemoveProtection = true;
								break;
							}
						}
					}
					
					if (!NextLabsPolicyConstants.RESPONSE_ALLOW_VALUE.equalsIgnoreCase(response) || 
							!obligationRemoveProtection)
						throw new InvalidAccessException(dsObjectName);
				} catch (Exception ex) {
					System.out.println("DRMHandler.processDataset() caught exception: " + ex.getMessage());
					ex.printStackTrace(System.out);
					
					if (!NextLabsPolicyConstants.RESPONSE_ALLOW_VALUE.equalsIgnoreCase(pdpDefaultAction))
						throw new InvalidAccessException(dsObjectName);
				}
			}
			
						
			try {
				// introduce unprotect command
				dispReqArgs.put("nxlaction", drmCommand);
				
				// create dispatcher service request
				DispatcherRequestFactory.createDispatcherRequest(
						DSF_PROVIDER, DSF_SERVICE, DSF_PRIORITY, tcComps_Pri, tcComps_Sec, 
						null, 0, null, DSF_SERVICE, dispReqArgs);
								
				return dsObjectName;
			} catch (Exception ex) {
				MessageDialog.openError(
						window.getShell(),
						MSG_DIAG_TITLE,
						"Caught exception while processing " + dsObjectName + ": " + ex.getMessage());
			}
	
		} catch (TCException tce) {
			MessageDialog.openError(
					window.getShell(),
					MSG_DIAG_TITLE,
					"Caught exception while processing: " + tce.getMessage());
		}
		
		return "";
	}
	
}
