/*
 * Created on March 22, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2016 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.internal.commands;

import java.util.List;

import com.google.inject.Inject;
import com.google.inject.name.Named;
import com.nextlabs.drm.internal.NameTokens;
import com.nextlabs.drm.internal.config.DRMCommands;
import com.nextlabs.drm.internal.config.NextLabsConstants;
import com.siemens.splm.clientfx.base.published.IMessageService;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IModelObject;
import com.siemens.splm.clientfx.ui.commands.published.ICommandDisplay;
import com.siemens.splm.clientfx.ui.commands.published.AbstractCommandHandler;

/**
 * One-step command for Siemens Teamcenter AWC
 * @author clow
 * 
 */
public class DRMCommandHandler extends AbstractCommandHandler implements NextLabsConstants {

	// Message Service
	@Inject
	private IMessageService m_messageService;
	
	/**
	 * Constructor
	 * 
	 * @param commandDisplay command display to use for this handler
	 */
	@Inject
	public DRMCommandHandler(
			@Named(NameTokens.CMD_DRMCommand) ICommandDisplay commandDisplay) {
		super(NameTokens.CMD_DRMCommand, commandDisplay);
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
		String drmCommand = DRMCommands.PROTECT.toString();
		DRMCommandExecute executor = new DRMCommandExecute(m_messageService);
		
		List<IModelObject> selectedObjs = this.getSelectedModelObjects();
		executor.execute(null, drmCommand, selectedObjs);
	}
	
	private void enableCommand(boolean enabled) {
		setIsVisible(enabled);
		setIsEnabled(enabled);
	}
	
}
