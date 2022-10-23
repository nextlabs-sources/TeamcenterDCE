package com.nextlabs.drm.rmx.templatemodifier.configuration;

import java.io.File;

/*
 * Created on November 18, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

public abstract class ModifierConstants {
	
	public static final boolean REMOVED_BACKUP_FILES = false;
	public static final boolean ARCHIVED_BACKUP_FILES = true;
	
	public static final String MOD_LINE_REGEX = ".*\\.\\w*(.nxl).*";
	public static final String ORG_TEXT_DEF_REGEX = "\\t*TEXT\\t*\"Text\"\\t*\\*\\t*-1";
	public static final String MOD_TEXT_DEF_REGEX = "\\t*BINARY\\t*\"Text\"\\t*\\*\\t*-1";
	
	// Template modifier supported actions
	public static final String ACT_CUSTOMIZE = "customize";
	public static final String ACT_RESTORE = "restore";
	public static final String ACT_UNDO = "undo";
	
	// NextLabs element attributes
	public static final String NXL_FORMAT = "BINARY";
	public static final String NXL_EXT = "nxl";
	public static final String NXL_NR = "Nxl3_DRM_Protected_Reference";
	
	// constants for TC system environment variable
	public static final String TC_DATA_ENVVAR = "TC_DATA";
	
	// constants for definition file
	public static final String DEFINITION_FILE_PATH = "gs_info";
	public static final String DEFINITION_FILE_EXT = "des";

	// constants for template and baseline
	public static final String TEMPLATE_FILE_PATH = "model";
	public static final String BASELINE_FILE_PATH = "model" + File.separator + "baselines";
	public static final String TEMPLATE_FILE_EXT = "xml";
	public static final String BASELINE_FILE_EXT = "xml";
	
	public static final String TC_DATASET = "TcDataset";
	public static final String TC_DATASET_REFERENCE = "TcDatasetReference";
	public static final String TC_DATASET_REFERENCE_INFO = "TcDatasetReferenceInfo";
	
	public static final String ATTR_TYPE_NAME = "typeName";
	public static final String ATTR_NAME = "name";
	public static final String ATTR_FORMAT = "format";
	public static final String ATTR_TEMPLATE = "template";
	
	public static final String NXL_COMMENT_START = " NXL customization ";
	public static final String NXL_COMMENT_END = " end of NXL customization ";
	
}
