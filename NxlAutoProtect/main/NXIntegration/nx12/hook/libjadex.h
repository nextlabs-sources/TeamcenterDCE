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

/* typedef for 36 funcs */
/*
	UGS::Dex::DxfdwgCreator::GetExportFrom
	ApiName:	DxfdwgCreator_GetExportFrom
	FullName:	?GetExportFrom@DxfdwgCreator@Dex@UGS@@QEBA?AW4JA_DXFDWG_CREATOR_ExportFromOption@@XZ
	UnDecorated:	public: enum JA_DXFDWG_CREATOR_ExportFromOption __cdecl UGS::Dex::DxfdwgCreator::GetExportFrom(void)const __ptr64
	Package = UGS::Dex::DxfdwgCreator
	Versions(#Ordinal):
		12.0.2.9 (#327)
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
		12.0.2.9 (#328)
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
		12.0.2.9 (#329)
*/
#define NXTo2dCreator_GetExportFrom_FULLNAME "?GetExportFrom@NXTo2dCreator@Dex@UGS@@QEBA?AW4JA_NXTO2D_CREATOR_ExportFromOption@@XZ"
typedef CPP_ENUM (*pfNXTo2dCreator_GetExportFrom)(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);
extern CPP_ENUM NXTo2dCreator_GetExportFrom(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);

/*
	UGS::Dex::Step203Creator::GetExportFrom
	ApiName:	Step203Creator_GetExportFrom
	FullName:	?GetExportFrom@Step203Creator@Dex@UGS@@QEBA?AW4JA_STEP203_CREATOR_ExportFromOption@@XZ
	UnDecorated:	public: enum JA_STEP203_CREATOR_ExportFromOption __cdecl UGS::Dex::Step203Creator::GetExportFrom(void)const __ptr64
	Package = UGS::Dex::Step203Creator
	Versions(#Ordinal):
		12.0.2.9 (#330)
*/
#define Step203Creator_GetExportFrom_FULLNAME "?GetExportFrom@Step203Creator@Dex@UGS@@QEBA?AW4JA_STEP203_CREATOR_ExportFromOption@@XZ"
typedef CPP_ENUM (*pfStep203Creator_GetExportFrom)(Step203Creator_PTR obj	// class UGS::Dex::Step203Creator * __ptr64
	);
extern CPP_ENUM Step203Creator_GetExportFrom(Step203Creator_PTR obj	// class UGS::Dex::Step203Creator * __ptr64
	);

/*
	UGS::Dex::Step214Creator::GetExportFrom
	ApiName:	Step214Creator_GetExportFrom
	FullName:	?GetExportFrom@Step214Creator@Dex@UGS@@QEBA?AW4JA_STEP214_CREATOR_ExportFromOption@@XZ
	UnDecorated:	public: enum JA_STEP214_CREATOR_ExportFromOption __cdecl UGS::Dex::Step214Creator::GetExportFrom(void)const __ptr64
	Package = UGS::Dex::Step214Creator
	Versions(#Ordinal):
		12.0.2.9 (#331)
*/
#define Step214Creator_GetExportFrom_FULLNAME "?GetExportFrom@Step214Creator@Dex@UGS@@QEBA?AW4JA_STEP214_CREATOR_ExportFromOption@@XZ"
typedef CPP_ENUM (*pfStep214Creator_GetExportFrom)(Step214Creator_PTR obj	// class UGS::Dex::Step214Creator * __ptr64
	);
extern CPP_ENUM Step214Creator_GetExportFrom(Step214Creator_PTR obj	// class UGS::Dex::Step214Creator * __ptr64
	);

/*
	UGS::Dex::StepCreator::GetExportFrom
	ApiName:	StepCreator_GetExportFrom
	FullName:	?GetExportFrom@StepCreator@Dex@UGS@@QEBA?AW4JA_STEP_CREATOR_ExportFromOption@@XZ
	UnDecorated:	public: enum JA_STEP_CREATOR_ExportFromOption __cdecl UGS::Dex::StepCreator::GetExportFrom(void)const __ptr64
	Package = UGS::Dex::StepCreator
	Versions(#Ordinal):
		12.0.2.9 (#332)
*/
#define StepCreator_GetExportFrom_FULLNAME "?GetExportFrom@StepCreator@Dex@UGS@@QEBA?AW4JA_STEP_CREATOR_ExportFromOption@@XZ"
typedef CPP_ENUM (*pfStepCreator_GetExportFrom)(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);
extern CPP_ENUM StepCreator_GetExportFrom(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);

