@echo OFF

if "%~1"=="" (
	goto USAGE
)
set nxRoot=%1
echo NX_ROOT_DIR=%nxRoot%
if not exist %nxRoot% (
	echo ERROR: %nxRoot% not exist
	goto END
)
if "%~2"=="" (
	goto USAGE
)
set exportRoot=%2
echo targetDir=%exportRoot%


echo Copying dlls from NXBIN
if not exist %exportRoot% ( mkdir %exportRoot% )

copy %nxRoot%\NXBIN\libassy.dll %exportRoot%\
copy %nxRoot%\NXBIN\libdraw.dll %exportRoot%\
copy %nxRoot%\NXBIN\libdisp.dll %exportRoot%\
copy %nxRoot%\NXBIN\libhidl.dll %exportRoot%\
copy %nxRoot%\NXBIN\libjadex.dll %exportRoot%\
copy %nxRoot%\NXBIN\libjafreeformsurfaces.dll %exportRoot%\
copy %nxRoot%\NXBIN\liblwks.dll %exportRoot%\
copy %nxRoot%\NXBIN\libpart.dll %exportRoot%\
copy %nxRoot%\NXBIN\libpartdisp.dll %exportRoot%\
copy %nxRoot%\NXBIN\libpartutils.dll %exportRoot%\
copy %nxRoot%\NXBIN\libpdmservices.dll %exportRoot%\
copy %nxRoot%\NXBIN\libsyss.dll %exportRoot%\
copy %nxRoot%\NXBIN\libugmr.dll %exportRoot%\
copy %nxRoot%\NXBIN\libugui.dll %exportRoot%\
copy %nxRoot%\NXBIN\libugutilsint.dll %exportRoot%\

goto END

:USAGE
echo "USAGE: %0 <NX_ROOT_DIR> <targetDir>"

:END
