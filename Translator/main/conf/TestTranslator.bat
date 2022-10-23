rem This batch file is for running unit test in Dev environment

java -Dlog4j.configuration=file:bin/log4j2.properties -cp ./bin/*;. com.nextlabs.drm.rmx.test.TestRunner