/*
	UGS::Dex::NXTo2dCreator::GetInputFile
	ApiName:	NXTo2dCreator_GetInputFile
	FullName:	?GetInputFile@NXTo2dCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::NXTo2dCreator::GetInputFile(void)const __ptr64
	Package = UGS::Dex::NXTo2dCreator
	Versions(#Ordinal):
		12.0.2.9 (#409)
*/
#define NXTo2dCreator_GetInputFile_FULLNAME "?GetInputFile@NXTo2dCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfNXTo2dCreator_GetInputFile)(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);
extern char const * NXTo2dCreator_GetInputFile(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);

/*
	UGS::Dex::DxfdwgCreator::GetInputFileName
	ApiName:	DxfdwgCreator_GetInputFileName
	FullName:	?GetInputFileName@DxfdwgCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::DxfdwgCreator::GetInputFileName(void)const __ptr64
	Package = UGS::Dex::DxfdwgCreator
	Versions(#Ordinal):
		12.0.2.9 (#415)
*/
#define DxfdwgCreator_GetInputFileName_FULLNAME "?GetInputFileName@DxfdwgCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfDxfdwgCreator_GetInputFileName)(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);
extern char const * DxfdwgCreator_GetInputFileName(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);

/*
	UGS::Dex::IgesCreator::GetInputFileName
	ApiName:	IgesCreator_GetInputFileName
	FullName:	?GetInputFileName@IgesCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::IgesCreator::GetInputFileName(void)const __ptr64
	Package = UGS::Dex::IgesCreator
	Versions(#Ordinal):
		12.0.2.9 (#416)
*/
#define IgesCreator_GetInputFileName_FULLNAME "?GetInputFileName@IgesCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfIgesCreator_GetInputFileName)(IgesCreator_PTR obj	// class UGS::Dex::IgesCreator * __ptr64
	);
extern char const * IgesCreator_GetInputFileName(IgesCreator_PTR obj	// class UGS::Dex::IgesCreator * __ptr64
	);

/*
	UGS::Dex::Step203Creator::GetInputFileName
	ApiName:	Step203Creator_GetInputFileName
	FullName:	?GetInputFileName@Step203Creator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::Step203Creator::GetInputFileName(void)const __ptr64
	Package = UGS::Dex::Step203Creator
	Versions(#Ordinal):
		12.0.2.9 (#417)
*/
#define Step203Creator_GetInputFileName_FULLNAME "?GetInputFileName@Step203Creator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfStep203Creator_GetInputFileName)(Step203Creator_PTR obj	// class UGS::Dex::Step203Creator * __ptr64
	);
extern char const * Step203Creator_GetInputFileName(Step203Creator_PTR obj	// class UGS::Dex::Step203Creator * __ptr64
	);

/*
	UGS::Dex::Step214Creator::GetInputFileName
	ApiName:	Step214Creator_GetInputFileName
	FullName:	?GetInputFileName@Step214Creator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::Step214Creator::GetInputFileName(void)const __ptr64
	Package = UGS::Dex::Step214Creator
	Versions(#Ordinal):
		12.0.2.9 (#418)
*/
#define Step214Creator_GetInputFileName_FULLNAME "?GetInputFileName@Step214Creator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfStep214Creator_GetInputFileName)(Step214Creator_PTR obj	// class UGS::Dex::Step214Creator * __ptr64
	);
extern char const * Step214Creator_GetInputFileName(Step214Creator_PTR obj	// class UGS::Dex::Step214Creator * __ptr64
	);

/*
	UGS::Dex::StepCreator::GetInputFileName
	ApiName:	StepCreator_GetInputFileName
	FullName:	?GetInputFileName@StepCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::StepCreator::GetInputFileName(void)const __ptr64
	Package = UGS::Dex::StepCreator
	Versions(#Ordinal):
		12.0.2.9 (#419)
*/
#define StepCreator_GetInputFileName_FULLNAME "?GetInputFileName@StepCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfStepCreator_GetInputFileName)(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);
extern char const * StepCreator_GetInputFileName(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);

