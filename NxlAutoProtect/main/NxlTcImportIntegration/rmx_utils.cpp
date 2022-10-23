#include <stdafx.h>

#include <uf_ugmgr.h>
#include <NxlSCFIntegration/nxrmx_pdm_server.h>

bool teamcenter_protect(const char *partNumber, const char *partRev, const char *partFileType, const char *partFileName="")
{
	bool success = true;
	char encoded_name[MAX_FSPEC_BUFSIZE] = "";
	NX_CALL(UF_UGMGR_encode_part_filename(partNumber, partRev, partFileType, partFileName, encoded_name));
	DBGLOG("EncodedName='%s'", encoded_name);
	char cliName[MAX_FSPEC_BUFSIZE] = "";
	UF_UGMGR_convert_name_to_cli(encoded_name, cliName);
	DBGLOG("cliName='%s'", cliName);
	int outputCode = 0;
	char *outputString = NULL;
	DBGLOG("Invoke PDM(inputCode=%#X, inputString='%s')", INPUT_CODE_PROTECT, cliName);
	NX_EVAL(UF_UGMGR_invoke_pdm_server(INPUT_CODE_PROTECT, cliName, &outputCode, &outputString));
	DBGLOG("==>outputCode=%#X, outputstring='%s'", outputCode, outputString);
	if (outputString)
		UF_free(outputString);
	return success;
}