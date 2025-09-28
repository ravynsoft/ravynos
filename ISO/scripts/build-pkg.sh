#!/bin/sh

# https://github.com/dwcarder/oneoff-pkg-create
# build a one-off FreeBSD package (outside of the ports ecosystem)
# 2017-02-13 Dale W. Carder <dwcarder@es.net>
#
# Requires:
#  1) a MANIFEST.template file specifying some package manifest metadata
#     with the exception of the files to be installed which this script
#     generates.
#
#  2) a Makefile in the current directory with an install target that 
#     allows specifying installation into a STAGEDIR directory. 
#  
#  OR) specify a directory that already contains a filesystem with
#     the components to be packaged.
#
# This code is forked from https://github.com/danrue/oneoff-pkg-create 
# by Dan Rue <drue@therub.org> which was in turn based on 
# https://github.com/bdrewery/freebsd_base_pkgng by Bryan Drewery 
# <bdrewery@FreeBSD.org>.

set -x

usage () {
	echo "Usage: $0 -m <manifest_template> [-d <files_directory>]"
	exit 1
}

OPTIND=1         # Reset in case getopts has been used previously in the shell.
#MANIFEST_TEMPLATE=""
STAGEDIR=""
FILES_MODE=false

while getopts "h?m:d:" opt; do
    case "$opt" in
    h|\?)
        usage
        exit 0
        ;;
    m)  MANIFEST_TEMPLATE=$OPTARG
        ;;
    d)  case $OPTARG in
	    # this mess removes trailing slashes.
            *[!/]*/) OPTARG=${OPTARG%"${OPTARG##*[!/]}"};;
        esac
	STAGEDIR=${OPTARG}
	FILES_MODE=true
        ;;
    esac
done

shift $((OPTIND-1))
[ "$1" = "--" ] && shift

if [ -z ${MANIFEST_TEMPLATE+x} ] ; then
	echo "manifest template not defined."
	usage
fi
if [ ! -e ${MANIFEST_TEMPLATE} ]; then
	echo "can't find manifest template. "
	usage
fi

if [ ${FILES_MODE} = true ]; then
   if [ ! -d ${STAGEDIR} ]; then
	echo "can't find files directory. "
	usage
   fi
else
	if [ ! -e Makefile ]; then
	   echo "can't find a Makefile in the current directory.  "
	   echo "Put one there that understands installing into a STAGEDIR, otherwise use the -d option."
	   usage
	fi

	# makefile mode, install into a temp dir that we will package up
	export STAGEDIR=/tmp/stage.$$
	mkdir ${STAGEDIR}
	make install
fi


DIR_SIZE=$(find ${STAGEDIR} -type f -exec stat -f %z {} + | awk 'BEGIN {s=0} {s+=$1} END {print s}')
export DIR_SIZE
{
	# parse the template
	. ${MANIFEST_TEMPLATE}
} > +MANIFEST

{
	# Add files in
	echo "files {"
	find ${STAGEDIR} -type f -exec sha256 -r {} + | sed 's: :\t:' |
       	   awk -F'[\t]' '{print "    \"" $2 "\" = \{sum: \"" $1 "\", uname: root, gname: wheel\} ;" }'

	# Add symlinks in
	find ${STAGEDIR} -type l |
		awk "{print \"    /\" \$1 \": '-'\"}"

	echo "}"
	
	# note, I haven't tested this
	# Add empty directories in
	#echo "directories:"
	#find ${STAGEDIR} -type d -mindepth 1 |
	#	awk '{print "    /" $1 ": y"}'

} | sed -e "s:${STAGEDIR}::" >> +MANIFEST


# Create the package
IGNORE_OSVERSION=yes pkg create --verbose -r ${STAGEDIR} -m . -o .

# Replace transient-packages-list with a new one reflecting all transient packages;
# this will be used for installing them later on
ls -lh
ls *.pkg > transient-packages-list
readlink -f transient-packages-list
cat transient-packages-list

# clean up our mess
rm +MANIFEST
if [ ${FILES_MODE} = false ]; then
	rm -r ${STAGEDIR}
fi