/*
	UGS::Dex::STLCreator::GetObjectList
	ApiName:	STLCreator_GetObjectList
	FullName:	?GetObjectList@STLCreator@Dex@UGS@@QEBAPEAVSELECT_OBJECT_list@3@XZ
	UnDecorated:	public: class UGS::SELECT_OBJECT_list * __ptr64 __cdecl UGS::Dex::STLCreator::GetObjectList(void)const __ptr64
	Package = UGS::Dex::STLCreator
	Versions(#Ordinal):
		12.0.2.9 (#452)
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
		12.0.2.9 (#456)
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
		12.0.2.9 (#457)
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
		12.0.2.9 (#458)
*/
#define NXTo2dCreator_GetObjectSelector_FULLNAME "?GetObjectSelector@NXTo2dCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ"
typedef ObjectSelector_PTR (*pfNXTo2dCreator_GetObjectSelector)(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);
extern ObjectSelector_PTR NXTo2dCreator_GetObjectSelector(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);

/*
	UGS::Dex::Step203Creator::GetObjectSelector
	ApiName:	Step203Creator_GetObjectSelector
	FullName:	?GetObjectSelector@Step203Creator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ
	UnDecorated:	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::Step203Creator::GetObjectSelector(void)const __ptr64
	Package = UGS::Dex::Step203Creator
	Versions(#Ordinal):
		12.0.2.9 (#459)
*/
#define Step203Creator_GetObjectSelector_FULLNAME "?GetObjectSelector@Step203Creator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ"
typedef ObjectSelector_PTR (*pfStep203Creator_GetObjectSelector)(Step203Creator_PTR obj	// class UGS::Dex::Step203Creator * __ptr64
	);
extern ObjectSelector_PTR Step203Creator_GetObjectSelector(Step203Creator_PTR obj	// class UGS::Dex::Step203Creator * __ptr64
	);

/*
	UGS::Dex::Step214Creator::GetObjectSelector
	ApiName:	Step214Creator_GetObjectSelector
	FullName:	?GetObjectSelector@Step214Creator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ
	UnDecorated:	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::Step214Creator::GetObjectSelector(void)const __ptr64
	Package = UGS::Dex::Step214Creator
	Versions(#Ordinal):
		12.0.2.9 (#460)
*/
#define Step214Creator_GetObjectSelector_FULLNAME "?GetObjectSelector@Step214Creator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ"
typedef ObjectSelector_PTR (*pfStep214Creator_GetObjectSelector)(Step214Creator_PTR obj	// class UGS::Dex::Step214Creator * __ptr64
	);
extern ObjectSelector_PTR Step214Creator_GetObjectSelector(Step214Creator_PTR obj	// class UGS::Dex::Step214Creator * __ptr64
	);

/*
	UGS::Dex::StepCreator::GetObjectSelector
	ApiName:	StepCreator_GetObjectSelector
	FullName:	?GetObjectSelector@StepCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ
	UnDecorated:	public: class UGS::Dex::ObjectSelector * __ptr64 __cdecl UGS::Dex::StepCreator::GetObjectSelector(void)const __ptr64
	Package = UGS::Dex::StepCreator
	Versions(#Ordinal):
		12.0.2.9 (#461)
*/
#define StepCreator_GetObjectSelector_FULLNAME "?GetObjectSelector@StepCreator@Dex@UGS@@QEBAPEAVObjectSelector@23@XZ"
typedef ObjectSelector_PTR (*pfStepCreator_GetObjectSelector)(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);
extern ObjectSelector_PTR StepCreator_GetObjectSelector(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);

/*
	UGS::Dex::DxfdwgCreator::GetOutputFile
	ApiName:	DxfdwgCreator_GetOutputFile
	FullName:	?GetOutputFile@DxfdwgCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::DxfdwgCreator::GetOutputFile(void)const __ptr64
	Package = UGS::Dex::DxfdwgCreator
	Versions(#Ordinal):
		12.0.2.9 (#487)
*/
#define DxfdwgCreator_GetOutputFile_FULLNAME "?GetOutputFile@DxfdwgCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfDxfdwgCreator_GetOutputFile)(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);
extern char const * DxfdwgCreator_GetOutputFile(DxfdwgCreator_PTR obj	// class UGS::Dex::DxfdwgCreator * __ptr64
	);

