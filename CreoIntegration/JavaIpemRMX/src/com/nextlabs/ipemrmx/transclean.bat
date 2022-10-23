@echo off
setlocal ENABLEDELAYEDEXPANSION 

REM Script to clean up
REM
REM Usage:
REM   transclean.bat "stage folder"
REM
set INDIR=%1
set LOGFILE=%INDIR:"=%\trans_clean.log 
set nxlfiles=%%INDIR:"=%\nxlfiles
(
echo Starting transclean.bat %1

cd /d %INDIR%

for /F "delims=," %%f IN ( nxlfiles ) DO (
echo Deleting %%f 
del "%%f"
echo. 
) 

endlocal

set EXITVALUE=%ERRORLEVEL% 
cd /d %CURRDIR%

echo Exiting with errorlevel=%ERRORLEVEL%

EXIT /B %EXITVALUE% 
) > %LOGFILE%