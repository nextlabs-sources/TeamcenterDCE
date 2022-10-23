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


echo Copying h files in ugopen\
if not exist %exportRoot%\ugopen ( mkdir %exportRoot%\ugopen )
copy %nxRoot%\ugopen\*.h %exportRoot%\ugopen\

echo Copying lib files in ugopen\
copy %nxRoot%\ugopen\libnxopencpp.lib %exportRoot%\ugopen\
copy %nxRoot%\ugopen\libnxopencpp_pdm.lib %exportRoot%\ugopen\
copy %nxRoot%\ugopen\libufun.lib %exportRoot%\ugopen\
copy %nxRoot%\ugopen\libufun_cae.lib %exportRoot%\ugopen\
copy %nxRoot%\ugopen\libugopenint.lib %exportRoot%\ugopen\
copy %nxRoot%\ugopen\libugopenint_cae.lib %exportRoot%\ugopen\


echo Copying hxx files in ugopen\NXOpen
if not exist %exportRoot%\ugopen\NXOpen ( mkdir %exportRoot%\ugopen\NXOpen )
copy %nxRoot%\ugopen\NXOpen\*.hxx %exportRoot%\ugopen\NXOpen


echo Copying signcpp.exe files in NXBIN\
if not exist %exportRoot%\NXBIN ( mkdir %exportRoot%\NXBIN )
copy %nxRoot%\NXBIN\signcpp.exe %exportRoot%\NXBIN\
copy %nxRoot%\NXBIN\libjam.dll %exportRoot%\NXBIN\
copy %nxRoot%\NXBIN\libjson.dll %exportRoot%\NXBIN\
copy %nxRoot%\NXBIN\libpman.dll %exportRoot%\NXBIN\
copy %nxRoot%\NXBIN\libsyss.dll %exportRoot%\NXBIN\
rem this is only included in 1953 and 1980
copy %nxRoot%\NXBIN\libnxc.dll %exportRoot%\NXBIN\
rem this is required from 2007.1700
copy %nxRoot%\NXBIN\libtomcrypt.dll %exportRoot%\NXBIN\
copy %nxRoot%\NXBIN\salt_clt.dll %exportRoot%\NXBIN\

goto END

:USAGE
echo "USAGE: %0 <NX_ROOT_DIR> <targetDir>"

:END