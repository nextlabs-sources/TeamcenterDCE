/*
	Library:	libjadex.dll
*/
#ifndef NXL_LIBJADEX_DLL_H_INCLUDED
#define NXL_LIBJADEX_DLL_H_INCLUDED

#include "libpartdisp.h"

typedef const void *ObjectSelector_PTR;	// class UGS::Dex::ObjectSelector * __ptr64
typedef const void *IgesCreator_PTR;	// class UGS::Dex::IgesCreator * __ptr64
typedef const void *StepCreator_PTR;	// class UGS::Dex::StepCreator * __ptr64
typedef const void *Step203Creator_PTR;	// class UGS::Dex::Step203Creator * __ptr64
typedef const void *Step214Creator_PTR;	// class UGS::Dex::Step214Creator * __ptr64
typedef const void *DxfdwgCreator_PTR;	// class UGS::Dex::DxfdwgCreator * __ptr64
typedef const void *NXTo2dCreator_PTR;	// class UGS::Dex::NXTo2dCreator * __ptr64
typedef const void *Catiav4Creator_PTR;	// class UGS::Dex::Catiav4Creator * __ptr64
typedef const void *Catiav5Creator_PTR;	// class UGS::Dex::Catiav5Creator * __ptr64

/* typedef for 23 funcs */
/*
	UGS::Dex::DxfdwgCreator::GetExportFrom
	ApiName:	DxfdwgCreator_GetExportFrom
	FullName:	?GetExportFrom@DxfdwgCreator@Dex@UGS@@QEBA?AW4JA_DXFDWG_CREATOR_ExportFromOption@@XZ
	UnDecorated:	public: enum JA_DXFDWG_CREATOR_ExportFromOption __cdecl UGS::Dex::DxfdwgCreator::GetExportFrom(void)const __ptr64
	Package = UGS::Dex::DxfdwgCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#469)
*/
#define DxfdwgCreator_GetExportFrom_FULLNAME "?GetExportFrom@DxfdwgCreator@Dex@UGS@@QEBA?AW4JA_DXFDWG_CREATOR_ExportFromOption@@XZ"
typedef CPP_ENUM (*pfDxfdwgCreator_GetExportFrom)(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);
extern CPP_ENUM DxfdwgCreator_GetExportFrom(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);

/*
	UGS::Dex::IgesCreator::GetExportFrom
	ApiName:	IgesCreator_GetExportFrom
	FullName:	?GetExportFrom@IgesCreator@Dex@UGS@@QEBA?AW4JA_IGES_CREATOR_export_from_option@@XZ
	UnDecorated:	public: enum JA_IGES_CREATOR_export_from_option __cdecl UGS::Dex::IgesCreator::GetExportFrom(void)const __ptr64
	Package = UGS::Dex::IgesCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#471)
*/
#define IgesCreator_GetExportFrom_FULLNAME "?GetExportFrom@IgesCreator@Dex@UGS@@QEBA?AW4JA_IGES_CREATOR_export_from_option@@XZ"
typedef CPP_ENUM (*pfIgesCreator_GetExportFrom)(IgesCreator_PTR obj	// class UGS::Dex::IgesCreator * __ptr64
	);
extern CPP_ENUM IgesCreator_GetExportFrom(IgesCreator_PTR obj	// class UGS::Dex::IgesCreator * __ptr64
	);

/*
	UGS::Dex::NXTo2dCreator::GetExportFrom
	ApiName:	NXTo2dCreator_GetExportFrom
	FullName:	?GetExportFrom@NXTo2dCreator@Dex@UGS@@QEBA?AW4JA_NXTO2D_CREATOR_ExportFromOption@@XZ
	UnDecorated:	public: enum JA_NXTO2D_CREATOR_ExportFromOption __cdecl UGS::Dex::NXTo2dCreator::GetExportFrom(void)const __ptr64
	Package = UGS::Dex::NXTo2dCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#472)
*/
#define NXTo2dCreator_GetExportFrom_FULLNAME "?GetExportFrom@NXTo2dCreator@Dex@UGS@@QEBA?AW4JA_NXTO2D_CREATOR_ExportFromOption@@XZ"
typedef CPP_ENUM (*pfNXTo2dCreator_GetExportFrom)(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);
extern CPP_ENUM NXTo2dCreator_GetExportFrom(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);

/*
	UGS::Dex::StepCreator::GetExportFrom
	ApiName:	StepCreator_GetExportFrom
	FullName:	?GetExportFrom@StepCreator@Dex@UGS@@QEBA?AW4JA_STEP_CREATOR_ExportFromOption@@XZ
	UnDecorated:	public: enum JA_STEP_CREATOR_ExportFromOption __cdecl UGS::Dex::StepCreator::GetExportFrom(void)const __ptr64
	Package = UGS::Dex::StepCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#473)
*/
#define StepCreator_GetExportFrom_FULLNAME "?GetExportFrom@StepCreator@Dex@UGS@@QEBA?AW4JA_STEP_CREATOR_ExportFromOption@@XZ"
typedef CPP_ENUM (*pfStepCreator_GetExportFrom)(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);
extern CPP_ENUM StepCreator_GetExportFrom(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);

