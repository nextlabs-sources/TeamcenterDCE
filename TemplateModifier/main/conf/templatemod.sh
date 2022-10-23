#!/bin/bash
RMX_HOME="$(dirname "$(readlink -f "$0")")/"
PARAM1=$1
PARAM2=$2
PARAM3=$3
PARAM4=$4
PARAM5=$5
PARAM6=$6
java -cp "${RMX_HOME}.":"${RMX_HOME}bin/templatemodifier.jar":"${RMX_HOME}bin/log4j-api-2.17.1.jar":"${RMX_HOME}bin/log4j-core-2.17.1.jar" com.nextlabs.drm.rmx.templatemodifier.TemplateModifierApp $PARAM1 $PARAM2 $PARAM3 $PARAM4 $PARAM5 $PARAM6
