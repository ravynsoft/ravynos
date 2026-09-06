#!/bin/sh
if [ -z "$VER" ] ; then
	echo set VER!
	exit
fi
me=`pwd`

proj=gettext-tiny
projver=${proj}-${VER}

tempdir=/tmp/${proj}-0000
rm -rf $tempdir
mkdir -p $tempdir

cd $tempdir
GITDIR=https://github.com/rofl0r/$proj
GITDIR=$me
git clone "$GITDIR" $projver
rm -rf $projver/.git
rm -rf $projver/docs
rm -f $projver/.gitignore
rm -f $projver/create-dist.sh

tar cf $proj.tar $projver/
xz -z -9 -e $proj.tar
mv $proj.tar.xz $me/$projver.tar.xz
