#!/bin/bash

. ./bp_tc_classpath.sh

RMX_HOME="$(dirname "$(readlink -f "$0")")/"
PARAM1=$1
PARAM2=$2
PARAM3=$3
PARAM4=$4
PARAM5=$5
PARAM6=$6

java -cp "${RMX_HOME}.":"${RMX_HOME}bin/batchprotect.jar":"${RMX_HOME}xlib/jar"/*:"${RMX_HOME}xlib/openaz"/*:"${RMX_HOME}xlib/jaxb-ri"/*:"${BP_TCLIBS_CP}" com.nextlabs.drm.rmx.batchprotect.BatchProtectApp $PARAM1 $PARAM2 $PARAM3 $PARAM4 $PARAM5 $PARAM6
