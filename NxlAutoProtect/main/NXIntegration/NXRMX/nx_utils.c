#include <uf_defs.h>
#include <uf.h>
#include <uf_ui.h>
#include <uf_part.h>
#include <uf_obj.h>
#include <uf_mdlcmp.h>
#include <uf_mtx.h>
#include <uf_assem.h>
#include <uf_cfi.h>
#include <uf_view.h>
#include <uf_draw.h>
#include <Windows.h>

/*nxlutils*/
#include <utils_windows.h>
#include <utils_nxl.h>
//
#include "nx_utils.h"
#include <hook/hook_defs.h>
#include <hook/libugmr.h>
#include <hook/libsyss.h>

int report_error(const char *file, int line, const char *func, const char *call, int ret)
{
    if (ret)
    {
        char errMsg[133];
        UF_get_fail_message(ret, errMsg);
		NXERR("%s():%s returns %d(Error:%s) @Line-%d in File-%s", func, call, ret, errMsg, line, file);
    }
	else
	{
		TRACELOG("%s():%s returns %d", func, call, ret);
	}

    return (ret);
}

void nx_dialog_show(char *msg)
{
	int ret = UF_UI_lock_ug_access( UF_UI_FROM_CUSTOM );
	if(ret == UF_UI_LOCK_SET)
	{
		LOG_CALLED(uc1601( msg, 1 ));
		NX_CALL(UF_UI_unlock_ug_access( UF_UI_FROM_CUSTOM ));
	}
	else
	{
		NXERR("UF_UI_lock_ug_access returns %d", ret);
	}
}

void nx_dialog_show_lines(char **lines, int nlines)
{
	char *msg = string_dup(strings_join(lines, nlines, "*"));
	nx_dialog_show(msg);
	string_free(msg);
}

void nx_show_status(char *status)
{
	int ret = UF_UI_lock_ug_access( UF_UI_FROM_CUSTOM );
	if(ret == UF_UI_LOCK_SET)
	{
		LOG_CALLED(uc1601( status, 0 ));
		NX_CALL(UF_UI_unlock_ug_access( UF_UI_FROM_CUSTOM ));
	}
	else
	{
		NXERR("UF_UI_lock_ug_access returns %d", ret);
	}
}

void nx_show_listing_window(const char *msg) {
	//ensure listing window is opened
	logical windowOpening = false;
	if (!NX_EVAL(UF_UI_is_listing_window_open(&windowOpening))
		|| !windowOpening)
	{
		//if eval failed or window is not opening, force open it
		NX_CALL(UF_UI_open_listing_window());
	}
	UF_UI_write_listing_window(msg);
}

BOOL nx_isugmr()
{
	static BOOL initialized = FALSE;
	static BOOL isugmr = FALSE;
	if(!initialized)
	{
		logical isactive = FALSE;
		UF_is_ugmanager_active(&isactive);
		isugmr = isactive?TRUE:FALSE;
		NXSYS("is_ugmgr=%d", isugmr);
		initialized = TRUE;
	}
	return isugmr;
}

std::string tag_to_name(tag_t partTag)
{
	if(partTag != NULL_TAG)
	{
		char partName[MAX_FSPEC_BUFSIZE];
		if(NX_EVAL(UF_PART_ask_part_name(partTag, partName)))	
		{
			//NXDBG("[%u]Name:%s", partTag, partName);
			return partName;
		}
	}
	return std::string();
}
std::string name_to_display(const char *name) {

	char *dispName = NULL;
	std::string ret;
	if (NX_EVAL(UF_PART_file_name_for_display(name, &dispName)))
	{
		ret = dispName;
		UF_free(dispName);
	}
	else
	{
		ret = name;
	}
	return ret;
}
bool name_is_managed(const char* partName) {
	char cliName[MAX_FSPEC_BUFSIZE] = "";
	bool ismanaged = NX_CALL(UF_UGMGR_convert_name_to_cli(partName, cliName)) == ERROR_OK;
	TRACELOG("'%s':ismanaged=%d cliName='%s'", partName, ismanaged, cliName);
	return ismanaged;
}
std::string name_to_file(const char *partName)
{
	if(nx_isugmr())
	{
		//managed mode
		TableEntry_PTR entry = TableEntry_LookupBySpec(partName);
		if(entry != NULL)
		{
			tag_t tag = TableEntry_GetPartTag(entry);
			const char *name = TableEntry_GetPathName(entry);
			if (name != nullptr) {
				return name;
			}
		}
		else if (file_exist(partName)) {
			//for unmangaged part, part name is the file path
			return partName;
		}
	}
	else
	{
		//unmanaged mode
		return partName;
	}
	return std::string();
}

