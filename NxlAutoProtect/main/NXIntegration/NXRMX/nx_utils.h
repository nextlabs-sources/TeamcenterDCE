/*
	The NX utilities that doesn't related to Business Logics of Nextlabs
*/
#ifndef NX_USAGE_CONTROL_NX_UTILS_H_INCLUDED
#define NX_USAGE_CONTROL_NX_UTILS_H_INCLUDED
#include "nx_integration_exports.h"
#include <stdafx.h>
#include <uf.h>

extern char sMsgBuf[1024];
#define NXSYS(fmt, ...) {sprintf(sMsgBuf, PROJECT_NAME "!" fmt "\n", ##__VA_ARGS__);UF_print_syslog(sMsgBuf, FALSE);		INFOLOG(fmt, ##__VA_ARGS__);}
#define NXERR(fmt, ...) {sprintf(sMsgBuf, PROJECT_NAME "!ERROR!" fmt "\n", ##__VA_ARGS__);UF_print_syslog(sMsgBuf, TRUE); 		ERRLOG(fmt, ##__VA_ARGS__); }
#define NXWAR(fmt, ...) {sprintf(sMsgBuf, PROJECT_NAME "!WARN !" fmt "\n", ##__VA_ARGS__);UF_print_syslog(sMsgBuf, TRUE); 		WARLOG(fmt, ##__VA_ARGS__); }
#define NXDBG(fmt, ...) {DBGLOG(fmt, ##__VA_ARGS__); }
#define NXMSG(fmt, ...) {sprintf(sMsgBuf,	fmt "\n", ##__VA_ARGS__);	nx_show_listing_window(sMsgBuf);	LOG4CPLUS_INFO_FMT(DEFAULT_LOGGER, LOG4CPLUS_TEXT("UserMessage:") LOG4CPLUS_TEXT(fmt), ##__VA_ARGS__);}
#define NXONLY(fmt, ...) {sprintf(sMsgBuf, PROJECT_NAME "!" fmt "\n", ##__VA_ARGS__);UF_print_syslog(sMsgBuf, FALSE);OutputDebugString(sMsgBuf);}

//#define NXLDTL(fmt, ...) TRACELOG(fmt, ##__VA_ARGS__)

#define REG_ROOT_KEY "SOFTWARE\\NextLabs\\NXRMX"
//
//Debug Utilities
//
NXIAPI int report_error(const char *file, int line, const char *func, const  char *call, int ret);
//TRUE=success FALSE=ERROR
#define NX_EVAL(X) (!report_error( __FILE__, __LINE__, __FUNCTION__, #X, (X)))
#define NX_CALL(X) report_error( __FILE__, __LINE__, __FUNCTION__, #X, (X))
int nx_get_major_version();

//partial download is introduced from NX 12.0.2 managed mode
//fromn NX 2007, unmanaged mode also support minimal load
#define nx_partial_download_enabled() (nx_get_major_version() >= 12)

typedef unsigned int tag_t;
#ifndef NULLTAG
#define NULLTAG (tag_t)0
#endif

//type definitions
typedef struct tags_s
{
	int count;
	tag_t *tags;
}tags_t, *tags_p;

inline std::vector<tag_t> tags_make(tag_t *tags, int ntags) {
	return std::vector<tag_t>(tags, tags + ntags);
}
inline bool tags_contains(const std::vector<tag_t> &taglist, tag_t tag) {
	return std::find(taglist.begin(), taglist.end(), tag) != taglist.end();
}

NXIAPI void nx_dialog_show_lines(char **lines, int nlines);
NXIAPI void nx_dialog_show(char *msg);
NXIAPI void nx_show_status(char *status);
NXIAPI void nx_show_listing_window(const char *msg);
NXIAPI BOOL nx_isugmr();
std::wstring nx_get_tmp();
bool nxl_protection_is_enabled();

NXIAPI std::string tag_to_name(tag_t partTag);
NXIAPI std::string name_to_file(const char *partName);
NXIAPI std::string name_to_display(const char *name);
NXIAPI int part_ask_occs_of(tag_t part, tag_t rootPart);
NXIAPI std::vector<tag_t> part_list_all();
NXIAPI BOOL drawing_list_visible_parts(tag_t drawing, std::vector<tag_t> &visibleParts);
NXIAPI std::vector<tag_t> part_list_display_parts();
std::vector<tag_t> assembly_list_parts(tag_t rootPart);

NXIAPI std::vector<tag_t> export_find_source_parts(const std::vector<tag_t> &objects);

NXIAPI std::vector<tag_t> selection_ask_objects();

/*
	In NX, when the source file name contains some special characters(the currently known characters are space and dot)
	NX will replace it with underscore(_) as the final file name
		equal_no: two names are not matched
		equal_partial: source name is matched as a part of target name
		equal_full: two names are fullly matched 
		*fuzzyMatch: two names are matched while some of underline characters in target name are matched with non-word and non-num character in source name
			set it to NULL when caller doesn't care if result is fuzzy
*/
typedef enum compare_result_e {equal_no = 0, equal_partial, equal_full} compare_result_t;
NXIAPI compare_result_t nx_name_compare(const char *srcName, const char *tarName, BOOL *fuzzyMatch);

inline bool StringEndsWith(const std::string &fullString, const std::string &end) {
	size_t off = fullString.length() - end.length();
	return off >= 0
		&& _stricmp(&fullString[off], end.c_str());
}
#endif