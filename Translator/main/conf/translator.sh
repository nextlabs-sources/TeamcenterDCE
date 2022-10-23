#!/bin/bash
RMX_HOME="$(dirname "$(readlink -f "$0")")/"
java -Dlog4j.configurationFile="${RMX_HOME}log4j2.properties" -cp "${RMX_HOME}.":"${RMX_HOME}*" com.nextlabs.drm.rmx.translator.TranslatorApp "$@"
