@echo off

call "bp_tc_classpath.bat"

REM echo %~dp0
set RMX_HOME=%~dp0
set PARAM1=%1
set PARAM2=%2
set PARAM3=%3
set PARAM4=%4
set PARAM5=%5
set PARAM6=%6
java -cp "%RMX_HOME%.";"%RMX_HOME%bin/batchprotect.jar";"%RMX_HOME%xlib/jar"/*;"%RMX_HOME%xlib/openaz"/*;"%RMX_HOME%xlib/jaxb-ri"/*;%BP_TCLIBS_CP% com.nextlabs.drm.rmx.batchprotect.BatchProtectApp %PARAM1% %PARAM2% %PARAM3% %PARAM4% %PARAM5% %PARAM6%
