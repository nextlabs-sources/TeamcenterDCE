/*
===============================================================================
File description:

    File   : NxlHook_register_calls.c
    Module : Register NxlHook.dll

===============================================================================
*/

#include <tc/emh.h>
#include "hook_defs.h"
#include "hook_utils.h"
#include "hook_log.h"

/*************************************************************/
/* register the NxlHook callbacks							 */
/*      Method name must be {LIBRARY_NAME}_register_callbacks*/
/*                                                           */
/* Returns: ITK_ok - success                                 */
/*          !ITK_ok - otherwise                              */
/*************************************************************/
// This file is only here to tell TC that we had a hook module in this NxlHook.dll
// The main entry in dllmain.c will auto kick-in after this file is recognize by Teamcenter
// In Teamcenter Preferences: tc_customization_library should have NxlHook
 __declspec(dllexport) int NxlHook_register_callbacks()
{
	return ITK_ok;
}
