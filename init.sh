#!/bin/sh

PATH="/rescue"

if [ "`ps -o command 1 | tail -n 1 | ( read c o; echo ${o} )`" = "-s" ]; then
	echo "==> Running in single-user mode"
	SINGLE_USER="true"
fi

echo "Waiting for FURYBSD media to initialize"
while : ; do
    [ -e "/dev/iso9660/FURYBSD" ] && echo "found /dev/iso9660/FURYBSD" && break
    sleep 1
done

mount -t tmpfs tmpfs /etc
mount -t tmpfs tmpfs /usr/home
mount -t tmpfs tmpfs /tmp
mount -t tmpfs tmpfs /var
tar -xf /etc.txz -C /etc
tar -xf /home.txz -C /usr/home
tar -xf /var.txz -C /var

echo "==> Mount cdrom"
mdmfs -P -F /system.uzip -o ro md.uzip /usr/local

if [ "$SINGLE_USER" = "true" ]; then
	echo -n "Enter memdisk size used for read-write access in the live system: "
	read MEMDISK_SIZE
else
	MEMDISK_SIZE="1024"
fi

echo "==> Mount swap-based memdisk"
mdmfs -s "${MEMDISK_SIZE}m" md /memdisk || exit 1
mount -t unionfs /memdisk /usr/local

BOOTMODE=`sysctl -n machdep.bootmethod`
export BOOTMODE

if [ "${BOOTMODE}" = "BIOS" ]; then
  echo "BIOS detected"
  cp /usr/home/liveuser/xorg.conf.d/driver-vesa.conf /etc/X11/xorg.conf
fi

if [ "${BOOTMODE}" = "UEFI" ]; then
  echo "UEFI detected"
  cp /usr/home/liveuser/xorg.conf.d/driver-scfb.conf /etc/X11/xorg.conf
fi

VMGUEST=`sysctl -n kern.vm_guest`
export VMGUEST

if [ "${VMGUEST}" = "xen" ]; then
  echo "XEN guest detected"
  sysrc devd_enable="NO"
fi

sysrc -f /etc/rc.conf kld_list+="sysctlinfo"

if [ "$SINGLE_USER" = "true" ]; then
	echo "Starting interactive shell in temporary rootfs ..."
	sh
fi

kenv init_shell="/bin/sh"
exit 0
