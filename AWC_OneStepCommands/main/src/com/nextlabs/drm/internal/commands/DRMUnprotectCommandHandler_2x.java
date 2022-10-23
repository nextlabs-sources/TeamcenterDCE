/*
 * Created on March 22, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2016 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.internal.commands;

import java.util.List;
import java.util.Map;

import com.google.inject.Inject;
import com.google.inject.name.Named;
import com.nextlabs.drm.internal.NameTokens;
import com.nextlabs.drm.internal.config.DRMCommands;
import com.nextlabs.drm.internal.config.NextLabsConstants;
import com.siemens.splm.clientfx.base.published.IMessageService;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IModelObject;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IServiceData;
import com.siemens.splm.clientfx.ui.commands.published.ICommandDisplay;
import com.siemens.splm.clientfx.ui.commands.published.AbstractCommandHandler;
import com.teamcenter.services.gwt.core.internal.SessionServiceImpl;
import com.teamcenter.services.gwt.core.internal.DataManagementServiceImpl;
import com.teamcenter.services.gwt.core.published._2007_01.session.GetTCSessionInfoResponse;
import com.google.gwt.user.client.rpc.AsyncCallback;

/**
 * One-step command for Siemens Teamcenter AWC
 * @author clow
 * 
 */
public class DRMUnprotectCommandHandler extends AbstractCommandHandler implements NextLabsConstants {
	
	// Message Service
	@Inject
	private IMessageService m_messageService;
	
	/**
	 * Constructor
	 * 
	 * @param commandDisplay: command display to use for this handler
	 * 
	 */
	@Inject
	public DRMUnprotectCommandHandler(
			@Named(NameTokens.CMD_DRMUnprotectCommand) ICommandDisplay commandDisplay) {
		super(NameTokens.CMD_DRMUnprotectCommand, commandDisplay);
	}

	@Override
	public void commandContextChanged() {
		// Object selectedObj = this.getCommandContext().getSelection().getSelectedObject();
		boolean allSupportedType = true;
		List<IModelObject> selectedObjs = this.getSelectedModelObjects();
		
		if (selectedObjs.size() <= 0) {
			allSupportedType = false;
		} else {
			for (IModelObject selectedObj : selectedObjs) {
				if (!selectedObj.getTypeObject().isInstanceOf(TYPE_ITEM_REVISION) && 
						!selectedObj.getTypeObject().isInstanceOf(TYPE_DATASET)) {
					allSupportedType = false;
					break;
				}
			}
		}
		
		if (allSupportedType) {
			enableCommand(true);
		} else {
			enableCommand(false);
		}
	}
		
	@Override
	protected void doExecute() {		
		// default action
		final String drmCommand = DRMCommands.UNPROTECT.toString();
		final List<IModelObject> selectedObjs = this.getSelectedModelObjects();
		
		SessionServiceImpl serviceImpl = new SessionServiceImpl();
		
		serviceImpl.getTCSessionInfo(
				new AsyncCallback<GetTCSessionInfoResponse>() {
					
					@Override
					public void onFailure(Throwable caught) {
						m_messageService.notify("Failed to retrive TC session information.");
					}
					
					@Override
					public void onSuccess(GetTCSessionInfoResponse result) {
						IModelObject userIModelObject = result.user;
						final String uid = userIModelObject.getUid();
						
						DataManagementServiceImpl dataMngtService = new DataManagementServiceImpl();
						
						dataMngtService.getProperties(
								new IModelObject[] { userIModelObject }, 
								new String[] { "userid" }, 
								new AsyncCallback<IServiceData>() {
									
									@Override
									public void onSuccess(IServiceData response) {
										Map<String, IModelObject> maps = response.getModelObjects();
										String userid = maps.get(uid).getPropertyObject("userid").getDisplayValue();
										
										DRMCommandExecute executor = new DRMCommandExecute(m_messageService);
										executor.execute(userid, drmCommand, selectedObjs);
									}
									
									@Override
									public void onFailure(Throwable caught) {
										m_messageService.notify("Failed to retrieve user ID.");
									}
									
								}); // end of getProperties
						
					}
				});
		
	}
	
	private void enableCommand(boolean enabled) {
		setIsVisible(enabled);
		setIsEnabled(enabled);
	}
	
}
