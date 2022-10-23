/*
	Library:	libpartdisp.dll
	Version:	10.0.0.24
*/
#include <hook/hook_defs.h>
#include <hook/libpartdisp.h>
#include <NXRMX/nx_contexts.h>
#include <uf_assem.h>

/*
	CGME_create_cgm
	void __cdecl CGME_create_cgm(enum UF_CGM_export_reason_e,unsigned int,struct CGM_options_s * __ptr64,struct CGM_custom_colors_s * __ptr64,struct CGME_custom_widths_s * __ptr64,void (__cdecl*)(struct CGM_file_s * __ptr64,unsigned int,void * __ptr64),void * __ptr64,bool,char const * __ptr64)
*/
static pfCGME_create_cgm CGME_create_cgm_original;
static pfCGME_create_cgm CGME_create_cgm_next;
static void CGME_create_cgm_hooked(UF_CGM_export_reason_t p1, unsigned int p2, UF_CGM_export_options_p_t p3, UF_CGM_custom_colors_p_t p4, UF_CGM_custom_widths_p_t p5, CALLBACK_PTR p6, void * p7, BOOL p8, char const * p9)
{
	CALL_START("CGME_create_cgm(p1='%d', p2='%u', p3='%p', p4='%p', p5='%p', p6, p7, p8='%d', p9='%s')", p1, p2, p3, p4, p5, p8, p9);
	CALL_NEXT(CGME_create_cgm_next(p1, p2, p3, p4, p5, p6, p7, p8, p9));
	switch(p1)
	{
		//File -> Export -> User Defined Features
		case UF_CGM_misc_appl_reason:    /* All other CGM Export operations */
			if(p2 == NULLTAG)
			{
				tag_t workpart = UF_ASSEM_ask_work_part();
				nx_on_exported_1to1(workpart, p9);
			}
			else
			{
				nx_on_exported_drawing(p2, p9);
			}
			break;
		//File -> Export -> PDF
		case UF_CGM_pdf_hidden_text_reason:   /* File->Export PDF */
			//skip protection
			NXDBG("Creating cgm-'%s' for exporting PDF", p9);
			nx_on_exported_drawing(p2, p9);
			break;
		case UF_CGM_plot_reason:  /* Plotting either interactive, NXOpen or Grip */
		case UF_CGM_print_reason:     /* File->Print either interactive or NX Open */
		case UF_CGM_copy_display_reason: /* Edit->Copy Display */
		case UF_CGM_pdf_reason:          /* CGM is for a PDF */
		case UF_CGM_vised_hidden_text_reason:   /* Visual Editor */
		case UF_CGM_max_reasons:
			NXWAR("New Reason Found(%d)", p1);
		//File -> Export -> CGM
		case UF_CGM_export_reason:    /* CGM Export either interactive or NXOpen */
		default:
			{
				nx_on_exported_drawing(p2, p9);
			}
			break;
	}
	CALL_END("CGME_create_cgm(p1='%d', p2='%u', p3='%p', p4='%p', p5='%p', p6, p7, p8='%d', p9='%s')", p1, p2, p3, p4, p5, p8, p9);
}

/*
	CGME_export_cgm
	void __cdecl CGME_export_cgm(enum UF_CGM_export_reason_e,unsigned int,struct CGM_options_s * __ptr64,bool,char const * __ptr64)
*/
static pfCGME_export_cgm CGME_export_cgm_original;
static pfCGME_export_cgm CGME_export_cgm_next;
static void CGME_export_cgm_hooked(UF_CGM_export_reason_t p1, unsigned int p2, UF_CGM_export_options_p_t p3, BOOL p4, char const * p5)
{
	CALL_START("CGME_export_cgm(p1='%d', p2='%u', p3='%p', p4='%d', p5='%s')", p1, p2, p3, p4, p5);
	CALL_NEXT(CGME_export_cgm_next(p1, p2, p3, p4, p5));
	CALL_END("CGME_export_cgm(p1='%d', p2='%u', p3='%p', p4='%d', p5='%s')", p1, p2, p3, p4, p5);
}

/*
	PrintPDFBuilder_commit
	public: virtual unsigned int __cdecl UGS::Gateway::PrintPDFBuilder::commit(void) __ptr64
*/
static pfPrintPDFBuilder_commit PrintPDFBuilder_commit_original;
static pfPrintPDFBuilder_commit PrintPDFBuilder_commit_next;
static unsigned int PrintPDFBuilder_commit_hooked(PrintPDFBuilder_PTR obj)
{
	unsigned int ret = 0;
	CALL_START("PrintPDFBuilder_commit(obj='%p')", obj);
	CALL_NEXT(ret = PrintPDFBuilder_commit_next(obj));
	CALL_END("PrintPDFBuilder_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}

void libpartdisp_hook()
{
	//File->Export->CGM : reason = 1 (UF_CGM_export_reason)
	//File->Export->User Defined Features : reason = 5 (UF_CGM_misc_appl_reason)
	//File->Export->PDF : reason = 6 (UF_CGM_pdf_hidden_text_reason)
	HOOK_API("libpartdisp.dll", CGME_create_cgm);
	//HOOK_CPP(CGME_export_cgm, module);

	//File->Export->PDF : reason = 6 (UF_CGM_pdf_hidden_text_reason)
	HOOK_API("libpartdisp.dll", PrintPDFBuilder_commit);
}

