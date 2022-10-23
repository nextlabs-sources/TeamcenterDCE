/** 
    @file 

    This file contains the declaration for the Dispatch Library  NxlUtils

*/

#ifndef NXL_UTILS_EXPORTS_H_INCLUDED
#define NXL_UTILS_EXPORTS_H_INCLUDED

#ifdef WNT
    #if defined(NXLUTILS_EXPORTS)
        #define NXLAPI __declspec(dllexport)
    #else
        #define NXLAPI __declspec(dllimport)
    #endif
#else
    #define NXLAPI
#endif

#endif