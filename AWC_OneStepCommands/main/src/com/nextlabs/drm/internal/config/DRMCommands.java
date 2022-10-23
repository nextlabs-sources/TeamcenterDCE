/*
 * Created on September 20, 2017
 *
 * All sources, binaries and HTML pages (C) copyright 2017 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.internal.config;

public enum DRMCommands {
	PROTECT {
		@Override
		public String toString() {
			return "protect";
		}
	},
	
	UNPROTECT {
		@Override
		public String toString() {
			return "unprotect";
		}
	};
}
