#ifndef NXL_SCF_INTEGRATION_EXPORTS_H_INCLUDED
#define NXL_SCF_INTEGRATION_EXPORTS_H_INCLUDED

#ifdef WNT
#if IPLIB==NxlSCFIntegration || defined(NxlSCFIntegration_EXPORT)
#define SCFIAPI __declspec(dllexport)
#else
#define SCFIAPI __declspec(dllimport)
#endif
#else
#define SCFIAPI
#endif

#endif