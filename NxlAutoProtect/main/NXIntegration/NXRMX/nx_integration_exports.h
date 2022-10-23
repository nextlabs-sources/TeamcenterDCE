/** 
    @file 

    This file contains the declaration for the Dispatch Library  NXIntegration
*/

#ifndef NX_INTEGRATION_EXPORTS_H_INCLUDED
#define NX_INTEGRATION_EXPORTS_H_INCLUDED

#ifdef WNT
    #if defined(NX_INTEGRATION_EXPORTS)
        #define NXIAPI __declspec(dllexport)
    #else
        #define NXIAPI __declspec(dllimport)
    #endif
#else
    #define NXIAPI
#endif

#endif