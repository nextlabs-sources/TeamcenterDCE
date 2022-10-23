/*==================================================================================================

Copyright (c) 2008 Unigraphics Solutions Inc.
Unpublished - All rights reserved
====================================================================================================
File description:

File to be used in native signing


==================================================================================================*/
#include <stdafx.h>
#include <uf_defs.h>

static unsigned char nxauthblock[] = "NXAUTHBLOCK      "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"                                                  "
"      NXAUTHBLOCK";

extern DllExport void NXSigningResource(void)
{
	static bool doNothing = false;
	if (doNothing)
		nxauthblock[0] = 'N';
}
