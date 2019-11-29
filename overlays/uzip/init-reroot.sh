#!/bin/sh

PATH="/rescue"

mount -u -w / >/dev/null 2>/dev/null
mkdir -p /cdrom /union /usr >/dev/null 2>/dev/null
mount_cd9660 /dev/iso9660/FURYBSD /cdrom >/dev/null 2>/dev/null
mdmfs -P -F /cdrom/data/usr.uzip -o ro md.uzip /usr >/dev/null 2>/dev/null
mdmfs -s 512m md /union >/dev/null 2>/dev/null
mount -t unionfs /union /usr >/dev/null 2>/dev/null

BOOTMODE=`sysctl -n machdep.bootmethod`
export BOOTMODE

if [ "${BOOTMODE}" = "BIOS" ]; then
  cp /usr/home/liveuser/xorg.conf.d/driver-vesa.conf /etc/X11/xorg.conf >/dev/null 2>/dev/null
fi

if [ "${BOOTMODE}" = "UEFI" ]; then
  cp /usr/home/liveuser/xorg.conf.d/driver-scfb.conf /etc/X11/xorg.conf >/dev/null 2>/dev/null
fi

VMGUEST=`sysctl -n kern.vm_guest`
export VMGUEST

if [ "${VMGUEST}" = "xen" ]; then
  /usr/sbin/sysrc devd_enable="NO" >/dev/null 2>/dev/null
fi

/usr/sbin/sysrc -f /etc/rc.conf kld_list+="sysctlinfo" >/dev/null 2>/dev/null

kenv init_shell="/bin/sh" >/dev/null 2>/dev/null
exit 0