std::vector<tag_t> part_list_all()
{
	int nParts = UF_PART_ask_num_parts();
	std::vector<tag_t> parts;
	//NXDBG("%d parts loaded in current session", nParts);
	if(nParts > 0)
	{
		int ipart;
		for(ipart=0; ipart < nParts; ipart++)
		{
			parts.push_back(UF_PART_ask_nth_part(ipart));
		}
	}
	return parts;
}
int part_ask_occs_of(tag_t part, tag_t rootPart) {
	tag_t* occs = NULL;
	int noccs = UF_ASSEM_ask_occs_of_part(rootPart, part, &occs);
	UF_free(occs);
	return noccs;
}
std::vector<tag_t> assembly_list_parts(tag_t rootPart)
{
	std::vector<tag_t> parts;
	if (rootPart != NULLTAG)
	{
		int ipart;
		int nParts = UF_PART_ask_num_parts();
		for (ipart = 0; ipart < nParts; ipart++)
		{
			tag_t part = UF_PART_ask_nth_part(ipart);
			if (part == rootPart)
			{
				NXDBG("[%d/%d]RootPart=%u", ipart + 1, nParts, part);
				parts.insert(parts.begin(), part);
			}
			else
			{
				int noccs = part_ask_occs_of(part, rootPart);
				if (noccs > 0)
				{
					NXDBG("[%d/%d]Part=%u nOccs=%d", ipart + 1, nParts, part, noccs);
					parts.push_back(part);
				}
			}
		}
	}
	return parts;

}
std::vector<tag_t> part_list_display_parts()
{
	tag_t displayPart = UF_PART_ask_display_part();
	NXDBG("DisplayPart=%u", displayPart);
	return assembly_list_parts(displayPart);
}

