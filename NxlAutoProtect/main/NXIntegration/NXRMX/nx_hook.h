#ifndef NX_INTEGRATION_HOOK_H_INCLUDED
#define NX_INTEGRATION_HOOK_H_INCLUDED

extern void fcc_hook();
extern void libpart_hook();
extern void libugmr_hook();
extern void libpartutils_hook();
extern void libugui_hook();
extern void libsyss_hook();
extern void libpartdisp_hook();
extern void liblwks_hook();
extern void libjadex_hook();
extern void libugutilsint_hook();
extern void libjafreeformsurfaces_hook();

extern void hook_save_bookmark();
extern void fix_export_outside_teamcenter();
extern void fix_fcc_download();
extern void hook_save_part_with_jt_data();

bool install_nx_hook();
#endif