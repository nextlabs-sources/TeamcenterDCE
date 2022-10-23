#pragma once
#include "TestFwkHlp.h"

#define DECLAR_TESTFUNCS() \
DECLAR_HLP_FUNC(SelRPMDir) \
DECLAR_HLP_FUNC(SelNonRPMDir) \
DECLAR_HLP_FUNC(ValidateNXL) \
DECLAR_HLP_FUNC(OpenLog) \
DECLAR_HLP_FUNC(PurgeOutput) \
DECLAR_HLP_FUNC(EndCase) \
DECLAR_HLP_FUNC(ViewInfo) \
DECLAR_HLP_FUNC(ViewModelInfo) \
DECLAR_TESTFUNC(Save) \
DECLAR_TESTFUNC(SaveAs) \
DECLAR_TESTFUNC(Backup) \
DECLAR_TESTFUNC(Export_CreoView) \
DECLAR_TESTFUNC(ExportPart_2D) \
DECLAR_TESTFUNC(ExportPart_3D) \
DECLAR_TESTFUNC(ExportPart_Image) \
DECLAR_TESTFUNC(ExportDrw) \
DECLAR_TESTFUNC(ExportAsm_2D) \
DECLAR_TESTFUNC(ExportAsm_3D) \
DECLAR_TESTFUNC(IpemSave) \
DECLAR_TESTFUNC(IpemSaveAs) \
DECLAR_TESTFUNC(IpemSaveAll)

#define DEFINE_TESTCOMMANDS() \
DEFINE_HLP_COMMAND(SelRPMDir) \
DEFINE_HLP_COMMAND(SelNonRPMDir) \
DEFINE_HLP_COMMAND(ValidateNXL) \
DEFINE_HLP_COMMAND(OpenLog) \
DEFINE_HLP_COMMAND(PurgeOutput) \
DEFINE_HLP_COMMAND(EndCase) \
DEFINE_HLP_COMMAND(ViewInfo) \
DEFINE_HLP_COMMAND(ViewModelInfo) \
DEFINE_TESTCOMMAND(Save, "") \
DEFINE_TESTCOMMAND(SaveAs, "") \
DEFINE_TESTCOMMAND(Backup, "") \
DEFINE_TESTCOMMAND(Export_CreoView, "ed|edz|pvs|pvz") \
DEFINE_TESTCOMMAND(ExportPart_2D, "dxf|stp|dwg|asc|pdf") \
DEFINE_TESTCOMMAND(ExportPart_3D, "igs|vda|neu|patranntr|cosmosntr|stl|iv|obj|slp|unv|gbf|asc|facet|shrinkwrap|sat|x_t|pdfu3d|u3d|zip|amf") \
DEFINE_TESTCOMMAND(ExportAsm_2D, "dxf|stp|dwg|pdf|png|tiff|jpg|eps|tifsnap|pngsnap|pic") \
DEFINE_TESTCOMMAND(ExportAsm_3D, "igs|vda|neu|stp|stl|iv|obj|slp|www|unv|wrl|gbf|asc|facet|shrinkwrap|sat|x_t|pdfu3d|u3d|zip|amf") \
DEFINE_TESTCOMMAND(ExportPart_Image, "tif|jpg|eps|tifsnap|pic|png|pngsnap") \
DEFINE_TESTCOMMAND(ExportDrw, "igs|dxf|stp|cgm|dwg|pdf|she|tsh|tif|pic|zip|pngsnap") \
DEFINE_TESTCOMMAND(IpemSave, "") \
DEFINE_TESTCOMMAND(IpemSaveAs, "") \
DEFINE_TESTCOMMAND(IpemSaveAll, "")
