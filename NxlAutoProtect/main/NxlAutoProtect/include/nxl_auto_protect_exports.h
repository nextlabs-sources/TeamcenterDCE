
/** 
    @file 

    This file contains the declaration for the Dispatch Library  NxlAutoProtect

*/

#ifndef NXL_AUTO_PROTECT_EXPORTS
#define NXL_AUTO_PROTECT_EXPORTS

#ifdef WNT
    #if IPLIB==libNxlAutoProtect || defined(NXLAUTOPROTECT_EXPORT)
        #define DCEAPI __declspec(dllexport)
    #else
        #define DCEAPI __declspec(dllimport)
    #endif
#else
    #define DCEAPI
#endif

#endif