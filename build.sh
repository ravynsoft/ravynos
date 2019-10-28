#!/bin/sh

cwd="`realpath | sed 's|/scripts||g'`"
workdir="/usr/local"
livecd="${workdir}/furybsd"
cache="${livecd}/cache"
version=12.0
arch=AMD64
base="${cache}/${version}/base"
packages="${cache}/packages"
iso="${livecd}/iso"
uzip="${livecd}/uzip"
cdroot="${livecd}/cdroot"
ramdisk_root="${cdroot}/data/ramdisk"
vol="furybsd"
label="FURYBSD"
isopath="${iso}/${vol}.iso"

# Only run as superuser
if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root" 1>&2
  exit 1
fi

workspace()
{
  umount ${uzip}/var/cache/pkg >/dev/null 2>/dev/null
  umount ${uzip}/dev >/dev/null 2>/dev/null
  if [ -d "${livecd}" ] ;then
    chflags -R noschg ${uzip} ${cdroot} >/dev/null 2>/dev/null
    rm -rf ${uzip} ${cdroot} >/dev/null 2>/dev/null
  fi
  mkdir -p ${livecd} ${base} ${iso} ${packages} ${uzip} ${ramdisk_root}/dev ${ramdisk_root}/etc >/dev/null 2>/dev/null
}

base()
{
  if [ ! -f "${base}/base.txz" ] ; then 
    cd ${base}
    fetch http://ftp.freebsd.org/pub/FreeBSD/releases/amd64/${version}-RELEASE/base.txz
  fi
  
  if [ ! -f "${base}/kernel.txz" ] ; then
    cd ${base}
    fetch http://ftp.freebsd.org/pub/FreeBSD/releases/amd64/${version}-RELEASE/kernel.txz
  fi
  cd ${base}
  tar -zxvf base.txz -C ${uzip}
  tar -zxvf kernel.txz -C ${uzip}
  touch ${uzip}/etc/fstab
}

packages()
{
  cp /etc/resolv.conf ${uzip}/etc/resolv.conf
  mkdir ${uzip}/var/cache/pkg
  mount_nullfs ${packages} ${uzip}/var/cache/pkg
  mount -t devfs devfs ${uzip}/dev
  cat ${cwd}/settings/packages | xargs pkg-static -c ${uzip} install -y
  rm ${uzip}/etc/resolv.conf
  umount ${uzip}/var/cache/pkg
  umount ${uzip}/dev || true
}

rc()
{
  if [ ! -f "${uzip}/etc/rc.conf" ] ; then
    touch ${uzip}/etc/rc.conf
  fi
  cat ${cwd}/settings/rc | xargs chroot ${uzip} sysrc -f /etc/rc.conf
}

user()
{
  mkdir -p ${uzip}/usr/home/liveuser/Desktop
  cp ${cwd}/fury-install ${uzip}/usr/home/liveuser/
  cp ${cwd}/xorg.conf.d/ ${uzip}/xorg.conf.d
  cp ${cwd}/fury-install.desktop ${uzip}/usr/home/liveuser/Desktop/
  chroot ${uzip} echo furybsd | chroot ${uzip} pw mod user root -h 0
  chroot ${uzip} pw useradd liveuser \
  -c "Live User" -d "/home/liveuser" \
  -g wheel -G operator -m -s /bin/csh -k /usr/share/skel -w none
  chroot ${uzip} pw groupadd liveuser
  chroot ${uzip} echo furybsd | chroot ${uzip} pw mod user liveuser -h 0
  chroot ${uzip} chown -R 1001:1001 /usr/home/liveuser
  cp ${cwd}/doas.conf ${uzip}/usr/local/etc/
  cp ${cwd}/sddm.conf ${uzip}/usr/local/etc/
}

uzip() 
{
  install -o root -g wheel -m 755 -d "${cdroot}"
  makefs "${cdroot}/data/system.ufs" "${uzip}"
  mkuzip -o "${cdroot}/data/system.uzip" "${cdroot}/data/system.ufs"
  rm -f "${cdroot}/data/system.ufs"
}

ramdisk() 
{
  cp -R ${cwd}/overlays/ramdisk/ ${ramdisk_root}
  cd "${uzip}" && tar -cf - rescue | tar -xf - -C "${ramdisk_root}"
  touch "${ramdisk_root}/etc/fstab"
  cp ${uzip}/etc/login.conf ${ramdisk_root}/etc/login.conf
  makefs -b '10%' "${cdroot}/data/ramdisk.ufs" "${ramdisk_root}"
  gzip "${cdroot}/data/ramdisk.ufs"
  rm -rf "${ramdisk_root}"
}

boot() 
{
  cp -R ${cwd}/overlays/boot/ ${cdroot}
  cd "${uzip}" && tar -cf - --exclude boot/kernel boot | tar -xf - -C "${cdroot}"
  for kfile in kernel geom_uzip.ko nullfs.ko tmpfs.ko unionfs.ko; do
  tar -cf - boot/kernel/${kfile} | tar -xf - -C "${cdroot}"
  done
}

image() 
{
  sh ${cwd}/scripts/mkisoimages.sh -b $label $isopath ${cdroot}
}

cleanup()
{
  if [ -d "${livecd}" ] ;then
    chflags -R noschg ${uzip} ${cdroot} >/dev/null 2>/dev/null
    rm -rf ${uzip} ${cdroot} >/dev/null 2>/dev/null
  fi
}

workspace
base
packages
rc
user
uzip
ramdisk
boot
image
cleanup
