
#include <hook/hook_defs.h>
#include <hook/libdisp.h>
#include <NXRMX/nx_utils.h>
#include <NXRMX/nx_contexts.h>


/*
	ImageExportBuilder_commit
	public: virtual unsigned int __cdecl UGS::Gateway::ImageExportBuilder::commit(void) __ptr64
*/
static pfImageExportBuilder_commit ImageExportBuilder_commit_original;
static pfImageExportBuilder_commit ImageExportBuilder_commit_next;
static unsigned int ImageExportBuilder_commit_hooked(CPP_PTR obj)
{
	unsigned int ret = 0;
	CALL_START("ImageExportBuilder_commit(obj='%p')", obj);
	const char* fileName = ImageExportBuilder_GetFileName(obj);
	//0-png 1-jpg 2-gif 3-tif 4-bmp
	CPP_ENUM fileFormat = ImageExportBuilder_GetFileFormat(obj);
	DBGLOG("Exporting display parts to '%s' in %d", fileName, fileFormat);
	CALL_NEXT(ret = ImageExportBuilder_commit_next(obj));
	nx_on_exported_visibles(fileName);
	CALL_END("ImageExportBuilder_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}

void install_export_image_2007()
{
	//File->Export->Images
	HOOK_API("libdisp.dll", ImageExportBuilder_commit);
}