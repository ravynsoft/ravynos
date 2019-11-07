#!/bin/sh

PATH="/rescue"

if [ "`ps -o command 1 | tail -n 1 | ( read c o; echo ${o} )`" = "-s" ]; then
	echo "==> Running in single-user mode"
	SINGLE_USER="true"
fi

echo "==> Remount rootfs as read-write"
mount -u -w /

echo "==> Make mountpoints"
mkdir -p /cdrom /memdisk /mnt /sysroot /tmp

echo "Waiting for FURYBSD media to initialize"
while : ; do
    [ -e "/dev/iso9660/FURYBSD" ] && echo "found /dev/iso9660/FURYBSD" && break
    sleep 1
done

echo "==> Mount cdrom"
mount_cd9660 /dev/iso9660/FURYBSD /cdrom
mdmfs -P -F /cdrom/data/system.uzip -o ro md.uzip /sysroot

if [ "$SINGLE_USER" = "true" ]; then
	echo -n "Enter memdisk size used for read-write access in the live system: "
	read MEMDISK_SIZE
else
	MEMDISK_SIZE="1024"
fi

echo "==> Mount swap-based memdisk"
mdmfs -s "${MEMDISK_SIZE}m" md /memdisk || exit 1
mount -t unionfs /memdisk /sysroot

mkdir -p /sysroot/media/cdrom
mount_nullfs -o ro /cdrom /sysroot/media/cdrom

mount -t tmpfs tmpfs /sysroot/tmp
mount -t tmpfs tmpfs /sysroot/mnt
mount -t nullfs /sysroot/tmp /tmp
mount -t nullfs /sysroot/mnt /mnt

echo "==> Mount devfs"
mount -t devfs devfs /sysroot/dev

BOOTMODE=`sysctl -n machdep.bootmethod`
export BOOTMODE

if [ "${BOOTMODE}" = "BIOS" ]; then
  echo "BIOS detected"
  cp /sysroot/usr/home/liveuser/xorg.conf.d/driver-vesa.conf /sysroot/etc/X11/xorg.conf
fi

if [ "${BOOTMODE}" = "UEFI" ]; then
  echo "UEFI detected"
  cp /sysroot/usr/home/liveuser/xorg.conf.d/driver-scfb.conf /sysroot/etc/X11/xorg.conf
fi

VMGUEST=`sysctl -n kern.vm_guest`
export VMGUEST

if [ "${VMGUEST}" = "xen" ]; then
  echo "XEN guest detected"
  chroot /sysroot sysrc devd_enable="NO"
fi

if [ "$SINGLE_USER" = "true" ]; then
	echo "Starting interactive shell in temporary rootfs ..."
	sh
fi

kenv init_shell="/bin/sh"
exit 0
