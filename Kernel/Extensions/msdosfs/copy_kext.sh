#!/bin/bash
#
# Copy the Development build (as built by Xcode) of msdosfs.kext to one or
# more test machines.  It will unload the kext if previously loaded,
# touch /System/Library/Extensions to invalidate the KEXT cache, manually
# load the new KEXT, and copy the symbols back to the development machine
# (in a directory named for the target machine).
#
# The target machine host names are passed as command line arguments.  If
# there are no command line arguments, but $MACHINE is set, then the target
# host name is in $MACHINE.  It is an error if there are no command line
# arguments and $MACHINE is not set.
#
# This script doesn't currently handle the case where the KEXT cannot be
# unloaded (usually because a volume is still mounted).  It will just copy
# the new KEXT over and fail to load it.
#
# Note that the kext gets copied to the current user's home directory
# on the target machine.  That way, if it panics, you can always reboot
# and get the standard KEXT from /System/Library/Extensions.
#
# Also note that this script assumes it can SSH as root to the target
# machine.  In a default install, root has no password, and no SSH keys,
# so SSH to root fails unless you give root a password or SSH keys.  I prefer
# to set up root's SSH authorized keys to be my user's keys, so my user
# can SSH to root without any prompt.  Here's how I do it:
#
# ~ $ sudo -s
# Password:
# ~ # cd ~root
# ~root # mkdir .ssh
# ~root # cp ~mark/.ssh/authorized_keys* .ssh
# ~root # exit
# ~ $
#

configuration=Debug
kext=msdosfs

if [ "$1" = "-config" ]; then
    configuration="$2"
    shift 2
fi

if [ $# -eq 0 ]; then
    if [ -z "$MACHINE" ]; then
	echo 'No arguments, and $MACHINE not set!' 1>&2
	exit 1
    fi
    set $MACHINE
fi

#
# Determine the $BUILT_PRODUCTS_DIR for the given configuration
#
BUILT_PRODUCTS_DIR=$(xcodebuild -configuration $configuration -showBuildSettings 2>/dev/null | sed -n -E '1,$ s/[ \t]+BUILT_PRODUCTS_DIR = //p')

for m
do
    ssh root@$m kextunload -b com.apple.filesystems.${kext}
    scp -r "$BUILT_PRODUCTS_DIR/${kext}.kext" root@$m:"/var/tmp"
    ssh root@$m chgrp -R wheel "/var/tmp/${kext}.kext"
    ssh root@$m touch /System/Library/Extensions
    ssh root@$m kextutil -c -s /tmp "/var/tmp/${kext}.kext" || exit
    mkdir -p /tmp/$m
    scp root@$m:/tmp/"com.apple.*.sym" /tmp/$m
    rm -rf /tmp/$m/${kext}.kext{,.dSYM}
    cp -r "$BUILT_PRODUCTS_DIR/${kext}.kext"{,.dSYM} /tmp/$m
done
