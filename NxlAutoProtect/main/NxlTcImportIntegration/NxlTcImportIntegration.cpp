// NxlTcImportIntegration.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NxlTcImportIntegration.h"
#include <vector>
#include <string>
#include <uf_ugmgr.h>
#include <uf_exit.h>
#include <ImportContext.hpp>

ImportContext *s_pImportContext;
// This is an example of an exported function.
NXLTCIMPORTINTEGRATION_API void ufsta(char *param, int *returnCode, int rlen)
{
	if (NX_EVAL(UF_initialize()))
	{
		DBGLOG("Initializing ImportContext...");
		s_pImportContext = new ImportContext();
		RegisterImportListeners();
	}
	UF_terminate();
}