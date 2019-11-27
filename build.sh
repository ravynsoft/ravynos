#!/bin/sh

cwd="`realpath | sed 's|/scripts||g'`"
workdir="/usr/local"
livecd="${workdir}/furybsd"
cache="${livecd}/cache"
version="12.0"
arch=AMD64
base="${cache}/${version}/base"
packages="${cache}/packages"
ports="${cache}/furybsd-ports-master"
iso="${livecd}/iso"
uzip="${livecd}/uzip"
cdroot="${livecd}/cdroot"
ramdisk_root="${cdroot}/data/ramdisk"
vol="furybsd"
label="FURYBSD"
isopath="${iso}/${vol}.iso"
desktop=$1
export DISTRIBUTIONS="kernel.txz base.txz"
export BSDINSTALL_DISTSITE="http://ftp.freebsd.org/pub/FreeBSD/releases/amd64/12.0-RELEASE/"
export BSDINSTALL_CHROOT="/usr/local/furybsd/uzip"
export BSDINSTALL_DISTDIR="/usr/local/furybsd/cache/12.0/base"


# Only run as superuser
if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root" 1>&2
  exit 1
fi

# Make sure git is installed
if [ ! -f "/usr/local/bin/git" ] ; then
  echo "Git is required"
  echo "Please install it with pkg install git or pkg install git-lite first"
  exit 1
fi

case $desktop in
  'kde')
    export desktop="kde"
    export edition="KDE"
    ;;
  'gnome')
    export desktop="gnome"
    export edition="GNOME"
    ;;
  *)
    export desktop="xfce"
    export edition="XFCE"
    ;;
esac

vol="FuryBSD-${version}-${edition}"
label="FURYBSD"
isopath="${iso}/${vol}.iso"

workspace()
{
  umount ${uzip}/var/cache/pkg >/dev/null 2>/dev/null
  umount ${ports} >/dev/null 2>/dev/null
  rm -rf ${ports} >/dev/null 2>/dev/null
  umount ${cache}/furybsd-packages/
  rm ${cache}/master.zip >/dev/null 2>/dev/null
  umount ${uzip}/dev >/dev/null 2>/dev/null
  if [ -d "${livecd}" ] ;then
    chflags -R noschg ${uzip} ${cdroot} >/dev/null 2>/dev/null
    rm -rf ${uzip} ${cdroot} ${ports} >/dev/null 2>/dev/null
  fi
  mkdir -p ${livecd} ${base} ${iso} ${packages} ${uzip} ${ramdisk_root}/dev ${ramdisk_root}/etc >/dev/null 2>/dev/null
}

base()
{
  if [ ! -f "${base}/base.txz" ] ; then 
    bsdinstall distfetch
  fi
  
  if [ ! -f "${base}/kernel.txz" ] ; then
    cd ${base}
    bsdinstall distfetch
  fi
  bsdinstall distextract
}

packages()
{
  cp /etc/resolv.conf ${uzip}/etc/resolv.conf
  mkdir ${uzip}/var/cache/pkg
  mount_nullfs ${packages} ${uzip}/var/cache/pkg
  mount -t devfs devfs ${uzip}/dev
  cat ${cwd}/settings/packages.common | xargs pkg-static -c ${uzip} install -y
  cat ${cwd}/settings/packages.${desktop} | xargs pkg-static -c ${uzip} install -y
  rm ${uzip}/etc/resolv.conf
  umount ${uzip}/var/cache/pkg
  umount ${uzip}/dev
}

ports()
{
  cp /etc/resolv.conf ${uzip}/etc/resolv.conf
  cd ${cache} && fetch https://github.com/furybsd/furybsd-ports/archive/master.zip
  cd ${cache} && unzip master.zip
  cd ${ports} && ./furybsd-make-ports
  mkdir -p ${uzip}/usr/local/furybsd/cache/furybsd-packages
  mkdir -p ${uzip}/usr/local/etc/pkg/repos
  cp ${cwd}/furybsd.conf ${uzip}/usr/local/etc/pkg/repos
  mount -t nullfs ${cache}/furybsd-packages/ ${uzip}/usr/local/furybsd/cache/furybsd-packages/
  mount -t devfs devfs ${uzip}/dev
  case $desktop in
    'kde')
      chroot ${uzip} pkg install -fy furybsd-kde-desktop
      ;;
    'gnome')
      chroot ${uzip} pkg install -fy furybsd-gnome-desktop
      ;;
    *)
      chroot ${uzip} pkg install -fy furybsd-xfce-desktop
      ;;
  esac
  rm ${cache}/master.zip
  rm ${uzip}/etc/resolv.conf
  umount ${uzip}/dev
  umount ${cache}/furybsd-packages/
  rm -rf ${ports}
  rm -rf ${uzip}/usr/local/furybsd/cache/furybsd-packages/
  rm -rf ${uzip}/usr/local/etc/pkg
}

