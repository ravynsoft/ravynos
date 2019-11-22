#!/bin/sh

cwd="`realpath | sed 's|/scripts||g'`"
workdir="/usr/local"
livecd="${workdir}/furybsd"
cache="${livecd}/cache"
version="12.0"
arch=AMD64
base="${cache}/${version}/base"
packages="${cache}/packages"
ports="/usr/ports"
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
  rm -rf ${cache}/furybsd-ports-master/ >/dev/null 2>/dev/null
  rm ${cache}/master.zip >/dev/null 2>/dev/null
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
    bsdinstall distfetch
  fi
  
  if [ ! -f "${base}/kernel.txz" ] ; then
    cd ${base}
    bsdinstall distfetch
  fi
  bsdinstall distextract
  echo "/dev/iso9660/FURYBSD / cd9660 ro 0 0" > ${uzip}/etc/fstab
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
  if [ -d "${ports}/Mk" ] ; then
    portsnap fetch update
  else
    portsnap fetch extract
  fi
  cd ${cache} && fetch https://github.com/furybsd/furybsd-ports/archive/master.zip
  cd ${cache} && unzip master.zip
  cd ${cache}/furybsd-ports-master && ./mkport.sh x11-themes/furybsd-wallpapers
  cd ${ports}/x11-themes/furybsd-wallpapers && make package
  cp ${ports}/x11-themes/furybsd-wallpapers/work/pkg/* ${uzip}
  cd ${cache}/furybsd-ports-master && ./mkport.sh sysutils/furybsd-dsbdriverd
  cd ${ports}/sysutils/furybsd-dsbdriverd && make package
  cp ${ports}/sysutils/furybsd-dsbdriverd/work/pkg/* ${uzip}
  cd ${cache}/furybsd-ports-master && ./mkport.sh x11-drivers/furybsd-xorg-tool
  cd ${ports}/x11-drivers/furybsd-xorg-tool && make package
  cp ${ports}/x11-drivers/furybsd-xorg-tool/work/pkg/* ${uzip}
  case $desktop in
    'kde')
      echo "no settings port yet"
      ;;
    'gnome')
      echo "no settings port yet"
      ;;
    *)
      cd ${cache}/furybsd-ports-master && ./mkport.sh x11/furybsd-xfce-settings
      cd ${ports}/x11/furybsd-xfce-settings && make package
      cp ${ports}/x11/furybsd-xfce-settings/work/pkg/* ${uzip}
      ;;
  esac
  mount -t devfs devfs ${uzip}/dev
  chroot ${uzip} /bin/sh -c "ls /furybsd* | xargs pkg add"
  chroot ${uzip} /bin/sh -c "ls /furybsd* | xargs rm"
  chroot ${uzip} /bin/sh -c "ls /dsbdriverd* | xargs pkg add"
  chroot ${uzip} /bin/sh -c "ls /dsbdriverd* | xargs rm"
  rm -rf ${cache}/furybsd-ports-master/
  rm ${cache}/master.zip
  umount ${uzip}/dev
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
  cp ${cwd}/doas.conf ${uzip}/usr/local/etc/
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

loader()
{
  cp ${cwd}/overlays/boot/boot/loader.conf ${uzip}/boot/loader.conf
  cp ${cwd}/overlays/ramdisk/init.sh ${uzip}/init.sh
}

tar()
{
  chroot ${uzip} tar -zcf /etc.txz -C /etc .
  chroot ${uzip} tar -zcf /var.txz -C /var .
  chroot ${uzip} tar -zcf /modules.txz -C /boot/modules .
  chroot ${uzip} mv /etc/login.conf /login.conf.bak
  chroot ${uzip} rm -rf /etc
  chroot ${uzip} chflags -R noschg /var
  chroot ${uzip} rm -rf /var
  chroot ${uzip} chflags -R noschg /boot/modules
  chroot ${uzip} rm -rf /boot/modules
  chroot ${uzip} mkdir /etc
  chroot ${uzip} mv /login.conf.bak /etc/login.conf
  chroot ${uzip} mkdir /var
  chroot ${uzip} mkdir /boot/modules
  chroot ${uzip} mkdir /compat
}

uzip() 
{
  install -o root -g wheel -m 755 -d "${cdroot}"
  makefs "${cdroot}/data/system.ufs" "${uzip}/usr"
  mkuzip -o "${uzip}/system.uzip" "${cdroot}/data/system.ufs"
  rm -f "${cdroot}/data/system.ufs"
  chflags -R noschg ${uzip}/usr
  rm -rf ${uzip}/usr
  mkdir ${uzip}/usr
  mkdir ${uzip}/memdisk
}

image() 
{
  sh ${cwd}/scripts/mkisoimages.sh -b $label $isopath ${uzip}
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
loader
tar
uzip
image
cleanup
