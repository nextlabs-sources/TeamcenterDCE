#!/bin/bash

SCRIPTDIR=$(dirname $(readlink -e $0))
echo ScriptDirectory=$SCRIPTDIR

OSVERSION=`uname`
echo OS = $OSVERSION
if  [[ $OSVERSION == CYGWIN* ]]; then
	export NLPROJECTROOT=`cygpath -m $SCRIPTDIR`
	export XLIB_DIR="//storageserver/share/data/Gavin/build_artifacts"
else
	export NLPROJECTROOT=$SCRIPTDIR
	export XLIB_DIR="/mnt/storageserver/share/data/Gavin/build_artifacts"
fi

if [ "$NLEXTERNALDIR" == "" ]; then
	export NLEXTERNALDIR="/mnt/gavin/p4/external"
fi

if [ "$NLEXTERNALDIR2" == "" ]; then
	export NLEXTERNALDIR2="/mnt/gavin/p4/main/external"
fi

export BUILDTYPE=Release

export CONFIG_TYPE=DEBUG

export PROJECT_NAME="TeamcenterDCE"
export VERSION_MAJOR=5
export VERSION_MINOR=2
export VERSION_MAINTENANCE=0
export VERSION_PATCH=0
export BUILD_NUMBER=999
export VERSION_STR="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_MAINTENANCE.$VERSION_PATCH"
export VERSION_BUILD_SHORT="$BUILD_NUMBER"
export BUILD_DATE=$(date +"%Y%m%d")
export BUILD_DATE_LONG=$(date +"%Y%m%d%H%M")
export REPOSITORY_ROOT="$SCRIPTDIR/repository_root"

export XLIB_NX_TEMPLATE_LEGACY_LIBS="$XLIB_DIR/DocumentControlExtension-2.7.0.0-16-201701200207.zip"
#`make -f Makefile.compile`

# Write file

(
cat <<EOT
#!/bin/bash

export OSVERSION=$OSVERSION
export PROJECT_NAME="$PROJECT_NAME"
export VERSION_MAJOR="$VERSION_MAJOR"
export VERSION_MINOR="$VERSION_MINOR"
export VERSION_MAINTENANCE="$VERSION_MAINTENANCE"
export VERSION_PATCH="$VERSION_PATCH"
export VERSION_STR="$VERSION_STR"
export VERSION_BUILD_SHORT="$VERSION_BUILD_SHORT"
export BUILD_NUMBER="$BUILD_NUMBER"
export BUILD_DATE="$BUILD_DATE"
export BUILD_DATE_LONG="$BUILD_DATE_LONG"

export REPOSITORY_ROOT="$REPOSITORY_ROOT"

export XLIB_NX_TEMPLATE_LEGACY_LIBS="$XLIB_NX_TEMPLATE_LEGACY_LIBS"

export NLPROJECTROOT="$NLPROJECTROOT"
export NLEXTERNALDIR="$NLEXTERNALDIR"
export NLEXTERNALDIR2="$NLEXTERNALDIR2"

export CONFIG_TYPE="$CONFIG_TYPE"
export BUILDTYPE="$BUILDTYPE"
EOT
) > "$SCRIPTDIR/buildLocal.config"