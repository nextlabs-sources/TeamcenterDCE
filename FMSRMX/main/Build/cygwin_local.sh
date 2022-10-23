#!/bin/bash
#
# DESCRIPTION
#	Invoke custom application build script
#
# USAGE:
#	Steps to invoke this script.
#		export NLEXTERNALDIR=<path>
#		export NLPROFSERVDIR=<path>
#		export BUILD_NUMBER=<#>
#		source setEnv
#		./buildDev
#
# HUDSON BUILD:
#	cd $WORKSPACE
#	export NLEXTERNALDIR=c:/hudson/jobs/external_latest/workspace
#	export NLPROFSERVDIR=c:/depot/ProfessionServices/buildCommon
#	source setEnv
#	./buildDev

#
# Compile
#

LOG_FILE=build_local.log

(
# Prepare environment
export BUILDTYPE="debug"

PWD=`pwd`
echo CurrentDirectory=$(PWD)
export NLPROJECTROOT=`cygpath -m $PWD`
echo NLPROJECTROOT=$NLPROJECTROOT

$NLPROJECTROOT/configure --type=dev || exit $?
source $NLPROJECTROOT/build.config

OSVERSION=`uname`

# Prepare dependencies
$ANT -f $NLPROJECTROOT/Build/build_xlib.xml || exit $?

# Compile code and build installers
make -f $NLPROJECTROOT/Build/Makefile.compile || exit $?

) 2>&1 | tee $LOG_FILE

exit ${PIPESTATUS[0]}
