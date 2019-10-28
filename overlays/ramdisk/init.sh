#!/bin/sh

PATH="/rescue"

if [ "`ps -o command 1 | tail -n 1 | ( read c o; echo ${o} )`" = "-s" ]; then
	echo "==> Running in single-user mode"
	SINGLE_USER="true"
fi

echo "==> Remount rootfs as read-write"
mount -u -w /

echo "==> Make mountpoints"
mkdir -p /cdrom /memdisk /sysroot /tmp


echo "==> Remount tmp with tmpfs"
mount -t tmpfs tmpfs /tmp

echo "Waiting for FURYBSD media to initialize"
while : ; do
    [ -e "/dev/iso9660/FURYBSD" ] && echo "found /dev/iso9660/FURYBSD" && break
    sleep 1
done

echo "==> Mount cdrom"
mount_cd9660 /dev/iso9660/FURYBSD /cdrom
mdmfs -P -F /cdrom/data/system.uzip -o ro md.uzip /sysroot

echo "--> Extract /etc from uzip"
tar -zcf /tmp/etc.txz -C /sysroot/etc .

echo "--> Extract /home from uzip"
tar -zcf /tmp/home.txz -C /sysroot/usr/home .

echo "--> Extract /root from uzip"
tar -zcf /tmp/root.txz -C /sysroot/root .

echo "--> Extract /var from uzip"
tar -zcf /tmp/var.txz -C /sysroot/var .

echo "--> Extract /usr/local/etc from uzip"
tar -zcf /tmp/usr-local-etc.txz -C /sysroot/usr/local/etc .

echo "--> Remount /etc with tmpfs"
mount -t tmpfs tmpfs /sysroot/etc

echo "--> Remount /home with tmpfs"
mount -t tmpfs tmpfs /sysroot/usr/home

echo "--> Remount /root with tmpfs"
mount -t tmpfs tmpfs /sysroot/root

echo "->> Remount /var with tmpfs"
mount -t tmpfs tmpfs /sysroot/var

echo "->> Remount /usr/local/etc with tmpfs"
mount -t tmpfs tmpfs /sysroot/usr/local/etc

echo "--> Restore /etc into writable layer"
tar -xf /tmp/etc.txz -C /sysroot/etc/

echo "--> Restore /home into writable layer"
tar -xf /tmp/home.txz -C /sysroot/usr/home/

echo "--> Restore /root into writable layer"
tar -xf /tmp/root.txz -C /sysroot/root/

echo "->> Restore /var into writable layer"
tar -xf /tmp/var.txz -C /sysroot/var/

echo "->> Restore /usr/local/etc into writable layer"
tar -xf /tmp/usr-local-etc.txz -C /sysroot/usr/local/etc/

echo "==> Mount devfs"
mount -t devfs devfs /sysroot/dev

BOOTMODE=`sysctl -n machdep.bootmethod`
export BOOTMODE

if [ "${BOOTMODE}" = "BIOS" ]; then
  echo "BIOS detected"
  mkdir -p /sysroot/usr/local/etc/X11/xorg.conf.d
  cp /sysroot/usr/home/liveuser/driver-vesa.conf /sysroot/usr/local/etc/X11/xorg.conf.d/driver-vesa.conf
fi

if [ "${BOOTMODE}" = "UEFI" ]; then
  echo "UEFI detected"
  mkdir -p /sysroot/usr/local/etc/X11/xorg.conf.d
  cp /sysroot/usr/home/liveuser/driver-scfb.conf /sysroot/usr/local/etc/X11/xorg.conf.d/driver-scfb.conf
fi

if [ "$SINGLE_USER" = "true" ]; then
	echo "Starting interactive shell in temporary rootfs ..."
	sh
fi

kenv init_shell="/bin/sh"
exit 0