/*
	UGS::Dex::WavefrontObjCreator::GetExportFrom
	ApiName:	WavefrontObjCreator_GetExportFrom
	FullName:	?GetExportFrom@WavefrontObjCreator@Dex@UGS@@QEBA?AW4JA_WAVEFRONT_OBJ_CREATOR_ExportFromOption@@XZ
	UnDecorated:	public: enum JA_WAVEFRONT_OBJ_CREATOR_ExportFromOption __cdecl UGS::Dex::WavefrontObjCreator::GetExportFrom(void)const __ptr64
	Package = UGS::Dex::WavefrontObjCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#474)
*/
#define WavefrontObjCreator_GetExportFrom_FULLNAME "?GetExportFrom@WavefrontObjCreator@Dex@UGS@@QEBA?AW4JA_WAVEFRONT_OBJ_CREATOR_ExportFromOption@@XZ"
typedef CPP_ENUM (*pfWavefrontObjCreator_GetExportFrom)(CPP_PTR obj	// class UGS::Dex::WavefrontObjCreator * __ptr64
	);
extern CPP_ENUM WavefrontObjCreator_GetExportFrom(CPP_PTR obj	// class UGS::Dex::WavefrontObjCreator * __ptr64
	);

/*
	UGS::Dex::BaseCreator::GetInputFileName
	ApiName:	BaseCreator_GetInputFileName
	FullName:	?GetInputFileName@BaseCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::BaseCreator::GetInputFileName(void)const __ptr64
	Package = UGS::Dex::BaseCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#572)
*/
#define BaseCreator_GetInputFileName_FULLNAME "?GetInputFileName@BaseCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfBaseCreator_GetInputFileName)(CPP_PTR obj	// class UGS::Dex::BaseCreator * __ptr64
	);
extern char const * BaseCreator_GetInputFileName(CPP_PTR obj	// class UGS::Dex::BaseCreator * __ptr64
	);

/*
	UGS::Dex::STLCreator::GetObjectList
	ApiName:	STLCreator_GetObjectList
	FullName:	?GetObjectList@STLCreator@Dex@UGS@@QEBAPEAVSELECT_OBJECT_list@3@XZ
	UnDecorated:	public: class UGS::SELECT_OBJECT_list * __ptr64 __cdecl UGS::Dex::STLCreator::GetObjectList(void)const __ptr64
	Package = UGS::Dex::STLCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#632)
*/
#define STLCreator_GetObjectList_FULLNAME "?GetObjectList@STLCreator@Dex@UGS@@QEBAPEAVSELECT_OBJECT_list@3@XZ"
typedef SELECT_OBJECT_list_PTR (*pfSTLCreator_GetObjectList)(CPP_PTR obj	// class UGS::Dex::STLCreator * __ptr64
	);
extern SELECT_OBJECT_list_PTR STLCreator_GetObjectList(CPP_PTR obj	// class UGS::Dex::STLCreator * __ptr64
	);

/*
	UGS::Dex::DxfdwgCreator::GetObjectSelector
	ApiName:	DxfdwgCreator_GetObjectSelector
	FullName:	?GetObjectSelector@DxfdwgCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ
	UnDecorated:	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::DxfdwgCreator::GetObjectSelector(void)const __ptr64
	Package = UGS::Dex::DxfdwgCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#636)
*/
#define DxfdwgCreator_GetObjectSelector_FULLNAME "?GetObjectSelector@DxfdwgCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ"
typedef ObjectSelector_PTR (*pfDxfdwgCreator_GetObjectSelector)(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);
extern ObjectSelector_PTR DxfdwgCreator_GetObjectSelector(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);

/*
	UGS::Dex::IgesCreator::GetObjectSelector
	ApiName:	IgesCreator_GetObjectSelector
	FullName:	?GetObjectSelector@IgesCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ
	UnDecorated:	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::IgesCreator::GetObjectSelector(void)const __ptr64
	Package = UGS::Dex::IgesCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#638)
*/
#define IgesCreator_GetObjectSelector_FULLNAME "?GetObjectSelector@IgesCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ"
typedef ObjectSelector_PTR (*pfIgesCreator_GetObjectSelector)(IgesCreator_PTR obj	// class UGS::Dex::IgesCreator * __ptr64
	);
extern ObjectSelector_PTR IgesCreator_GetObjectSelector(IgesCreator_PTR obj	// class UGS::Dex::IgesCreator * __ptr64
	);

