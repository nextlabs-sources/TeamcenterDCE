@echo OFF
set RMX_CONFIG="%RMX_ROOT%\\config\\"
set RMX_LIB="%RMX_ROOT%\\Translator\\tonxlfile\\bin\\"
java -cp %RMX_CONFIG%.;%RMX_LIB%slf4j-api-1.7.21.jar;%RMX_LIB%slf4j-log4j12-1.7.21.jar;%RMX_LIB%log4j-1.2.17.jar;"%RMX_LIB%log4j-api-2.17.1.jar";"%RMX_LIB%log4j-core-2.17.1.jar";"%RMX_LIB%jakarta.activation.jar";"%RMX_LIB%jakarta.xml.bind-api.jar";%RMX_LIB%translator.jar com.nextlabs.drm.rmx.configuration.ConfigManagerUtil
