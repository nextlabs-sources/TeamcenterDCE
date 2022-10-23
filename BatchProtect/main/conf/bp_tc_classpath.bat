@echo off

if not defined TC_ROOT echo TC_ROOT must be set.& goto end
echo TC_ROOT: %TC_ROOT%

FOR /F "tokens=* delims=\" %%i IN ('where /R "%TC_ROOT%"\portal\plugins tc_version.properties') DO set tcVerPropsPath=%%i
if not defined tcVerPropsPath echo Cannot find tc_version.properties in either 2-Tier RAC or 4-Tier RAC.& goto end
REM echo tcVerPropsPath: %tcVerPropsPath%

FOR /F "usebackq eol=# tokens=1, 2 delims==" %%A IN ("%tcVerPropsPath%") DO if "%%A"=="MarketVersion" set tcVer=%%B
if not defined tcVer echo Cannot determine TC version from tc_version.properties. Please contact NextLabs support.& goto end
REM echo tcVer: %tcVer%

FOR /F "usebackq eol=# tokens=1, 2 delims==" %%A IN ("%tcVerPropsPath%") DO if "%%A"=="InternalVersion" set tcInternalVer=%%B
if not defined tcInternalVer echo Cannot determine TC version from tc_version.properties. Please contact NextLabs support.& goto end
REM echo tcInternalVer: %tcInternalVer%

set tcMajMinVer=%tcInternalVer%
FOR /F "tokens=1, 2 delims==.(" %%A IN ("%tcMajMinVer%") DO set tcMajMinVer=%%A.%%B
REM echo tcMajMinVer: %tcMajMinVer%

REM according to base installer versioning 
set BP_TCLIBS_CP=;
set BP_CP=%~dp0
if %tcVer% == 11 goto tc11cp
if %tcMajMinVer% == 12.3 goto tc12_3cp
if %tcMajMinVer% == 12.4 goto tc12_4cp
if %tcVer% == 12 goto tc12cp
if %tcMajMinVer% == 13.1 goto tc13_1cp
if %tcMajMinVer% == 13.2 goto tc13_2cp
if %tcMajMinVer% == 13.3 goto tc13_3cp
if %tcVer% == 13 goto tc13cp
	
:tc11cp
echo TC Version: TC11.2.0
set BP_TCLIBS_CP="%TC_ROOT%"\portal\plugins\TcSoaClient_11000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCommon_11000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreLoose_11000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreTypes_11000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaQueryLoose_11000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaQueryTypes_11000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaStrongModel_11000.2.0.jar"
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaCoreStrong_11000.2.0.jar"
goto end

:tc12cp
echo TC Version: TC12.0.0.0
set BP_TCLIBS_CP="%TC_ROOT%"\portal\plugins\TcSoaClient_12000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCommon_12000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreLoose_12000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreTypes_12000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaQueryLoose_12000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaStrongModel_12000.2.0.jar"
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaCoreStrong_12000.2.0.jar"
goto end

:tc12_3cp
echo TC Version: TC12.3.0.0
set BP_TCLIBS_CP="%TC_ROOT%"\portal\plugins\TcSoaClient_12000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCommon_12000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreLoose_12000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreTypes_12000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaQueryLoose_12000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaStrongModel_12000.3.0.jar"
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaCoreStrong_12000.3.0.jar"
goto end

:tc12_4cp
echo TC Version: TC12.4.0.0
set BP_TCLIBS_CP="%TC_ROOT%"\portal\plugins\TcSoaClient_12000.4.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCommon_12000.4.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreLoose_12000.4.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreTypes_12000.4.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaQueryLoose_12000.4.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaStrongModel_12000.4.0.jar"
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaCoreStrong_12000.4.0.jar"
goto end

:tc13cp
echo TC Version: TC13.0.0.0
set BP_TCLIBS_CP="%TC_ROOT%"\portal\plugins\TcSoaClient_13000.0.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCommon_13000.0.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreLoose_13000.0.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreTypes_13000.0.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaQueryLoose_13000.0.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaStrongModel_13000.0.0.jar"
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaCoreStrong_13000.0.0.jar"
goto end

:tc13_1cp
echo TC Version: TC13.1.0.0
set BP_TCLIBS_CP="%TC_ROOT%"\portal\plugins\TcSoaClient_13000.1.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCommon_13000.1.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreLoose_13000.1.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreTypes_13000.1.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaQueryLoose_13000.1.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaStrongModel_13000.1.0.jar"
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaCoreStrong_13000.1.0.jar"
goto end

:tc13_2cp
echo TC Version: TC13.2.0.0
set BP_TCLIBS_CP="%TC_ROOT%"\portal\plugins\TcSoaClient_13000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCommon_13000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreLoose_13000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreTypes_13000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaQueryLoose_13000.2.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaStrongModel_13000.2.0.jar"
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaCoreStrong_13000.2.0.jar"
goto end

:tc13_3cp
echo TC Version: TC13.3.0.0
set BP_TCLIBS_CP="%TC_ROOT%"\portal\plugins\TcSoaClient_13000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCommon_13000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreLoose_13000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaCoreTypes_13000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%TC_ROOT%"\portal\plugins\TcSoaQueryLoose_13000.3.0.jar
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaStrongModel_13000.3.0.jar"
set BP_TCLIBS_CP=%BP_TCLIBS_CP%;"%BP_CP%xlib\tcjar\TcSoaCoreStrong_13000.3.0.jar"
goto end

:end
REM Clear up temp variables
echo ==============================================================
set tcVerPropsPath=
set tcVer=
set tcInternalVer=
set tcMajMinVer=
