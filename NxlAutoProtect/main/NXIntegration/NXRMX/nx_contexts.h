/*
	The NX events which will be triggered by NX hook
*/
#ifndef NX_CONTEXTS_H_INCLUDED
#define NX_CONTEXTS_H_INCLUDED
#include <uf.h>
#include "nx_utils.h"
#include <NXRMX/PartContext.h>
#include <NXRMX/SpawnContext.h>
#include <NxlUtils\NxlPath.hpp>

void nx_begin_export();
void nx_end_export();
//part file is loaded into NX
//read stream is opened or closed
void nx_on_part_loaded(tag_t partTag, const char *partName, const char *partFile);
//write stream is opened
void nx_on_part_saving(tag_t partTag, const char *targetFile);
void nx_on_part_saved(tag_t partTag, const char *tmpFile);

void overlay_force_refresh();
void UpdateButtonSensitivity();

/*	1 ==> 1
	One part file is being exported into one file
	the target file will have same metadata with the source part file
*/
//exporting in 1 to 1 modeL:one part file is exported into another file
#define nx_on_exported_1to1(p, f) NXDBG("1to1Export:FromPart-%u, ToFile-'%s'", p, f);nx_on_part_saved(p, f)

/*	M ==> 1
	Multiple part files are being exported into one file
	the target file will contain the merged metadata of these input part files
	the merged metadata are based on metadata priorities which should be defined by policy obligation
*/
void nx_on_exporting_objects(const std::vector<tag_t> &objects, tag_t toPart);
//Selected Objects are exported into one file
void nx_on_exported_objects(const std::vector<tag_t> &bodies, const char *exportedFile);
//Selected Parts are exported into one file
bool nx_on_exported_parts(const std::vector<tag_t> &parts, const char *exportedFile);
//the whole assembly(all parts) are exported into one file
void nx_on_exported_all(const char *exportedFile);
//the visible objects in input drawing are exported into one file
void nx_on_exported_drawing(tag_t drawing, const char *exportedFile);
void nx_on_exported_associated(const char *file, const char *extOfAssociated);
//the visible objects in current display are exported into one file
#define nx_on_exported_visibles(exportedFile) nx_on_exported_drawing(NULLTAG, exportedFile)

/*	X ==> 1
	The source parts are unknown, 
	the target file will contain the merged metadata of the whole assembly
	the merged metadata are based on metadata priorities which should be defined by policy obligation
	Note:this usage should be minimized
*/
//exporting in X to 1 mode:the usage of this should be minimized
#define nx_on_exported_xto1(file) NXWAR("Xto1Export:'%s'", file);nx_on_exported_all(file)
//WORKAROUND: this is a workaround solution to seach the source part file by the file name of the new created file
//use this when we know that the new created file name is based on the source part name
//if the source part is found, enforce it as 1to1 export, otherwise enforce it as xto1 export
void nx_on_exported_fuzzy(const char *exportedFile);

//read stream is opened, make sure the protected file has .nxl extension
void nx_on_file_reading(const char *file);
void nx_on_file_copied(const char *from, const char *to, bool appendNxl);
//the exporting is doing by spawned process
void nx_on_exporting_by_spawn(const char *inputFile, const char *outputFile);//temporarily this is only for iges

//contexts will be reset and initialized
void nx_on_initial();
//all context item will be processed
void nx_on_finalize();
void nx_on_reset();
/*
	Managed Mode
*/
//part file is downloaded to local
void nx_on_fcc_downloaded(const char *fccFile);
//local file is being saved to FCC
void nx_on_fcc_uploading(const char *tmpFile);
//local file is saved to Teamcenter
void nx_on_fcc_uploaded(const char *tmpFile, const char *volumeId);

void part_event_handler(UF_callback_reason_e_t reason, const void *pTag, void *closure);

void transient_rpm_dir_add(const char *dir);
void transient_rpm_dir_clear();

#endif