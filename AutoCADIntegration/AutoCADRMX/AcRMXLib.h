#pragma once
#if defined(AutoCADRMX_EXPORTS)
#define ACRMXAPI __declspec(dllexport)
#else
#define ACRMXAPI __declspec(dllimport)
#endif

#if defined(__cplusplus)
extern "C" {
#endif
	ACRMXAPI bool AcRMX_ProtectExportFile(const wchar_t* szFileToProtect);
	ACRMXAPI bool AcRMX_CheckRights(unsigned long dwRights, bool bIncludRefs, bool bAlert);
#if defined(__cplusplus)
}
#endif