/*
	UGS::Dex::NXTo2dCreator::GetObjectSelector
	ApiName:	NXTo2dCreator_GetObjectSelector
	FullName:	?GetObjectSelector@NXTo2dCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ
	UnDecorated:	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::NXTo2dCreator::GetObjectSelector(void)const __ptr64
	Package = UGS::Dex::NXTo2dCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#639)
*/
#define NXTo2dCreator_GetObjectSelector_FULLNAME "?GetObjectSelector@NXTo2dCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ"
typedef ObjectSelector_PTR (*pfNXTo2dCreator_GetObjectSelector)(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);
extern ObjectSelector_PTR NXTo2dCreator_GetObjectSelector(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);

/*
	UGS::Dex::StepCreator::GetObjectSelector
	ApiName:	StepCreator_GetObjectSelector
	FullName:	?GetObjectSelector@StepCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ
	UnDecorated:	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::StepCreator::GetObjectSelector(void)const __ptr64
	Package = UGS::Dex::StepCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#640)
*/
#define StepCreator_GetObjectSelector_FULLNAME "?GetObjectSelector@StepCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ"
typedef ObjectSelector_PTR (*pfStepCreator_GetObjectSelector)(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);
extern ObjectSelector_PTR StepCreator_GetObjectSelector(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);

/*
	UGS::Dex::WavefrontObjCreator::GetObjectSelector
	ApiName:	WavefrontObjCreator_GetObjectSelector
	FullName:	?GetObjectSelector@WavefrontObjCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ
	UnDecorated:	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::WavefrontObjCreator::GetObjectSelector(void)const __ptr64
	Package = UGS::Dex::WavefrontObjCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#641)
*/
#define WavefrontObjCreator_GetObjectSelector_FULLNAME "?GetObjectSelector@WavefrontObjCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ"
typedef ObjectSelector_PTR (*pfWavefrontObjCreator_GetObjectSelector)(CPP_PTR obj	// class UGS::Dex::WavefrontObjCreator * __ptr64
	);
extern ObjectSelector_PTR WavefrontObjCreator_GetObjectSelector(CPP_PTR obj	// class UGS::Dex::WavefrontObjCreator * __ptr64
	);

/*
	UGS::Dex::BaseCreator::GetOutputFile
	ApiName:	BaseCreator_GetOutputFile
	FullName:	?GetOutputFile@BaseCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::BaseCreator::GetOutputFile(void)const __ptr64
	Package = UGS::Dex::BaseCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#664)
*/
#define BaseCreator_GetOutputFile_FULLNAME "?GetOutputFile@BaseCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfBaseCreator_GetOutputFile)(CPP_PTR obj	// class UGS::Dex::BaseCreator * __ptr64
	);
extern char const * BaseCreator_GetOutputFile(CPP_PTR obj	// class UGS::Dex::BaseCreator * __ptr64
	);

/*
	UGS::Dex::DxfdwgCreator::GetOutputFileType
	ApiName:	DxfdwgCreator_GetOutputFileType
	FullName:	?GetOutputFileType@DxfdwgCreator@Dex@UGS@@QEBA?AW4JA_DXFDWG_CREATOR_OutputFileTypeOption@@XZ
	UnDecorated:	public: enum JA_DXFDWG_CREATOR_OutputFileTypeOption __cdecl UGS::Dex::DxfdwgCreator::GetOutputFileType(void)const __ptr64
	Package = UGS::Dex::DxfdwgCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#669)
*/
#define DxfdwgCreator_GetOutputFileType_FULLNAME "?GetOutputFileType@DxfdwgCreator@Dex@UGS@@QEBA?AW4JA_DXFDWG_CREATOR_OutputFileTypeOption@@XZ"
typedef CPP_ENUM (*pfDxfdwgCreator_GetOutputFileType)(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);
extern CPP_ENUM DxfdwgCreator_GetOutputFileType(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);

/*
	UGS::Dex::NXTo2dCreator::GetOutputFileType
	ApiName:	NXTo2dCreator_GetOutputFileType
	FullName:	?GetOutputFileType@NXTo2dCreator@Dex@UGS@@QEBA?AW4JA_NXTO2D_CREATOR_OutputAsOption@@XZ
	UnDecorated:	public: enum JA_NXTO2D_CREATOR_OutputAsOption __cdecl UGS::Dex::NXTo2dCreator::GetOutputFileType(void)const __ptr64
	Package = UGS::Dex::NXTo2dCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#670)
*/
#define NXTo2dCreator_GetOutputFileType_FULLNAME "?GetOutputFileType@NXTo2dCreator@Dex@UGS@@QEBA?AW4JA_NXTO2D_CREATOR_OutputAsOption@@XZ"
typedef CPP_ENUM (*pfNXTo2dCreator_GetOutputFileType)(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);
extern CPP_ENUM NXTo2dCreator_GetOutputFileType(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);

