#!/bin/sh

PATH="/rescue"

mount -u -w /
mkdir -p /cdrom /union /usr
mount_cd9660 /dev/iso9660/FURYBSD /cdrom
mdmfs -P -F /cdrom/data/usr.uzip -o ro md.uzip /usr
mdmfs -s 512m md /union || exit 1 
mount -t unionfs /union /usr

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

/usr/sbin/sysrc -f /etc/rc.conf kld_list+="sysctlinfo"

kenv init_shell="/bin/sh"
exit 0
