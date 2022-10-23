package com.nextlabs.drm.rmx.batchprocess.configuration;

import java.util.Arrays;
import java.util.HashSet;

public class BatchProcessConstants {
	
	public static final String NXL_FILE_EXT = "nxl";
	
	public static final String DOT_NXL_FILE_EXT = ".nxl";
	
	public static final String NXL_TRANSFERRED_ATTRIBUTES = "NXL_Transferred_Attributes";
	public static final String NXL_NR_BLACKLIST = "NXL_NR_Blacklist";
	
	public static final String LAST_MOD_USER = "last_mod_user";
	public static final String LAST_MOD_DATE = "last_mod_date";
	public static final String NXL_LAST_MOD_USER = "nxl3_last_mod_user";
	public static final String NXL_LAST_MOD_DATE = "nxl3_last_mod_date";
	public static final String NXL_IS_NXL_PROTECTED = "nxl3_is_nxl_protected";
	
	// File name for connection properties
	public static final String PROFILE_PROPERTIES_FILE = "profile.properties";
			
	// Properties
	public static final String[] PROPS_MODELOBJS = new String[] {
		"object_name", "object_type", "checked_out", "active_seq"};
	
	// Property object name
	public static final String PROP_OBJECT_NAME = "object_name";
	
	// Property object type
	public static final String PROP_OBJECT_TYPE = "object_type";
	
	// Property checked out
	public static final String PROP_CHECKED_OUT = "checked_out";
	
	// Dataset Properties
	public static final String[] PROPS_DATASET = new String[] {
		"ref_list", "ref_names", "dataset_type"};
	
	// ImanFile Properties
	public static final String[] PROPS_IMANFILE = new String[] {
		"byte_size", "file_ext", "volume_tag", "original_file_name"};
	
	public static final String[] PROPS_IMANVOLUME = new String[] {
		"volume_name"};
	
	// Checkout Status Unknown
	public static final String DEFAULT_CHECKOUT_STATUS = " ";
	
	// Named reference to be handled separately for various applications integration
	// for example CAD drawing tools: NX, Solid Edge, Creo Parametric, etc
	/*public final static String[] INTEGRATION_NAMED_REFERENCES = new String[] {
		"UGMASTER", "UGPART", "UGALTREP", "UGSCENARIO", "NXSIMULATION", "ALSG", 
		"SE-part", "SE-assembly", "catpart", "catproduct", "AsmFile", "DrwFile", 
		"PrtFile"};*/
	public static final String[] INTEGRATION_NAMED_REFERENCES = new String[] {
		"UGPART", "ALSG", "Text", "JTPART", 
		"PDF_Reference", "word", "excel", 
		"powerpoint", "Image", "JPEG_Reference", 
		"TIF_Reference", "AsmFile", "DrwFile", 
		"PrtFile"};

	public static final HashSet<String> INTEGRATION_NR_LOOKUP = new HashSet<>(Arrays.asList(INTEGRATION_NAMED_REFERENCES));
}
