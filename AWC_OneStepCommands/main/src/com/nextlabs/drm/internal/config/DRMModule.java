/*
 * Created on March 22, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2016 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.internal.config;

import com.nextlabs.drm.internal.NameTokens;
import com.nextlabs.drm.internal.commands.DRMCommandDisplay;
import com.nextlabs.drm.internal.commands.DRMCommandHandler;
import com.nextlabs.drm.internal.commands.DRMUnprotectCommandDisplay;
import com.nextlabs.drm.internal.commands.DRMUnprotectCommandHandler;
import com.siemens.splm.clientfx.ui.commands.published.CommandsExtensionPointHelper;
import com.siemens.splm.clientfx.ui.commands.published.CommandId;
import com.gwtplatform.mvp.client.gin.AbstractPresenterModule;
import com.google.inject.Provider;

/**
 * Module DRMModule.
 */
public class DRMModule extends AbstractPresenterModule {

	@Override
	protected void configure() {

		CommandsExtensionPointHelper.registerCommandHandler(binder(),
				NameTokens.CMD_DRMCommand, NameTokens.CMD_DRMCommand,
				DRMCommandHandler.class, DRMCommandDisplay.class);
		// Add the command to the global one step commands
		CommandsExtensionPointHelper
				.contributeCommandToArea(
						binder(),
						com.siemens.splm.clientfx.ui.published.NameTokens.GLOBAL_COMMANDS,
						com.siemens.splm.clientfx.ui.published.NameTokens.ONE_STEP_COMMANDS,
						DRMCommandCommandIdProvider.class, 10000);

		CommandsExtensionPointHelper.registerCommandHandler(binder(),
				NameTokens.CMD_DRMUnprotectCommand, NameTokens.CMD_DRMUnprotectCommand,
				DRMUnprotectCommandHandler.class,
				DRMUnprotectCommandDisplay.class);

		CommandsExtensionPointHelper
				.contributeCommandToArea(
						binder(),
						com.siemens.splm.clientfx.ui.published.NameTokens.GLOBAL_COMMANDS,
						com.siemens.splm.clientfx.ui.published.NameTokens.ONE_STEP_COMMANDS,
						DRMUnprotectCommandCommandIdProvider.class, 10000);
	}

	// Command ID Provider for DRMCommand command
	public static class DRMCommandCommandIdProvider implements
			Provider<CommandId> {

		@Override
		public CommandId get() {
			return new CommandId(NameTokens.CMD_DRMCommand);
		}

	}

	// Command ID Provider for DRMUnprotectCommand command
	public static class DRMUnprotectCommandCommandIdProvider implements
			Provider<CommandId> {

		@Override
		public CommandId get() {
			return new CommandId(NameTokens.CMD_DRMUnprotectCommand);
		}

	}

}
