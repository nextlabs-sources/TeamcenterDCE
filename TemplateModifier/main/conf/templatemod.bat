@echo off
REM echo %~dp0
set RMX_HOME=%~dp0
set PARAM1=%1
set PARAM2=%2
set PARAM3=%3
set PARAM4=%4
set PARAM5=%5
set PARAM6=%6
java -cp "%RMX_HOME%.";"%RMX_HOME%bin/templatemodifier.jar";"%RMX_HOME%bin/log4j-api-2.17.1.jar";"%RMX_HOME%bin/log4j-core-2.17.1.jar" com.nextlabs.drm.rmx.templatemodifier.TemplateModifierApp %PARAM1% %PARAM2% %PARAM3% %PARAM4% %PARAM5% %PARAM6%