std::vector<tag_t> part_get_solidBodies(tag_t partTag)
{
	tag_t body = NULL_TAG;
	std::vector<tag_t> solidBodies;

	while(NX_EVAL(UF_OBJ_cycle_objs_in_part(partTag, UF_solid_type, &body)))
	{
		int type;
		int subtype;
		if(body != NULL_TAG
			&& NX_EVAL(UF_OBJ_ask_type_and_subtype(body, &type, &subtype)))
		{
			if(type == UF_solid_type
				&& subtype == UF_solid_body_subtype)
			{
				//NXDBG("[%u]SolidType:%u Type=%d SubType=%d", partTag, body, type, subtype);
				solidBodies.push_back(body);
			}
			continue;
		}
		break;
	}
	//NXDBG("[%u] has %d SolidBodies", partTag, solidBodies->count);
	return solidBodies;
}
tag_t object_find_owning_part(tag_t obj)
{
	if(obj != NULLTAG)
	{
		int type, subtype;
		char name[UF_OBJ_NAME_BUFSIZE];
		UF_OBJ_ask_name(obj, name);//object may not have name
		NX_EVAL(UF_OBJ_ask_type_and_subtype(obj, &type, &subtype));
		if(UF_ASSEM_is_part_occurrence(obj))
		{
			//this is a part occurrence, return it's prototype
			tag_t part = UF_ASSEM_ask_prototype_of_occ(obj);
			NXDBG("[%d]-{Name='%s' Type=%d Subtype=%d}:IsPartOccurrence PrototypeOfPartOcc=%d", obj, name, type, subtype, part);
			return part;
		}
		else if(type == UF_solid_type && subtype == UF_solid_face_subtype)
		{
			//for solid face, find part in its solid body
			tag_t body;
			NX_EVAL(UF_MODL_ask_face_body(obj, &body));
			NXDBG("[%d]-{Name='%s' Type=%d Subtype=%d}:IsSolidFace FaceBody=%d...", obj, name, type, subtype, body);
			return object_find_owning_part(body);
		}
		else if(type == UF_solid_type && subtype == UF_solid_edge_subtype)
		{
			//for solid edge, find part in its solid body
			tag_t body;
			NX_EVAL(UF_MODL_ask_edge_body(obj, &body));
			NXDBG("[%d]-{Name='%s' Type=%d Subtype=%d}:IsSolidEdge EdgeBody=%d...", obj, name, type, subtype, body);
			return object_find_owning_part(body);
		}
		else if(UF_ASSEM_is_occurrence(obj))
		{
			//mostly for solid body, find part in its part occurrence
			tag_t partOcc = UF_ASSEM_ask_part_occurrence(obj);
			NXDBG("[%d]-{Name='%s' Type=%d Subtype=%d}:IsOccurrence PartOccurrence=%d...", obj, name, type, subtype, partOcc);
			return object_find_owning_part(partOcc);
		}
		else
		{
			tag_t part = NULLTAG;
			//UF_DRAW_drafting_curve_type_t curveType;
			//NX_EVAL(UF_DRAW_ask_drafting_curve_type(obj, &curveType));
			//if(curveType != UF_DRAW_unknown_type)
			//{
			//	//this is a drafting curve, find in its parents
			//	int nparents = 0;
			//	tag_t *parents;
			//	if(NX_EVAL(UF_DRAW_ask_drafting_curve_parents(obj, &nparents, &parents)))
			//	{
			//		int iparent;
			//		NXDBG("[%d]-{Name='%s' Type=%d Subtype=%d curveType=%d}:nPrents=%d...", obj, name, type, subtype, curveType, nparents);
			//		for(iparent=0;iparent<nparents;iparent++)
			//		{
			//			NXDBG("[%d]-{Name='%s' Type=%d Subtype=%d curveType=%d}:CurvePrents[%d/%d]=%d...", obj, name, type, subtype, curveType, iparent+1, nparents, parents[iparent]);
			//			part = object_find_owning_part(parents[iparent]);
			//		}
			//		UF_free(parents);
			//	}
			//}
			if(part == NULLTAG)
			{
				//for other objects
				NX_EVAL(UF_OBJ_ask_owning_part(obj, &part));
				NXDBG("[%d]-{Name='%s' Type=%d Subtype=%d}:OwningPart=%d", obj, name, type, subtype, part);
			}
			return part;
		}
	}
	return NULLTAG;
}
static BOOL find_visible_parts_in_view(tag_t view, std::vector<tag_t> &visibleParts)
{
	int nvisibles, nclipped;
	tag_t *visibles, *clipped;
	//there are two kind of objects in drawing views
	//1, part occurrence(Type=63 SubType=1): these objects reflects the occurrences of selected parts
	//2, drawing curves(Type=1, 2, 5, and etc):these objects may be created fro silhouettes the selected parts
	//when the view is up to date, the owning parts of these objects are consistent with the selected parts shown in assembly navigator
	//when the view is out of date, the owning part of #1 objects are consistent with the selected parts, while #2 objects may not
	//because the #2 objects are still visible in work view and will be exported into new files, 
	//we should take the owning part of these objects as the source of metadata merging
	if(NX_EVAL(UF_VIEW_ask_visible_objects(view, &nvisibles, &visibles, &nclipped, &clipped)))
	{
		int i;
		tag_t part;
		NXDBG("UF_VIEW_ask_visible_objects returns %d visible objects, %d clipped objects", nvisibles, nclipped);
		for(i=0; i<nvisibles; i++)
		{
			TRACELOG("VisibleObjects[%d/%d]:%d...", i+1, nvisibles, visibles[i]);
			part = object_find_owning_part(visibles[i]);
			if (part != NULLTAG
				&& !tags_contains(visibleParts, part))
			{
				visibleParts.push_back(part);
			}
		}
		UF_free(visibles);
		UF_free(clipped);
		return TRUE;
	}
	return FALSE;
}

