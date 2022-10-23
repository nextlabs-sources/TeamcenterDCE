set BB_LIB=%RMX_ROOT%\BBIntegration

java -cp "%BB_LIB%\.";"%BB_LIB%\*" com.nextlabs.drm.bczhelper.BczHelper "%~1"
exit