/*
	UGS::Dex::IgesCreator::GetOutputFile
	ApiName:	IgesCreator_GetOutputFile
	FullName:	?GetOutputFile@IgesCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::IgesCreator::GetOutputFile(void)const __ptr64
	Package = UGS::Dex::IgesCreator
	Versions(#Ordinal):
		12.0.2.9 (#489)
*/
#define IgesCreator_GetOutputFile_FULLNAME "?GetOutputFile@IgesCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfIgesCreator_GetOutputFile)(IgesCreator_PTR obj	// class UGS::Dex::IgesCreator * __ptr64
	);
extern char const * IgesCreator_GetOutputFile(IgesCreator_PTR obj	// class UGS::Dex::IgesCreator * __ptr64
	);

/*
	UGS::Dex::NXTo2dCreator::GetOutputFile
	ApiName:	NXTo2dCreator_GetOutputFile
	FullName:	?GetOutputFile@NXTo2dCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::NXTo2dCreator::GetOutputFile(void)const __ptr64
	Package = UGS::Dex::NXTo2dCreator
	Versions(#Ordinal):
		12.0.2.9 (#491)
*/
#define NXTo2dCreator_GetOutputFile_FULLNAME "?GetOutputFile@NXTo2dCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfNXTo2dCreator_GetOutputFile)(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);
extern char const * NXTo2dCreator_GetOutputFile(NXTo2dCreator_PTR obj	// class UGS::Dex::NXTo2dCreator * __ptr64
	);

/*
	UGS::Dex::STLCreator::GetOutputFile
	ApiName:	STLCreator_GetOutputFile
	FullName:	?GetOutputFile@STLCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::STLCreator::GetOutputFile(void)const __ptr64
	Package = UGS::Dex::STLCreator
	Versions(#Ordinal):
		12.0.2.9 (#493)
*/
#define STLCreator_GetOutputFile_FULLNAME "?GetOutputFile@STLCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfSTLCreator_GetOutputFile)(CPP_PTR obj	// class UGS::Dex::STLCreator * __ptr64
	);
extern char const * STLCreator_GetOutputFile(CPP_PTR obj	// class UGS::Dex::STLCreator * __ptr64
	);

/*
	UGS::Dex::Step203Creator::GetOutputFile
	ApiName:	Step203Creator_GetOutputFile
	FullName:	?GetOutputFile@Step203Creator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::Step203Creator::GetOutputFile(void)const __ptr64
	Package = UGS::Dex::Step203Creator
	Versions(#Ordinal):
		12.0.2.9 (#494)
*/
#define Step203Creator_GetOutputFile_FULLNAME "?GetOutputFile@Step203Creator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfStep203Creator_GetOutputFile)(Step203Creator_PTR obj	// class UGS::Dex::Step203Creator * __ptr64
	);
extern char const * Step203Creator_GetOutputFile(Step203Creator_PTR obj	// class UGS::Dex::Step203Creator * __ptr64
	);

/*
	UGS::Dex::Step214Creator::GetOutputFile
	ApiName:	Step214Creator_GetOutputFile
	FullName:	?GetOutputFile@Step214Creator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::Step214Creator::GetOutputFile(void)const __ptr64
	Package = UGS::Dex::Step214Creator
	Versions(#Ordinal):
		12.0.2.9 (#496)
*/
#define Step214Creator_GetOutputFile_FULLNAME "?GetOutputFile@Step214Creator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfStep214Creator_GetOutputFile)(Step214Creator_PTR obj	// class UGS::Dex::Step214Creator * __ptr64
	);
extern char const * Step214Creator_GetOutputFile(Step214Creator_PTR obj	// class UGS::Dex::Step214Creator * __ptr64
	);

