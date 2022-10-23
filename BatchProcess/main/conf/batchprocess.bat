@echo off

call "bp_tc_classpath.bat"

REM echo %~dp0
set RMX_HOME=%~dp0
set PARAM1=%1
set PARAM2=%2
set PARAM3=%3
set PARAM4=%4

set PATH=%PATH%;%FMS_HOME%\lib

java -cp "%RMX_HOME%.";"%RMX_HOME%bin/batchprocess.jar";"%RMX_HOME%bin/batchprocess_configs.jar";"%RMX_HOME%xlib/common"/*;"%RMX_HOME%xlib/jaxb-ri"/*;"%RMX_HOME%xlib/rmjavasdk"/*;%BP_TCLIBS_CP% com.nextlabs.drm.rmx.batchprocess.BatchProcessApp %PARAM1% %PARAM2% %PARAM3% %PARAM4%
