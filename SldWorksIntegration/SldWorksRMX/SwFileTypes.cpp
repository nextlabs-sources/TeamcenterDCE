#include "SwFileTypes.h"

std::map <wstring, SwSourceFileType> SwExtnToFileTypeEnumMap = {
	{ L".SLDPRT",   SwSourceFileType::SW_SLDPRT },
	{ L".SLDASM",   SwSourceFileType::SW_SLDASM },
	{ L".SLDDRW",   SwSourceFileType::SW_SLDDRW },
	{ L".SLDLFP",   SwSourceFileType::SW_SLDLFP },
	{ L".PRTDOT",   SwSourceFileType::SW_PRTDOT },
	{ L".ASMDOT",   SwSourceFileType::SW_ASMDOT },
	{ L".DRWDOT",   SwSourceFileType::SW_DRWDOT },
	{ L".PRT",		SwSourceFileType::SW_PRT },
	{ L".ASM",      SwSourceFileType::SW_ASM },
	{ L".DRW",	    SwSourceFileType::SW_DRW },
	{ L".SLDALPRT", SwSourceFileType::SW_SLDALPRT },
	{ L".SLDALASM", SwSourceFileType::SW_SLDALASM }	
};


const exportFileHandlerMap SwFileTypes::exportFileSLDPRT = { { L".SLDPRT",SwFileHandlerMethod::USE_COPY_HOOK } /*SolidWorks Part */,
{ L".PRT",SwFileHandlerMethod::USE_COPY_HOOK } /*Part Files*/ ,{ L".SLDLFP",SwFileHandlerMethod::USE_COPY_HOOK } /*Lib Feat Part*/,{ L".SLDALPRT",SwFileHandlerMethod::USE_COPY_HOOK },
{ L".PRTDOT",SwFileHandlerMethod::USE_COPY_HOOK } /*Part Templates*/,{ L".SLDFTP",SwFileHandlerMethod::USE_NORMAL } /*Form Tools*/,
{ L".X_T",SwFileHandlerMethod::USE_NORMAL } /*ParaSolid */ ,{ L".X_B",SwFileHandlerMethod::USE_NORMAL } /*Parasolid Binary*/,
{ L".IGS",SwFileHandlerMethod::USE_NORMAL } /*Initial Graphics Exchange Specification*/,{L".STEP",SwFileHandlerMethod::USE_NORMAL }, { L".STP",SwFileHandlerMethod::USE_NORMAL } /*Step Files*/,
{ L".IFC",SwFileHandlerMethod::USE_NORMAL }, { L".SAT",SwFileHandlerMethod::USE_CREATE_HOOK }, { L".VDA",SwFileHandlerMethod::USE_NORMAL }, { L".WRL",SwFileHandlerMethod::USE_NORMAL }, { L".STL",SwFileHandlerMethod::USE_NORMAL },
{ L".3MF",SwFileHandlerMethod::USE_NORMAL } /*3D Manufacturing Format*/, { L".AMF",SwFileHandlerMethod::USE_CREATE_HOOK }/*Additive manufacturing file format*/,
{ L".EPRT",SwFileHandlerMethod::USE_NORMAL }, { L".3DXML",SwFileHandlerMethod::USE_NORMAL }/*3D XML*/, { L".XAML",SwFileHandlerMethod::USE_NORMAL }, { L".CGR",SwFileHandlerMethod::USE_NORMAL } /*CATIA Graphics Files*/, { L".HCG",SwFileHandlerMethod::USE_NORMAL } /*Highly Compressed Graphics*/,
{ L".HSF",SwFileHandlerMethod::USE_NORMAL }, { L".DXF",SwFileHandlerMethod::USE_NORMAL }/*Autocad Drawing Exchange Format*/, { L".DWG",SwFileHandlerMethod::USE_NORMAL }/*AutoCAD .dwg file format*/,
{ L".PDF", SwFileHandlerMethod::USE_NORMAL } /*Adobe Protable Document format*/, { L".PSD",SwFileHandlerMethod::USE_NORMAL }/*Adobe Photoshop*/, { L".AI", SwFileHandlerMethod::USE_COPY_HOOK }/*Adobe Illustrator */,
{ L".JPG", SwFileHandlerMethod::USE_NORMAL }, { L".PNG", SwFileHandlerMethod::USE_NORMAL }/*Portable Network Graphics Files*/, { L".TIF", SwFileHandlerMethod::USE_NORMAL } ,{L".JT",SwFileHandlerMethod::USE_CREATE_HOOK},
{L".SMG" , SwFileHandlerMethod::USE_NORMAL},{L".PLY",SwFileHandlerMethod::USE_NORMAL},{L".RFA",SwFileHandlerMethod::USE_PROCESS_HOOK},{L".SWAR",SwFileHandlerMethod::USE_COPY_HOOK } };