rc()
{
  if [ ! -f "${uzip}/etc/rc.conf" ] ; then
    touch ${uzip}/etc/rc.conf
  fi
  if [ ! -f "${uzip}/etc/rc.conf.local" ] ; then
    touch ${uzip}/etc/rc.conf.local
  fi
  cat ${cwd}/settings/rc.conf.common | xargs chroot ${uzip} sysrc -f /etc/rc.conf.local
  cat ${cwd}/settings/rc.conf.${desktop} | xargs chroot ${uzip} sysrc -f /etc/rc.conf.local
}

live-settings()
{
  cp ${cwd}/nginx.conf ${uzip}/usr/local/etc/nginx/nginx.conf
  cp ${uzip}/usr/local/www/phpsysinfo/phpsysinfo.ini.new ${uzip}/usr/local/www/phpsysinfo/phpsysinfo.ini
  cp ${uzip}/usr/local/etc/php.ini-production ${uzip}/usr/local/etc/php.ini
}

user()
{
  mkdir -p ${uzip}/usr/home/liveuser/Desktop
  cp ${cwd}/fury-install ${uzip}/usr/home/liveuser/
  cp -R ${cwd}/xorg.conf.d/ ${uzip}/usr/home/liveuser/xorg.conf.d
  cp ${cwd}/fury-config-xorg.desktop ${uzip}/usr/home/liveuser/Desktop/
  cp ${cwd}/fury-install.desktop ${uzip}/usr/home/liveuser/Desktop/
  cp ${cwd}/fury-sysinfo.desktop ${uzip}/usr/home/liveuser/Desktop/
  cp ${cwd}/fury-desktop-readme.txt ${uzip}/usr/home/liveuser/Desktop/"Getting Started.txt"
  chroot ${uzip} echo furybsd | chroot ${uzip} pw mod user root -h 0
  chroot ${uzip} pw useradd liveuser -u 1000 \
  -c "Live User" -d "/home/liveuser" \
  -g wheel -G operator -m -s /bin/csh -k /usr/share/skel -w none
  chroot ${uzip} pw groupadd liveuser -g 1000
  chroot ${uzip} echo furybsd | chroot ${uzip} pw mod user liveuser -h 0
  chroot ${uzip} chown -R 1000:1000 /usr/home/liveuser
}

dm()
{
  case $desktop in
    'kde')
      cp ${cwd}/sddm.conf ${uzip}/usr/local/etc/
      ;;
    'gnome')
      cp ${cwd}/custom.conf ${uzip}/usr/local/etc/gdm/custom.conf
      ;;
    *)
      cp ${cwd}/lightdm.conf ${uzip}/usr/local/etc/lightdm/
      chroot ${uzip} sed -i '' -e 's/memorylocked=128M/memorylocked=256M/' /etc/login.conf
      chroot ${uzip} cap_mkdb /etc/login.conf
      ;;
  esac
}

uzip() 
{
  cp -R ${cwd}/overlays/uzip/ ${uzip}
  install -o root -g wheel -m 755 -d "${cdroot}"
  makefs "${cdroot}/data/usr.ufs" "${uzip}/usr"
  mkuzip -o "${cdroot}/data/usr.uzip" "${cdroot}/data/usr.ufs"
  rm -f "${cdroot}/data/usr.ufs"
  chflags -R noschg ${uzip}/usr
  rm -rf ${uzip}/usr
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
  if [ -d "${livecd}" ] ; then
    chflags -R noschg ${uzip} ${cdroot} >/dev/null 2>/dev/null
    rm -rf ${uzip} ${cdroot} >/dev/null 2>/dev/null
  fi
}

workspace
base
packages
ports
rc
dm
live-settings
user
uzip
ramdisk
boot
image
cleanup