/*
	UGS::Dex::StepCreator::GetOutputFile
	ApiName:	StepCreator_GetOutputFile
	FullName:	?GetOutputFile@StepCreator@Dex@UGS@@QEBAPEBDXZ
	UnDecorated:	public: char const * __ptr64 __cdecl UGS::Dex::StepCreator::GetOutputFile(void)const __ptr64
	Package = UGS::Dex::StepCreator
	Versions(#Ordinal):
		12.0.2.9 (#499)
*/
#define StepCreator_GetOutputFile_FULLNAME "?GetOutputFile@StepCreator@Dex@UGS@@QEBAPEBDXZ"
typedef char const * (*pfStepCreator_GetOutputFile)(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);
extern char const * StepCreator_GetOutputFile(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);

/*
	UGS::Dex::DxfdwgCreator::GetOutputFileType
	ApiName:	DxfdwgCreator_GetOutputFileType
	FullName:	?GetOutputFileType@DxfdwgCreator@Dex@UGS@@QEBA?AW4JA_DXFDWG_CREATOR_OutputFileTypeOption@@XZ
	UnDecorated:	public: enum JA_DXFDWG_CREATOR_OutputFileTypeOption __cdecl UGS::Dex::DxfdwgCreator::GetOutputFileType(void)const __ptr64
	Package = UGS::Dex::DxfdwgCreator
	Versions(#Ordinal):
		12.0.2.9 (#500)
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
		12.0.2.9 (#501)
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
		12.0.2.9 (#522)
*/
#define ObjectSelector_GetSelectionComp_FULLNAME "?GetSelectionComp@ObjectSelector@Dex@UGS@@QEBAPEAVSELECT_OBJECT_list@3@XZ"
typedef SELECT_OBJECT_list_PTR (*pfObjectSelector_GetSelectionComp)(ObjectSelector_PTR obj	// class UGS::Dex::ObjectSelector * __ptr64
	);
extern SELECT_OBJECT_list_PTR ObjectSelector_GetSelectionComp(ObjectSelector_PTR obj	// class UGS::Dex::ObjectSelector * __ptr64
	);

/*
	UGS::Dex::DxfdwgCreator::commit
	ApiName:	DxfdwgCreator_commit
	FullName:	?commit@DxfdwgCreator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::DxfdwgCreator::commit(void) __ptr64
	Package = UGS::Dex::DxfdwgCreator
	Versions(#Ordinal):
		12.0.2.9 (#1077)
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
		12.0.2.9 (#1079)
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
		12.0.2.9 (#1082)
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
		12.0.2.9 (#1086)
*/
#define STLCreator_commit_FULLNAME "?commit@STLCreator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfSTLCreator_commit)(CPP_PTR obj	// class UGS::Dex::STLCreator * __ptr64
	);

/*
	UGS::Dex::Step203Creator::commit
	ApiName:	Step203Creator_commit
	FullName:	?commit@Step203Creator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::Step203Creator::commit(void) __ptr64
	Package = UGS::Dex::Step203Creator
	Versions(#Ordinal):
		12.0.2.9 (#1087)
*/
#define Step203Creator_commit_FULLNAME "?commit@Step203Creator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfStep203Creator_commit)(Step203Creator_PTR obj	// class UGS::Dex::Step203Creator * __ptr64
	);

/*
	UGS::Dex::Step214Creator::commit
	ApiName:	Step214Creator_commit
	FullName:	?commit@Step214Creator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::Step214Creator::commit(void) __ptr64
	Package = UGS::Dex::Step214Creator
	Versions(#Ordinal):
		12.0.2.9 (#1089)
*/
#define Step214Creator_commit_FULLNAME "?commit@Step214Creator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfStep214Creator_commit)(Step214Creator_PTR obj	// class UGS::Dex::Step214Creator * __ptr64
	);

/*
	UGS::Dex::StepCreator::commit
	ApiName:	StepCreator_commit
	FullName:	?commit@StepCreator@Dex@UGS@@UEAAIXZ
	UnDecorated:	public: virtual unsigned int __cdecl UGS::Dex::StepCreator::commit(void) __ptr64
	Package = UGS::Dex::StepCreator
	Versions(#Ordinal):
		12.0.2.9 (#1092)
*/
#define StepCreator_commit_FULLNAME "?commit@StepCreator@Dex@UGS@@UEAAIXZ"
typedef unsigned int (*pfStepCreator_commit)(StepCreator_PTR obj	// class UGS::Dex::StepCreator * __ptr64
	);


#endif