const exportFileHandlerMap SwFileTypes::exportFileSLDASM = { {L".ASM",SwFileHandlerMethod::USE_COPY_HOOK },{L".SLDASM",SwFileHandlerMethod::USE_COPY_HOOK} /*Assembly Files*/,
{L".ASMDOT",SwFileHandlerMethod::USE_COPY_HOOK} /*Assembly Templates*/,{L".SLDALASM",SwFileHandlerMethod::USE_COPY_HOOK} /*Analysis Lib Assembly */,
{L".PRT",SwFileHandlerMethod::USE_NORMAL},{L".SLDPRT",SwFileHandlerMethod::USE_COPY_HOOK} /*Part Files */,{L".X_T",SwFileHandlerMethod::USE_NORMAL} /*ParaSolid */ , {L".X_B",SwFileHandlerMethod::USE_NORMAL} /*Parasolid Binary*/,
{L".IGS",SwFileHandlerMethod::USE_NORMAL} /*Initial Graphics Exchange Specification*/,{L".STEP",SwFileHandlerMethod::USE_NORMAL},{L".STP",SwFileHandlerMethod::USE_NORMAL} /*Step Files*/,
{L".IFC",SwFileHandlerMethod::USE_NORMAL},{L".SAT",SwFileHandlerMethod::USE_CREATE_HOOK }, {L".STL",SwFileHandlerMethod::USE_CREATE_HOOK},{L".3MF",SwFileHandlerMethod::USE_NORMAL} /*3D Manufacturing Format*/,{L".AMF",SwFileHandlerMethod::USE_CREATE_HOOK}/*Additive manufacturing file format*/,
{L".WRL",SwFileHandlerMethod::USE_NORMAL},{L".EASM",SwFileHandlerMethod::USE_CREATE_HOOK},{L".3DXML",SwFileHandlerMethod::USE_CREATE_HOOK},{L".XAML",SwFileHandlerMethod::USE_CREATE_HOOK},{L".CGR",SwFileHandlerMethod::USE_NORMAL},{L".HCG",SwFileHandlerMethod::USE_NORMAL},{L".HSF",SwFileHandlerMethod::USE_NORMAL} /*HOOPS HSF */,
{L".PDF",SwFileHandlerMethod::USE_CREATE_HOOK} /*Adobe Protable Document format*/,{L".PSD",SwFileHandlerMethod::USE_CREATE_HOOK}/*Adobe Photoshop*/, {L".AI",SwFileHandlerMethod::USE_COPY_HOOK}/*Adobe Illustrator */,
{L".JPG",SwFileHandlerMethod::USE_NORMAL },{L".PNG",SwFileHandlerMethod::USE_NORMAL } /*Portable Network Graphics Files*/ ,{L".TIF",SwFileHandlerMethod::USE_NORMAL },{ L".JT",SwFileHandlerMethod::USE_CREATE_HOOK },{ L".SMG" , SwFileHandlerMethod::USE_CREATE_HOOK } , {L".PLY",SwFileHandlerMethod::USE_NORMAL },{ L".RFA",SwFileHandlerMethod::USE_PROCESS_HOOK },
/*if an assembly,drawing constains same references and all are in workspace folder, then performing save operation in TC taskpane with assembly being the active file in SW session
  causes all the files to be saved at the same time. In this case the source file will be assembly and target will be drawing */
{L".SLDDRW",SwFileHandlerMethod::USE_COPY_HOOK},{L".DRW",SwFileHandlerMethod::USE_COPY_HOOK},{ L".SWAR",SwFileHandlerMethod::USE_COPY_HOOK } };

