#!/bin/sh

# Build only those packages that we don't already have

if [ $(id -u) -gt 0 ]; then
    SUDO=sudo
fi
PKGDIR=/usr/ports/packages
PORTSROOT=$HOME/obj.$(uname -m)/portsroot
EXT=pkg
CLOUDSMITH=$HOME/.local/bin/cloudsmith

${SUDO} chroot $PORTSROOT pkg info | awk '{print $1}' > /tmp/packages.list
for p in $(cat /tmp/packages.list); do
    if [ ! -f $PKGDIR/$p.${EXT} ]; then
        ${SUDO} chroot $PORTSROOT pkg create -o /mnt $p
    fi
done

cd $PORTSROOT/mnt
ls

for p in *.${EXT}; do
    $CLOUDSMITH push raw -k $CSKEY airyx/core $p
done

${SUDO} cp *.${EXT} $PKGDIR/
${SUDO} pkg repo -o $PKGDIR $PKGDIR

cd $PKGDIR
for f in packagesite meta; do
    if [ -L ${f}.txz ]; then
        ${SUDO} rm -f ${f}.txz
        ${SUDO} cp ${f}.${EXT} ${f}.txz
    fi
done

for p in packagesite.* meta.*; do
    $CLOUDSMITH push raw -k $CSKEY airyx/core $p
done

echo === Repository updated
