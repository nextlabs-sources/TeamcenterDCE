
#ifndef NXL_SCF_INTEGRATION_H_INCLUDED
#define NXL_SCF_INTEGRATION_H_INCLUDED
#include <NxlSCFIntegration_exports.h>
#include <stdarg.h>
#include <tccore/method.h>

#ifdef __cplusplus
extern "C" {
#endif
	extern SCFIAPI int NxlSCFIntegration_register_callbacks();
	extern SCFIAPI int NxlSCFIntegration_register_handlers(int *decision, va_list args);

	extern SCFIAPI  int NXRMX_invoke_pdm_server(int *decision, va_list args);
	extern SCFIAPI int Nxl3_bcz_modifier(METHOD_message_t *msg, va_list args);
#ifdef __cplusplus
}
#endif
#endif //NXL_AUTO_PROTECT_H_INCLUDED
