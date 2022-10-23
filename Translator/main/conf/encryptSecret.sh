#!/bin/bash
RMX_HOME="$(dirname "$(readlink -f "$0")")/"
java -cp "${RMX_HOME}.":"${RMX_HOME}slf4j-api-1.7.21.jar":"${RMX_HOME}slf4j-log4j12-1.7.21.jar":"${RMX_HOME}log4j-1.2.17.jar":"${RMX_HOME}log4j-api-2.17.1.jar":"${RMX_HOME}log4j-core-2.17.1.jar":"${RMX_HOME}jakarta.activation.jar":"${RMX_HOME}jakarta.xml.bind-api.jar":"${RMX_HOME}translator.jar" com.nextlabs.drm.rmx.configuration.SecretUtil