/*
	UGS::Dex::ObjectSelector::GetSelectionComp
	ApiName:	ObjectSelector_GetSelectionComp
	FullName:	?GetSelectionComp@ObjectSelector@Dex@UGS@@QEBAPEAVSELECT_OBJECT_list@3@XZ
	UnDecorated:	public: class UGS::SELECT_OBJECT_list * __ptr64 __cdecl UGS::Dex::ObjectSelector::GetSelectionComp(void)const __ptr64
	Package = UGS::Dex::ObjectSelector
	Versions(#Ordinal):
		1980.1700.0.0 (#701)
*/
#define ObjectSelector_GetSelectionComp_FULLNAME "?GetSelectionComp@ObjectSelector@Dex@UGS@@QEBAPEAVSELECT_OBJECT_list@3@XZ"
typedef SELECT_OBJECT_list_PTR (*pfObjectSelector_GetSelectionComp)(ObjectSelector_PTR obj	// class UGS::Dex::ObjectSelector * __ptr64
	);
extern SELECT_OBJECT_list_PTR ObjectSelector_GetSelectionComp(ObjectSelector_PTR obj	// class UGS::Dex::ObjectSelector * __ptr64
	);

/*
	UGS::Dex::BaseCreator::UploadFileToTeamcenter
	ApiName:	BaseCreator_UploadFileToTeamcenter
	FullName:	?UploadFileToTeamcenter@BaseCreator@Dex@UGS@@IEAAXXZ
	UnDecorated:	protected: void __cdecl UGS::Dex::BaseCreator::UploadFileToTeamcenter(void) __ptr64
	Package = UGS::Dex::BaseCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#1394)
*/
#define BaseCreator_UploadFileToTeamcenter_FULLNAME "?UploadFileToTeamcenter@BaseCreator@Dex@UGS@@IEAAXXZ"
typedef void (*pfBaseCreator_UploadFileToTeamcenter)(CPP_PTR obj	// class UGS::Dex::BaseCreator * __ptr64
	);

/*
	UGS::Dex::DxfdwgCreator::commit
	ApiName:	DxfdwgCreator_commit
	FullName:	?commit@DxfdwgCreator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::DxfdwgCreator::commit(void) __ptr64
	Package = UGS::Dex::DxfdwgCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#1450)
*/
#define DxfdwgCreator_commit_FULLNAME "?commit@DxfdwgCreator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfDxfdwgCreator_commit)(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);

/*
	UGS::Dex::IgesCreator::commit
	ApiName:	IgesCreator_commit
	FullName:	?commit@IgesCreator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::IgesCreator::commit(void) __ptr64
	Package = UGS::Dex::IgesCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#1454)
*/
#define IgesCreator_commit_FULLNAME "?commit@IgesCreator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfIgesCreator_commit)(IgesCreator_PTR obj	// class UGS::Dex::IgesCreator * __ptr64
	);

/*
	UGS::Dex::NXTo2dCreator::commit
	ApiName:	NXTo2dCreator_commit
	FullName:	?commit@NXTo2dCreator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::NXTo2dCreator::commit(void) __ptr64
	Package = UGS::Dex::NXTo2dCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#1458)
*/
#define NXTo2dCreator_commit_FULLNAME "?commit@NXTo2dCreator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfNXTo2dCreator_commit)(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);

/*
	UGS::Dex::STLCreator::commit
	ApiName:	STLCreator_commit
	FullName:	?commit@STLCreator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::STLCreator::commit(void) __ptr64
	Package = UGS::Dex::STLCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#1463)
*/
#define STLCreator_commit_FULLNAME "?commit@STLCreator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfSTLCreator_commit)(CPP_PTR obj	// class UGS::Dex::STLCreator * __ptr64
	);

/*
	UGS::Dex::StepCreator::commit
	ApiName:	StepCreator_commit
	FullName:	?commit@StepCreator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::StepCreator::commit(void) __ptr64
	Package = UGS::Dex::StepCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#1466)
*/
#define StepCreator_commit_FULLNAME "?commit@StepCreator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfStepCreator_commit)(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);

/*
	UGS::Dex::WavefrontObjCreator::commit
	ApiName:	WavefrontObjCreator_commit
	FullName:	?commit@WavefrontObjCreator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::WavefrontObjCreator::commit(void) __ptr64
	Package = UGS::Dex::WavefrontObjCreator
	Versions(#Ordinal):
		1980.1700.0.0 (#1469)
*/
#define WavefrontObjCreator_commit_FULLNAME "?commit@WavefrontObjCreator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfWavefrontObjCreator_commit)(CPP_PTR obj	// class UGS::Dex::WavefrontObjCreator * __ptr64
	);


#endif
