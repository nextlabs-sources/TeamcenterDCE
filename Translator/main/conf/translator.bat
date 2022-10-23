echo %~dp0
set RMX_HOME=%~dp0
set PARAM1=%1
set PARAM2=%2
set PARAM3=%3
set PARAM4=%4
set PARAM5=%5
set PARAM6=%6
set PARAM7=%7
set PARAM8=%8
set PARAM9=%9
shift
shift
shift
shift
shift
shift
shift
shift
shift
java -Dlog4j.configurationFile="%RMX_HOME%log4j2.properties" -cp "%RMX_HOME%.";"%RMX_HOME%*" com.nextlabs.drm.rmx.translator.TranslatorApp %PARAM1% %PARAM2% %PARAM3% %PARAM4% %PARAM5% %PARAM6% %PARAM7% %PARAM8% %PARAM9% %1 %2 %3 %4 %5 %6
