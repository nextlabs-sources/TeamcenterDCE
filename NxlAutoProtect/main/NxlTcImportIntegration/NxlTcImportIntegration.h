// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NXLTCIMPORTINTEGRATION_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NXLTCIMPORTINTEGRATION_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WNT
	#ifdef NXLTCIMPORTINTEGRATION_EXPORTS
	#define NXLTCIMPORTINTEGRATION_API __declspec(dllexport)
	#else
	#define NXLTCIMPORTINTEGRATION_API __declspec(dllimport)
	#endif
#else
#define NXLTCIMPORTINTEGRATION_API 
#endif

#ifdef __cplusplus
extern "C" {
#endif

NXLTCIMPORTINTEGRATION_API void ufsta(char *param, int *returnCode, int rlen);


#ifdef __cplusplus
}
#endif