const exportFileHandlerMap SwFileTypes::exportFileSLDDRW = {{L".DRW",SwFileHandlerMethod::USE_COPY_HOOK},{L".SLDDRW",SwFileHandlerMethod::USE_COPY_HOOK}/*Drawing, Detached Drawing*/,{L".DRWDOT",SwFileHandlerMethod::USE_COPY_HOOK} /*Drawing Templates*/,
{ L".SLDPRT",SwFileHandlerMethod::USE_COPY_HOOK },{ L".PRT",SwFileHandlerMethod::USE_COPY_HOOK } /*Part Files */,{L".DXF",SwFileHandlerMethod::USE_NORMAL} /*Autocad Drawing Exchange Format*/,{L".DWG",SwFileHandlerMethod::USE_NORMAL}/*AutoCAD .dwg file format*/,
{L".EDRW",SwFileHandlerMethod::USE_NORMAL}/*e Drawings*/,{L".PDF",SwFileHandlerMethod::USE_NORMAL}/*Adobe Portable Document Format*/,
{L".PSD",SwFileHandlerMethod::USE_NORMAL} /*Adobe Photoshop*/,{L".AI",SwFileHandlerMethod::USE_COPY_HOOK} /*Adobe Illustrator*/,
{L".JPG",SwFileHandlerMethod::USE_NORMAL },{L".PNG",SwFileHandlerMethod::USE_NORMAL },{L".TIF",SwFileHandlerMethod::USE_NORMAL },{L".SLDDRT",SwFileHandlerMethod::USE_CREATE_HOOK},{ L".ASM",SwFileHandlerMethod::USE_COPY_HOOK },
{ L".SLDASM",SwFileHandlerMethod::USE_COPY_HOOK },{ L".SWAR",SwFileHandlerMethod::USE_COPY_HOOK } };



bool SwFileTypes::IsSupportedFileFormat(wstring fromFileType,wstring toFileType, SwFileHandlerMethod &handlerType) {
	
	std::transform(fromFileType.begin(), fromFileType.end(), fromFileType.begin(), ::toupper);
	std::transform(toFileType.begin(), toFileType.end(), toFileType.begin(), ::toupper);

	if (SwExtnToFileTypeEnumMap.find(fromFileType) == SwExtnToFileTypeEnumMap.end()) {
		//LOG_DEBUG_FMT("SwFileTypes::IsSupportedFileFormat. FileType(fromFileType) : %s not found in the SwExtnToFileTypeEnumMap", fromFileType.c_str());
		return false;
	}
	SwSourceFileType fileType = SwExtnToFileTypeEnumMap[fromFileType];

	exportFileHandlerMap expFileListMap;
	switch (fileType) {
	case SwSourceFileType::SW_SLDPRT:
	case SwSourceFileType::SW_SLDLFP:
	case SwSourceFileType::SW_PRTDOT:
	case SwSourceFileType::SW_SLDALPRT:
	case SwSourceFileType::SW_PRT:
		expFileListMap = exportFileSLDPRT;
		break;
	case SwSourceFileType::SW_SLDASM:
	case SwSourceFileType::SW_ASMDOT:
	case SwSourceFileType::SW_SLDALASM:
	case SwSourceFileType::SW_ASM:
		expFileListMap = exportFileSLDASM;
		break;
	case SwSourceFileType::SW_SLDDRW:
	case SwSourceFileType::SW_DRWDOT:
	case SwSourceFileType::SW_DRW:
		expFileListMap = exportFileSLDDRW;
		break;
	default:
		LOG_ERROR("Unknown File type: " << fromFileType.c_str());
		return false;
	}
	
	if (expFileListMap.find(toFileType) == expFileListMap.end()) {
		//Check for Creo Part File. Files are ceated as integratedassembly-secret.prt.1
		if (!toFileType.empty() && std::all_of(toFileType.begin()+1, toFileType.end(), ::isdigit)) {
			handlerType = SwFileHandlerMethod::USE_CREATE_HOOK;
			return true;
		}
		LOG_DEBUG_FMT("The exported file format is not supported . To File Type : %s", toFileType.c_str());
		return false;
	}

	handlerType = expFileListMap[toFileType];
	
	return true;

}


bool SwFileTypes::isNativeFileFormat(wstring fileType) {
	std::transform(fileType.begin(), fileType.end(), fileType.begin(), ::toupper);
	return (SwExtnToFileTypeEnumMap.find(fileType) != SwExtnToFileTypeEnumMap.end());
}