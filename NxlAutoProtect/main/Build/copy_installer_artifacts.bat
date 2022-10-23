@echo OFF
SET scriptDir=%~dp0
for %%a in (%scriptDir:~0,-1%) do set "projectDir=%%~dpa"
echo projectDir=%projectDir%
for %%a in (%projectDir:~0,-1%) do set "projectRoot=%%~dpa"
for %%a in (%projectRoot:~0,-1%) do set "srcRoot=%%~dpa"
echo srcRoot=%srcRoot%
set installArtifactsDir=%srcRoot%CADRMXInstaller\xlib\nx_rmx\NX_RMX_ROOT\
echo installArtifactsDir=%installArtifactsDir%

copy /Y %projectDir%log4cplus.properties %installArtifactsDir%

copy /Y %projectDir%Release\x64\NxlUtils64.dll %installArtifactsDir%bin
copy /Y %projectDir%Release\x64\RPMUtils64.dll %installArtifactsDir%bin

rmdir /S /Q %installArtifactsDir%NXIntegration2007
mkdir %installArtifactsDir%NXIntegration2007
mkdir %installArtifactsDir%NXIntegration2007\application
copy /Y %projectDir%NXIntegration\NXRMX\icons\* %installArtifactsDir%NXIntegration2007\application\
mkdir %installArtifactsDir%NXIntegration2007\startup
copy /Y %projectDir%NXIntegration\NXRMX\menus\* %installArtifactsDir%NXIntegration2007\startup\
copy /Y %projectDir%Release\x64\NXRMX2007.dll %installArtifactsDir%NXIntegration2007\startup\
copy /Y %projectDir%Release\x64\MiniRMX2007.dll %installArtifactsDir%NXIntegration2007\startup\
mkdir %installArtifactsDir%NXIntegration2007\udo\


rmdir /S /Q %installArtifactsDir%NXDRMIntegration
mkdir %installArtifactsDir%NXDRMIntegration
copy /Y %projectDir%Release\x64\NXDRMIntegration2007.dll %installArtifactsDir%NXDRMIntegration


rem rmdir /S /Q %installArtifactsDir%NXIntegration12
mkdir %installArtifactsDir%NXIntegration12
rmdir /S /Q %installArtifactsDir%NXIntegration12\application
mkdir %installArtifactsDir%NXIntegration12\application
copy /Y %projectDir%NXIntegration\NXRMX\icons\* %installArtifactsDir%NXIntegration12\application\
mkdir %installArtifactsDir%NXIntegration12\startup
del %installArtifactsDir%NXIntegration12\startup\nx_rmx_*
copy /Y %projectDir%NXIntegration\NXRMX\menus\* %installArtifactsDir%NXIntegration12\startup\
copy /Y %projectDir%Release\x64\NXRMX12.dll %installArtifactsDir%NXIntegration12\startup\
copy /Y %projectDir%Release\x64\MiniRMX12.dll %installArtifactsDir%NXIntegration12\startup\
mkdir %installArtifactsDir%NXIntegration12\udo\


rem rmdir /S /Q %installArtifactsDir%NXIntegration1980
mkdir %installArtifactsDir%NXIntegration1980
rmdir /S /Q %installArtifactsDir%NXIntegration1980\application
mkdir %installArtifactsDir%NXIntegration1980\application
copy /Y %projectDir%NXIntegration\NXRMX\icons\* %installArtifactsDir%NXIntegration1980\application\
mkdir %installArtifactsDir%NXIntegration1980\startup
del %installArtifactsDir%NXIntegration1980\startup\nx_rmx_*
copy /Y %projectDir%NXIntegration\NXRMX\menus\* %installArtifactsDir%NXIntegration1980\startup\
copy /Y %projectDir%Release\x64\NXRMX1980.dll %installArtifactsDir%NXIntegration1980\startup\
copy /Y %projectDir%Release\x64\MiniRMX1980.dll %installArtifactsDir%NXIntegration1980\startup\
mkdir %installArtifactsDir%NXIntegration1980\udo\


rem rmdir /S /Q %installArtifactsDir%NXIntegration1953
mkdir %installArtifactsDir%NXIntegration1953
rmdir /S /Q %installArtifactsDir%NXIntegration1953\application
mkdir %installArtifactsDir%NXIntegration1953\application
copy /Y %projectDir%NXIntegration\NXRMX\icons\* %installArtifactsDir%NXIntegration1953\application\
mkdir %installArtifactsDir%NXIntegration1953\startup
del %installArtifactsDir%NXIntegration1953\startup\nx_rmx_*
copy /Y %projectDir%NXIntegration\NXRMX\menus\* %installArtifactsDir%NXIntegration1953\startup\
copy /Y %projectDir%Release\x64\NXRMX1953.dll %installArtifactsDir%NXIntegration1953\startup\
copy /Y %projectDir%Release\x64\MiniRMX1953.dll %installArtifactsDir%NXIntegration1953\startup\
mkdir %installArtifactsDir%NXIntegration1953\udo\





