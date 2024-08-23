#!/bin/sh
#
# This script is for pulling the latest translations from Translation Project
#
# Assumption: executed from the top level xkeyboard-config directory
#

project=xkeyboard-config

if [ ! -d po ] ; then
	echo "No po subdirectory in the current directory, the script has to be executed from the top level $project directory"
	exit 1
fi

rsync -Lrtvz translationproject.org::tp/latest/$project/  po
