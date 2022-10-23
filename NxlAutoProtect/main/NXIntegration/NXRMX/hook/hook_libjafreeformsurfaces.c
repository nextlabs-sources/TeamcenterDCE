/*
	Library:	libjafreeformsurfaces.dll
*/
#include <hook/hook_defs.h>
#include <hook/libjafreeformsurfaces.h>
#include <NXRMX/nx_utils.h>
#include <NXRMX/nx_contexts.h>
#include <utils_string.h>
#include <hook/hook_file_closed.h>

//handle created dwg file
static std::vector<tag_t> s_selectedObjects;
static void HandleCreatedObjFile(const char *file, BOOL isWrite, LONGLONG fileSize)
{
	if (isWrite
		&& string_ends_with(file, ".obj", FALSE))
	{
		DBGLOG("FinishedWriting:'%s'(Size=%lld)", file, fileSize);
		nx_on_exported_objects(s_selectedObjects, file);
	}
}
/*
	ExportSubdivisionGeometryBuilder_commit
	public: virtual unsigned int __cdecl UGS::ModlSubdivJA::ExportSubdivisionGeometryBuilder::commit(void) __ptr64
*/
static pfExportSubdivisionGeometryBuilder_commit ExportSubdivisionGeometryBuilder_commit_original;
static pfExportSubdivisionGeometryBuilder_commit ExportSubdivisionGeometryBuilder_commit_next;
static unsigned int ExportSubdivisionGeometryBuilder_commit_hooked(SubdivisionGeometryBuilder_PTR obj)
{
	unsigned int ret = 0;
	CALL_START("ExportSubdivisionGeometryBuilder_commit(obj='%p')", obj);

	s_selectedObjects.clear();
	SELECT_OBJECT_list_PTR selectList = ExportSubdivisionGeometryBuilder_GetFeatureSet(obj);
	if (selectList != NULL)
	{
		int size = SELECT_OBJECT_list_ask_size(selectList);
		if (size > 0)
		{
			int nobjs = 0;
			tag_t* objs = SELECT_OBJECT_list_to_tag_array(selectList, &nobjs);
			s_selectedObjects = tags_make(objs, nobjs);
		}
	}
	register_FileClosedHandler(HandleCreatedObjFile);
	CALL_NEXT(ret = ExportSubdivisionGeometryBuilder_commit_next(obj));
	unregister_FileClosedHandler(HandleCreatedObjFile);
	CALL_END("ExportSubdivisionGeometryBuilder_commit(obj='%p') returns '%u'", obj, ret);
	return ret;
}


void libjafreeformsurfaces_hook()
{
	//File - Export - Subdivision Geometry
	HOOK_API("libjafreeformsurfaces.dll", ExportSubdivisionGeometryBuilder_commit);
}