BOOL drawing_list_visible_parts(tag_t drawing, std::vector<tag_t> &visibleParts)
{
	char name[UF_OBJ_NAME_BUFSIZE];
	UF_VIEW_type_t viewType;
	UF_VIEW_subtype_t viewSubType;
	if(drawing == NULLTAG)
	{
		//search in current view
		tag_t workview = NULLTAG;
		if(NX_EVAL(UF_VIEW_ask_work_view(&workview))
			&& NX_EVAL(UF_VIEW_ask_type(workview, &viewType, &viewSubType)))
		{
			NX_EVAL(UF_OBJ_ask_name(workview, name));
			if(viewType == UF_VIEW_MODEL_TYPE)
			{
				//work view is in modeling mode, searching part in workview
				NXDBG("WorkView-%d('%s')=ModelingView:ViewType=%d Subtype=%d", workview, name, viewType, viewSubType);
				return find_visible_parts_in_view(workview, visibleParts);
			}
			else
			{
				//work view is in drafting mode, get current drawing
				if(NX_EVAL(UF_DRAW_ask_current_drawing(&drawing)))
				{
					NXDBG("WorkView-%d('%s')=DraftingView:ViewType=%d Subtype=%d CurrentDrawing=%d...", workview, name, viewType, viewSubType, drawing);
					return drawing_list_visible_parts(drawing, visibleParts);
				}
			}
		}
	}
	else if(REG_get_dword(REG_ROOT_KEY, "EnumerateDrawingObjects", 0) <= 0)
	{
		NXDBG("Enumerating Drawing Objects is disabled, use all parts in displayed part");
		std::vector<tag_t> allparts = part_list_display_parts();
		visibleParts.insert(visibleParts.end(), allparts.begin(), allparts.end());
		return TRUE;
	}
	else
	{
		int nDrawViews;
		tag_t *drawViews;
		logical outofdate;
		NX_EVAL(UF_OBJ_ask_name(drawing, name));
		NX_EVAL(UF_DRAW_is_object_out_of_date(drawing, &outofdate));
		NXDBG("InputDrawing-%d:Name='%s' OutOfDate=%d", drawing, name, outofdate);
		if(NX_EVAL(UF_DRAW_ask_views(drawing, &nDrawViews, &drawViews)))
		{
			int iview;
			BOOL ret = FALSE;
			for(iview=0; iview<nDrawViews; iview++)
			{
				NX_EVAL(UF_OBJ_ask_name(drawViews[iview], name));
				NX_EVAL(UF_VIEW_ask_type(drawViews[iview], &viewType, &viewSubType));
				NX_EVAL(UF_DRAW_is_object_out_of_date(drawViews[iview], &outofdate));
				NXDBG("DrawingView[%d/%d]-'%s'(%d):Type=%d SubType=%d OutOfDate=%d", iview+1, nDrawViews, name, drawViews[iview], viewType, viewSubType, outofdate);
				if(find_visible_parts_in_view(drawViews[iview], visibleParts))
				{
					//any true is true
					ret = TRUE;
				}
			}
			UF_free(drawViews);
			return ret;
		}
	}
	return FALSE;
}
std::vector<tag_t> export_find_source_parts(const std::vector<tag_t> &objects)
{
	std::vector<tag_t> sourceParts;
	if(objects.size() > 0)
	{
		int i;
		for(i=0; i<objects.size(); i++)
		{
			int type;
			int subtype;
			if(NX_EVAL(UF_OBJ_ask_type_and_subtype(objects[i], &type, &subtype)))
			{
				NXDBG("Object[%d][%u]:Type=%d SubType=%d", i, objects[i], type, subtype);
				//if(type == UF_solid_type
				//	&& subtype == UF_solid_body_subtype)
				{
					tag_t partTag = object_find_owning_part(objects[i]);
					if(partTag != NULLTAG
						&& !tags_contains(sourceParts, partTag))
					{
						sourceParts.push_back(partTag);
					}
				}
			}
		}
		NXDBG("InputObjects=%d SourcePart=%d"
			, objects.size(), sourceParts.size());
	}
	return sourceParts;
}
std::vector<tag_t> selection_ask_objects()
{
	extern UF_UI_selection_p_t g_selectionUI;
	int nselected;
	tag_t *objs = NULL;
	if(g_selectionUI != NULL
		&& NX_EVAL(UF_UI_ask_sel_object_list(g_selectionUI, &nselected, &objs)))
	{
		NXDBG("%p-Selected %d objects", g_selectionUI, nselected);
	}
	else if(NX_EVAL(UF_UI_ask_global_sel_object_list(&nselected, &objs)))
	{
		NXDBG("Global Selcted %d objects", nselected);
	}
	if(objs != NULL)
	{
		std::vector<tag_t> selectedObjects = tags_make(objs, nselected);
		UF_free(objs);
		return selectedObjects;
	}
	return std::vector<tag_t>();
}

