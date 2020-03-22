#!/bin/sh

PATH="/rescue"

mount -u -w / >/dev/null 2>/dev/null
mkdir -p /cdrom /union /usr >/dev/null 2>/dev/null
mount_cd9660 /dev/iso9660/FURYBSD /cdrom >/dev/null 2>/dev/null
mdmfs -P -F /cdrom/data/usr.uzip -o ro md.uzip /usr >/dev/null 2>/dev/null
mdmfs -s 512m md /union >/dev/null 2>/dev/null
mount -t unionfs /union /usr >/dev/null 2>/dev/null

BOOTMODE=$(sysctl -n machdep.bootmethod)
export BOOTMODE

if [ "${BOOTMODE}" = "BIOS" ]; then
  cp /usr/home/liveuser/xorg.conf.d/driver-vesa.conf /etc/X11/xorg.conf >/dev/null 2>/dev/null
fi

if [ "${BOOTMODE}" = "UEFI" ]; then
  cp /usr/home/liveuser/xorg.conf.d/driver-scfb.conf /etc/X11/xorg.conf >/dev/null 2>/dev/null
fi

VMGUEST=$(sysctl -n kern.vm_guest)
export VMGUEST

if [ "${VMGUEST}" = "xen" ]; then
  /usr/sbin/sysrc devd_enable="NO" >/dev/null 2>/dev/null
fi

if [ "${VMGUEST}" = "vmware" ]; then
  rm /etc/X11/xorg.conf >/dev/null 2>/dev/null
  cp /usr/home/liveuser/xorg.conf.d/driver-vmware.conf /etc/X11/xorg.conf >/dev/null 2>/dev/null
  /usr/sbin/sysrc -f /etc/rc.conf vmware_guest_vmblock_enable="YES" >/dev/null 2>/dev/null
  /usr/sbin/sysrc -f /etc/rc.conf vmware_guest_vmmemctl_enable="YES" >/dev/null 2>/dev/null
  /usr/sbin/sysrc -f /etc/rc.conf vmware_guestd_enable="YES" >/dev/null 2>/dev/null
  /usr/sbin/sysrc -f /etc/rc.conf moused_enable="YES" >/dev/null 2>/dev/null
fi


if [ -f "/usr/sbin/pciconf" ] ; then
  /usr/sbin/pciconf -lv 2>/dev/null | /usr/bin/grep -q VirtualBox 2>/dev/null
  if [ $? -eq 0 ] ; then
    rm /etc/X11/xorg.conf >/dev/null 2>/dev/null
    cp /usr/home/liveuser/xorg.conf.d/driver-virtualbox.conf /etc/X11/xorg.conf >/dev/null 2>/dev/null
    /usr/sbin/sysrc -f /etc/rc.conf vboxguest_enable="YES" >/dev/null 2>/dev/null
    /usr/sbin/sysrc -f /etc/rc.conf vboxservice_enable="YES" >/dev/null 2>/dev/null
  else
    /usr/sbin/pkg delete -fy virtualbox-ose-additions >/dev/null 2>/dev/null
  fi
fi

/usr/sbin/sysrc -f /etc/rc.conf kld_list+="sysctlinfo" >/dev/null 2>/dev/null

kenv init_shell="/bin/sh" >/dev/null 2>/dev/null
exit 0
