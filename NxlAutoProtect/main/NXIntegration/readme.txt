Components: currently there are three components for NXRMX
	NXRMX (was named as NXIntegration)
	MiniRMX (was named as NxlHook)
	NXDRMIntegration (newly introduced from NX2007)
	
Folders
/config:[deprecated] used for common configuration for NXIntegration
/hook: hook utils and common hook impls on windows apis
/MiniRMX: source files for MiniRMX component
/NXDRMIntegration: source files for NXDRMIntegration component
/NXRMX: source files for NXRMX component

/nx12: project files and source files for NX 12
/nx1980:project files and source files for NX 1980
/nx2007: project files and source files for NX 2007


/export_nxapis.bat: used to copy needed NX header files and lib files out of NX installation folder
	those files will to be uploaded to perforce server /external/siemens/nx/<versions>
/export_nxdll.bat: used to copy NX dll files that will be annalysed for the hooks

