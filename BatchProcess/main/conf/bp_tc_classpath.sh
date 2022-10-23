#!/bin/bash

if [ ! -n "${TC_ROOT}" ]; then
	echo "TC_ROOT must be set."
	exit 1
fi
echo TC_ROOT: "${TC_ROOT}"

tcVerPropsPath=`find "${TC_ROOT}"/portal/plugins -type f -name 'tc_version.properties'`
if [ ! -n "${tcVerPropsPath}" ]; then
	echo "Cannot find tc_version.properties in either 2-Tier RAC or 4-Tier RAC."
	exit 1
fi
# echo tcVerPropsPath: "${tcVerPropsPath}"

tcVer=`grep -R "MarketVersion" "${tcVerPropsPath}" | awk -F'=' '{print $2}'`
if [ ! -n "${tcVer}" ]; then
	echo "Cannot determine TC version from tc_verion.properties. Please contact NextLabs support."
	exit 1
fi
# echo tcVer: "${tcVer}"

tcInternalVer=`grep -R "InternalVersion" "${tcVerPropsPath}" | awk -F'=' '{print $2}'`
if [ ! -n "${tcInternalVer}" ]; then
	echo "Cannot determine TC internal version from tc_verion.properties. Please contact NextLabs support."
	exit 1
fi
# echo tcInternalVer: "${tcInternalVer}"

tcMajMinVer=$(echo "${tcInternalVer}" | cut -f 1 -d ".").$(echo "${tcInternalVer}" | cut -f 2 -d ".")
# echo tcMajMinVer: "${tcMajMinVer}"


# according to base installer versioning 
BP_TCLIBS_CP=:
BP_CP="$(dirname "$(readlink -f "$0")")/"
if [ ${tcVer} == 10 ]; then
	echo "TC Version: TC10.1.0"
	BP_TCLIBS_CP="${TC_ROOT}/portal/plugins/TcSoaClient_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationLoose_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationTypes_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCommon_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreLoose_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreTypes_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryLoose_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryTypes_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaStrongModel_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaCoreStrong_10000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_10000.1.0/jacorb.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_10000.1.0/avalon-framework-4.1.5.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_10000.1.0/logkit.jar"
fi

if [ ${tcVer} == 11 ]; then
	echo "TC Version: TC11.2.0"
	BP_TCLIBS_CP="${TC_ROOT}/portal/plugins/TcSoaClient_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationLoose_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationTypes_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCommon_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreLoose_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreTypes_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryLoose_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryTypes_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaStrongModel_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaCoreStrong_11000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_11000.2.0/jacorb.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_11000.2.0/avalon-framework-4.1.5.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_11000.2.0/logkit.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/org.apache.log4j_1.2.15.v201012070815.jar"
fi

if [[ $(tcMajMinVer) == "12.3"* ]]; then
	echo "TC Version: TC12.3.0.0"
	BP_TCLIBS_CP="${TC_ROOT}/portal/plugins/TcSoaClient_12000.3.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationLoose_12000.3.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCommon_12000.3.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreLoose_12000.3.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreTypes_12000.3.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryLoose_12000.3.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaStrongModel_12000.3.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaCoreStrong_12000.3.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_12000.3.0/jacorb.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_12000.3.0/avalon-framework-4.1.5.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_12000.3.0/logkit.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/org.apache.log4j_1.2.15.v201012070815.jar"
fi

if [[ $(tcMajMinVer) == "12.4"* ]]; then
	echo "TC Version: TC12.4.0.0"
	BP_TCLIBS_CP="${TC_ROOT}/portal/plugins/TcSoaClient_12000.4.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationLoose_12000.4.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCommon_12000.4.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreLoose_12000.4.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreTypes_12000.4.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryLoose_12000.4.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaStrongModel_12000.4.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaCoreStrong_12000.4.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_12000.4.0/jacorb.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_12000.4.0/avalon-framework-4.1.5.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_12000.4.0/logkit.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/org.apache.log4j_1.2.15.v201012070815.jar"
fi

if [ ${tcVer} == 12 ]; then
	echo "TC Version: TC12.2.0.0"
	BP_TCLIBS_CP="${TC_ROOT}/portal/plugins/TcSoaClient_12000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationLoose_12000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCommon_12000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreLoose_12000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreTypes_12000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryLoose_12000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaStrongModel_12000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaCoreStrong_12000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_12000.2.0/jacorb.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_12000.2.0/avalon-framework-4.1.5.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_12000.2.0/logkit.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/org.apache.log4j_1.2.15.v201012070815.jar"
fi

if [[ $(tcMajMinVer) == "13.1"* ]]; then
	echo "TC Version: TC13.1.0.0"
	BP_TCLIBS_CP="${TC_ROOT}/portal/plugins/TcSoaClient_13000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationLoose_13000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCommon_13000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreLoose_13000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreTypes_13000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryLoose_13000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaStrongModel_13000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaCoreStrong_13000.1.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_13000.1.0/jacorb.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_13000.1.0/avalon-framework-4.1.5.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_13000.1.0/logkit.jar"
fi

if [[ $(tcMajMinVer) == "13.2"* ]]; then
	echo "TC Version: TC13.2.0.0"
	BP_TCLIBS_CP="${TC_ROOT}/portal/plugins/TcSoaClient_13000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationLoose_13000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCommon_13000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreLoose_13000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreTypes_13000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryLoose_13000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaStrongModel_13000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaCoreStrong_13000.2.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_13000.2.0/jacorb.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_13000.2.0/avalon-framework-4.1.5.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_13000.2.0/logkit.jar"
fi

if [ ${tcVer} == 13 ]; then
	echo "TC Version: TC13.0.0.0"
	BP_TCLIBS_CP="${TC_ROOT}/portal/plugins/TcSoaClient_13000.0.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaAdministrationLoose_13000.0.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCommon_13000.0.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreLoose_13000.0.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaCoreTypes_13000.0.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/TcSoaQueryLoose_13000.0.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaStrongModel_13000.0.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${BP_CP}xlib/tcjar/TcSoaCoreStrong_13000.0.0.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_13000.0.0/jacorb.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_13000.0.0/avalon-framework-4.1.5.jar"
	BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/com.teamcenter.rac.external_13000.0.0/logkit.jar"
fi

export BP_TCLIBS_CP
export BP_CP
# Clear up temp variables
BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/tccs/jar/fccclient.jar"
BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/tccs/jar/fscclient.jar"
BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/tcgatewaystubs.jar"
BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/tcserverjavabinding.jar"
BP_TCLIBS_CP="${BP_TCLIBS_CP}":"${TC_ROOT}/portal/plugins/tcmemjavabinding.jar"
echo ==============================================================
tcVerPropsPath=
tcVer=
tcInternalVer=
tcMajMinVer=
