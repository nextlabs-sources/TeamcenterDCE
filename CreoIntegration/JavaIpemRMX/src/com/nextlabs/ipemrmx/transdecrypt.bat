@echo off
setlocal ENABLEDELAYEDEXPANSION 

REM Script to decrypt nxl file(s) using translate rights checker
REM
REM Usage:
REM   transdecrypt.bat "nxl file path or folder"
REM

if "%JRE_DIR%" == "" set JRE_DIR=C:\Java\jre1.8.0_191
if "%RMX_ROOT%" == "" set RMX_ROOT=C:\Program Files\Nextlabs\TcDRM
set RMX_HOME=%RMX_ROOT%\TranslatorRightsChecker
set NXL=*.nxl
set INDIR=%1
set LOGFILE=%INDIR%\trans_decrypt.log
set IPEMRMX_TEMP_DIR=%temp%\ipemrmx
(
echo Starting transdecrypt.bat %1
echo RMX_ROOT=%RMX_ROOT% 

cd /d %INDIR%

REM Save processed nxl files for cleanup
for %%F IN ( %NXL% ) DO echo %%~nF,>>nxlfiles 

REM Save processed stage folder for cleanup
echo %INDIR% > %IPEMRMX_TEMP_DIR%\trans_process

REM Run translate rights check to decrypt nxl files
"%JRE_DIR%\bin\java" -cp "%RMX_HOME%\.";"%RMX_HOME%\*";"%RMX_HOME%\openaz\*" com.nextlabs.drm.transrightschecker.TransRightsCheckerApp %1

endlocal

set EXITVALUE=%ERRORLEVEL% 

echo Exiting with errorlevel=%ERRORLEVEL%

EXIT /B %EXITVALUE% 
) > %LOGFILE%