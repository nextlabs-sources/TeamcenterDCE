@echo off
rem Teamcenter LDIF Generator

if "%OS%" == "Windows_NT" setlocal

set "CURRENT_DIR=%cd%"

echo Current directories is %CURRENT_DIR%
java -cp "%CURRENT_DIR%\*";"%CURRENT_DIR%\." com.nextlabs.tc.BuildLDIF -config.file "%CURRENT_DIR%\tc.properties"