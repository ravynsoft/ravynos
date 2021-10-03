#!/bin/sh

gpart destroy -F ada0
gpart create -s gpt ada0
#gpart set -a lenovofix ada0
gpart add -t efi -s 1m -l efi0 ada0
newfs_msdos /dev/gpt/efi0
gpart add -t freebsd-swap -l swap0 -a 1m -s 2048m ada0
gpart add -t freebsd-zfs -l tank0 -a 1m ada0

# Create the root pool.
zpool create -R /mnt -O mountpoint=/ \
    -O atime=off -O canmount=off -O compression=on \
    tank ada0p3
zfs create -o canmount=off -o mountpoint=none tank/ROOT
zfs create -o mountpoint=/ tank/ROOT/default
zpool set bootfs=tank/ROOT/default tank
zfs create tank/Users
zfs create -o canmount=off tank/usr
zfs create tank/usr/local
zfs create tank/usr/obj
zfs create tank/usr/src
zfs create tank/usr/ports
zfs create tank/usr/ports/distfiles
zfs create -o canmount=off tank/var
zfs create tank/var/jail
zfs create tank/var/log
zfs create tank/var/tmp
zfs create tank/tmp


exit

# Post install stuff

cat >> /boot/loader.conf <<END
zfs_load="YES"
vfs.root.mountfrom="zfs:tank/ROOT/default"
END

cat >> /etc/rc.conf <<END
zfs_enable="YES"
zfsd_enable="YES"
END

cat >> /etc/fstab <<END
/dev/gpt/efi0 /boot/efi0 msdosfs rw,late 0 0
END

# UEFI:
mkdir /boot/efi0
mount -t msdosfs /dev/gpt/efi0 /boot/efi0

# UEFI with fallback naming:
# If we have a box that's happy with the fallback naming, we can just do that.
mkdir -p /boot/efi0/efi/boot
cp /boot/loader.efi /boot/efi0/efi/boot/bootx64.efi

# UEFI:
kldload efirt
mkdir -p /boot/efi0/efi/freebsd
cp /boot/loader.efi /boot/efi0/efi/freebsd/
efibootmgr -c -L freebsd0 -l ada0p1:/efi/freebsd/loader.efi

# Check to see what numbers were actually assigned first.
# Under FreeBSD < 13:
efibootmgr -a 1
efibootmgr -a 2
# Under FreeBSD 13, the syntax has changed:
efibootmgr -a -b 1
efibootmgr -a -b 2
# Under 13 this might be unnecessary. Check.
efibootmgr -o 1,2