compare_result_t nx_name_compare(const char *srcName, const char *tarName, BOOL *fuzzyMatch)
{
	if(srcName == NULL || tarName == NULL) return equal_no;
	//compare char by char
	int ichar = 0;
	char schar, tchar;
	if(fuzzyMatch != NULL) *fuzzyMatch = FALSE;
	while(TRUE)
	{
		schar = srcName[ichar];
		tchar = tarName[ichar];
		if(schar == '\0')
		{
			//source name is matched with target name
			if(tchar != '\0')
			{
				NXDBG("PartialMatch:Source='%s' Target-'%s'", srcName, tarName);
				return equal_partial;
			}
			else
			{
				NXDBG("FullMatch:Source='%s' Target-'%s'", srcName, tarName);
				return equal_full;
			}
		}
		else if(schar == tchar
			|| tolower(schar) == tolower(tchar))
		{
			//char matched
			ichar++;
			continue;
		}
		//treat _ in target name as a wildcard for non-word and non-number chars
		else if(tchar == '_'
			&& schar != '\\'	//not path seperator
			&& !(schar >= 'a' && schar <= 'z')//not lower case char
			&& !(schar >= 'A' && schar <= 'Z')//not upper case char
			&& !(schar >= '0' && schar <= '9'))//not number
		{
			if(fuzzyMatch != NULL) *fuzzyMatch = TRUE;
			NXDBG("FuzzyCharFound:'%c'", schar);
			ichar++;
			continue;
		}
		else
		{
			//not matched
			NXDBG("NotMatch:(Source='%s' Target-'%s')", srcName, tarName);
			return equal_no;
		}
	}
}

void HandleLastException()
{
	UException_PTR except = Exception_askLastException();
	if(except == NULL) return;
	//log basic exception information
	NXDBG("==>Type:%d", Exception_askType(except));
	const char *syslogMessage = Exception_askSyslogMessage(except);
	NXDBG("==>SyslogMessage:%s", syslogMessage);
	int code = Exception_askCode(except);
	NXDBG("==>Code:%d", code);
}

#define ENV_UGII_TMP "UGII_TMP_DIR"
std::wstring nx_get_tmp()
{
	//set UGII_TMP_PATH as rpm folder
	wchar_t buffer[MAX_PATH];
	if (GetEnvironmentVariableW(WTEXT(ENV_UGII_TMP), buffer, sizeof(buffer)/sizeof(wchar_t)) > 0)
	{
		return buffer;
	}
	NXWAR("'%s' is not defined!", ENV_UGII_TMP);
	return std::wstring();
}

int nx_get_major_version()
{
	static int majorVersion = 0;
	if (majorVersion == 0)
	{
		int minor, maint, build;
		if (module_query_ver("libsyss.dll", &majorVersion, &minor, &maint, &build))
		{
			INFOLOG("NX Version=%d.%d.%d.%d", majorVersion, minor, maint, build);
		}
		else
		{
			ERRLOG("Failed to query version from libsyss.dll");
		}
	}
	return majorVersion;
}