#!/bin/sh

# Exit on errors
set -e

# Determine the version of the running host system.
# Building ISOs for other major versions than the running host system
# is not supported and results in broken images anyway
#version=$(uname -r | cut -d "-" -f 1-2) # "12.2-RELEASE" or "13.0-CURRENT"
version="12.2-RELEASE"

if [ "${version}" = "13.0-CURRENT" ] ; then
  # version="13.0-RC3"
  version="13.0-RELEASE"
fi

VER=$(uname -r | cut -d "-" -f 1) # "12.2" or "13.0"
MAJOR=$(uname -r | cut -d "." -f 1) # "12" or "13"

if [ -z "${HELIUM}" ]; then
	HELIUM=$(pwd)/../helium
fi
HELIUMPKG=${HELIUM}/freebsd-src/release
HELIUMVER=$(head -1 ${HELIUM}/version)

# Dwnload from either https://download.freebsd.org/ftp/releases/
#                  or https://download.freebsd.org/ftp/snapshots/
VERSIONSUFFIX=$(uname -r | cut -d "-" -f 2) # "RELEASE" or "CURRENT"
FTPDIRECTORY="releases" # "releases" or "snapshots"
if [ "${VERSIONSUFFIX}" = "CURRENT" ] ; then
  FTPDIRECTORY="snapshots"
fi
# RCs are in the 'releases' ftp directory; hence check if $VERSIONSUFFIX begins with 'RC' https://serverfault.com/a/252406
if [ "${VERSIONSUFFIX#RC}"x = "${VERSIONSUFFIX}x" ]  ; then
  FTPDIRECTORY="releases"
fi

echo "${FTPDIRECTORY}"

# pkgset="branches/2020Q1" # TODO: Use it
desktop=$1
tag=$2
export cwd=$(realpath | sed 's|/scripts||g')
if [ -z "${workdir}" ]; then
	workdir="/usr/local"
fi
livecd="${workdir}/furybsd"
if [ -z "${arch}" ] ; then
  arch=amd64
fi
cache="${livecd}/${arch}/cache"
base="${cache}/${version}/base"
export packages="${cache}/packages"
iso="${livecd}/iso"
  if [ -n "$CIRRUS_CI" ] ; then
    # On Cirrus CI ${livecd} is in tmpfs for speed reasons
    # and tends to run out of space. Writing the final ISO
    # to non-tmpfs should be an acceptable compromise
    iso="${CIRRUS_WORKING_DIR}/artifacts"
  fi
export uzip="${livecd}/uzip"
export cdroot="${livecd}/cdroot"
ramdisk_root="${cdroot}/data/ramdisk"
vol="furybsd"
label="LIVE"
export DISTRIBUTIONS="kernel.txz base.txz helium.txz"

# Only run as superuser
if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root" 1>&2
  exit 1
fi

# Make sure git is installed
# We only need this in case we decide to pull in ingredients from
# other git repositories; this is currently not the case
# if [ ! -f "/usr/local/bin/git" ] ; then
#   echo "Git is required"
#   echo "Please install it with pkg install git or pkg install git-lite first"
#   exit 1
# fi

if [ -z "${desktop}" ] ; then
  export desktop=xfce
fi
edition=$(echo $desktop | tr '[:lower:]' '[:upper:]')
export edition
if [ ! -f "${cwd}/settings/packages.${desktop}" ] ; then
  echo "${cwd}/settings/packages.${desktop} is missing, exiting"
  exit 1
fi

# Get the version tag
mkdir -p ${workdir}/furybsd
if [ -z "$2" ] ; then
  rm ${workdir}/furybsd/tag >/dev/null 2>/dev/null || true
  export vol="${VER}"
else
  rm ${workdir}/furybsd/version >/dev/null 2>/dev/null || true
  echo "${2}" > ${workdir}/furybsd/tag
  export vol="${VER}-${tag}"
fi

# Get the short git SHA
SHA=$(echo ${CIRRUS_CHANGE_IN_REPO}| cut -c1-7)

# The environment variable BUILDNUMBER may have been set; if so, use it
if [ ! -z "${BUILDNUMBER}" ] ; then
  isopath="${iso}/${desktop}-${BUILDNUMBER}-${vol}-${arch}.iso"
elif [ ! -z "${SHA}" ] ; then
  isopath="${iso}/${desktop}-${SHA}-${vol}-${arch}.iso"
else
  isopath="${iso}/${desktop}-${vol}-${arch}.iso"
fi

# For helloSystem, we are using a different naming scheme for the ISOS
if [ "${desktop}" = "hello" ] ; then
  if [ -f overlays/uzip/hello/manifest ] ; then
    HELLO_VERSION=$(grep "^version:" overlays/uzip/hello/manifest | xargs | cut -d " " -f 2 | cut -d "_" -f 1)
    # If we are building hello, then set version number of the 'hello' transient package
    # based on environment variable set e.g., by Cirrus CI
    if [ ! -z $BUILDNUMBER ] ; then
      echo "Injecting $BUILDNUMBER" into manifest
      sed -i -e 's|\(^version:       .*_\).*$|\1'$BUILDNUMBER'|g' "${cwd}/overlays/uzip/hello/manifest"
      rm "${cwd}/overlays/uzip/hello/manifest-e"
      cat "${cwd}/overlays/uzip/hello/manifest"
      isopath="${iso}/${desktop}-${HELLO_VERSION}_${BUILDNUMBER}-FreeBSD-${VER}-${arch}.iso"
    else
      #isopath="${iso}/${desktop}-${HELLO_VERSION}_git${SHA}-FreeBSD-${VER}-${arch}.iso"
      isopath="${HELIUM}/dist/hello-${vol}-${arch}.iso"
    fi
  fi
fi

cleanup()
{
  if [ -n "$CIRRUS_CI" ] ; then
    # On CI systems there is no reason to clean up which takes time
    return 0
  else
    umount ${uzip}/var/cache/pkg >/dev/null 2>/dev/null || true
    umount ${uzip}/dev >/dev/null 2>/dev/null || true
    zpool destroy -f furybsd >/dev/null 2>/dev/null || true
    mdconfig -d -u 0 >/dev/null 2>/dev/null || true
    rm ${livecd}/pool.img >/dev/null 2>/dev/null || true
    rm -rf ${cdroot} >/dev/null 2>/dev/null || true
  fi
}

workspace()
{
  mkdir -p "${livecd}" "${base}" "${iso}" "${packages}" "${uzip}" "${ramdisk_root}/dev" "${ramdisk_root}/etc" >/dev/null 2>/dev/null
  truncate -s 3g "${livecd}/pool.img"
  mdconfig -f "${livecd}/pool.img" -u 0
  gpart create -s GPT md0
  gpart add -t freebsd-zfs md0
  sync ### Needed?
  zpool create furybsd /dev/md0p1
  sync ### Needed?
  zfs set mountpoint="${uzip}" furybsd
  # From FreeBSD 13 on, zstd can be used with zfs in base
  MAJOR=$(uname -r | cut -d "." -f 1)
  if [ $MAJOR -lt 13 ] ; then
    zfs set compression=gzip-6 furybsd 
  else
    zfs set compression=zstd-9 furybsd 
  fi

}

base()
{
  # TODO: Signature checking
  #if [ ! -f "${base}/base.txz" ] ; then 
  #  cd ${base}
  #  fetch https://download.freebsd.org/ftp/${FTPDIRECTORY}/${arch}/${version}/base.txz
  #fi
  
  #if [ ! -f "${base}/kernel.txz" ] ; then
  #  cd ${base}
  #  fetch https://download.freebsd.org/ftp/${FTPDIRECTORY}/${arch}/${version}/kernel.txz
  #fi
  cd ${base}
  tar -zxvf ${HELIUMPKG}/base.txz -C ${uzip}
  tar -zxvf ${HELIUMPKG}/kernel.txz -C ${uzip}
  tar -zxvf ${HELIUMPKG}/helium.txz -C ${uzip}
  touch ${uzip}/etc/fstab
}

pkg_add_from_url()
{
      url=$1
      pkg_cachesubdir=$2
      abi=${3+env ABI=$3}

      pkgfile=${url##*/}
      if [ ! -e ${uzip}${pkg_cachedir}/${pkg_cachesubdir}/${pkgfile} ]; then
        fetch -o ${uzip}${pkg_cachedir}/${pkg_cachesubdir}/ $url
      fi
      IGNORE_OSVERSION=yes /usr/local/sbin/pkg-static -c "${uzip}" install -y $(/usr/local/sbin/pkg-static query -F ${uzip}${pkg_cachedir}/${pkg_cachesubdir}/${pkgfile} %dn)
      IGNORE_OSVERSION=yes $abi /usr/local/sbin/pkg-static -c "${uzip}" add ${pkg_cachedir}/${pkg_cachesubdir}/${pkgfile}
      IGNORE_OSVERSION=yes /usr/local/sbin/pkg-static -c "${uzip}" lock -y $(/usr/local/sbin/pkg-static query -F ${uzip}${pkg_cachedir}/${pkg_cachesubdir}/${pkgfile} %o)
}

packages()
{
  # NOTE: Also adjust the Nvidia drivers accordingly below. TODO: Use one set of variables
  if [ $MAJOR -eq 12 ] ; then
    # echo "Major version 12, hence using release_2 packages since quarterly can be missing packages from one day to the next"
    # sed -i -e 's|quarterly|release_2|g' "${uzip}/etc/pkg/FreeBSD.conf"
    # rm -f "${uzip}/etc/pkg/FreeBSD.conf-e"
    echo "Major version 12, hence using quarterly packages to see whether it performs better than release_2"
  elif [ $MAJOR -eq 13 ] ; then
    echo "Major version 13, hence using quarterly packages since release_2 will probably not have compatible Intel driver"
  else
    echo "Other major version, hence changing /etc/pkg/FreeBSD.conf to use latest packages"
    sed -i -e 's|quarterly|latest|g' "${uzip}/etc/pkg/FreeBSD.conf"
    rm -f "${uzip}/etc/pkg/FreeBSD.conf-e"
  fi
  cp /etc/resolv.conf ${uzip}/etc/resolv.conf
  mkdir ${uzip}/var/cache/pkg
  mount_nullfs ${packages} ${uzip}/var/cache/pkg
  mount -t devfs devfs ${uzip}/dev
  # FIXME: In the following line, the hardcoded "i386" needs to be replaced by "${arch}" - how?
  for p in common-${MAJOR} ${desktop}; do
    sed '/^#/d;/\!i386/d;/^cirrus:/d;/^https:/d' "${cwd}/settings/packages.$p" | \
      xargs /usr/local/sbin/pkg-static -c "${uzip}" install -y
    pkg_cachedir=/var/cache/pkg
    # Install packages beginning with 'cirrus:'
    mkdir -p ${uzip}${pkg_cachedir}/furybsd-cirrus
    for url in $(sed -ne "s,^cirrus:,https://api.cirrus-ci.com/v1/artifact/,;s,%%ARCH%%,$arch,;s,%%VER%%,$VER,p" "${cwd}/settings/packages.$p"); do
        pkg_add_from_url "$url" furybsd-cirrus
    done
    # Install packages beginning with 'https:'
    mkdir -p ${uzip}${pkg_cachedir}/furybsd-https
    for url in $(grep -e '^https' "${cwd}/settings/packages.$p"); do
        # ABI=freebsd:12:$arch in an attempt to use package built on 12 for 13
        pkg_add_from_url "$url" furybsd-https "freebsd:12:$arch"
    done
  done
  # Install the packages we have generated in pkg() that are listed in transient-packages-list
  ls -lh "${packages}/transient/"
  while read -r p; do
    # FIXME: Is there something like "/usr/local/sbin/pkg-static add" that can be used
    # to install local packages (not from a repository) that will
    # resolve dependencies from the repositories?
    # The following will just fail in the case of unmet dependencies
    /usr/local/sbin/pkg-static -r ${uzip} add "${packages}/transient/${p}" # pkg-static add has no -y
  done <"${packages}/transient/transient-packages-list"
  
  # /usr/local/sbin/pkg-static -c ${uzip} info > "${cdroot}/data/system.uzip.manifest"
  # Manifest of installed packages ordered by size in bytes
  /usr/local/sbin/pkg-static -c ${uzip} query "%sb\t%n\t%v\t%c" | sort -r -s -n -k 1,1 > "${cdroot}/data/system.uzip.manifest"
  cp "${cdroot}/data/system.uzip.manifest" "${isopath}.manifest"
}

rc()
{
  if [ ! -f "${uzip}/etc/rc.conf" ] ; then
    touch ${uzip}/etc/rc.conf
  fi
  if [ ! -f "${uzip}/etc/rc.conf.local" ] ; then
    touch ${uzip}/etc/rc.conf.local
  fi
  cat "${cwd}/settings/rc.conf.common" | xargs chroot "${uzip}" sysrc -f /etc/rc.conf.local
  cat "${cwd}/settings/rc.conf.${desktop}" | xargs chroot "${uzip}" sysrc -f /etc/rc.conf.local
}


repos()
{
  # This is just an example of how a git repo needs to be structured
  # so that it can be consumed directly here
  # if [ ! -d "${cwd}/overlays/uzip/furybsd-common-settings" ] ; then
  #   git clone https://github.com/probonopd/furybsd-common-settings.git ${cwd}/overlays/uzip/furybsd-common-settings
  # else
  #   cd ${cwd}/overlays/uzip/furybsd-common-settings && git pull
  # fi
  true
}

user()
{
  mkdir -p ${uzip}/usr/home/liveuser/Desktop
  # chroot ${uzip} echo furybsd | chroot ${uzip} pw mod user root -h 0
  chroot ${uzip} pw useradd liveuser -u 1000 \
  -c "Live User" -d "/home/liveuser" \
  -g wheel -G operator -m -s /usr/bin/zsh -k /usr/share/skel -w none
  chroot ${uzip} pw groupadd liveuser -g 1000
  # chroot ${uzip} echo furybsd | chroot ${uzip} pw mod user liveuser -h 0
  chroot ${uzip} chown -R 1000:1000 /usr/home/liveuser
  chroot ${uzip} pw groupmod wheel -m liveuser
  chroot ${uzip} pw groupmod video -m liveuser
  chroot ${uzip} pw groupmod webcamd -m liveuser
}

dm()
{
  case $desktop in
    'kde')
      ;;
    'gnome')
      ;;
    'lumina')
      ;;
    'mate')
      chroot ${uzip} sed -i '' -e 's/memorylocked=128M/memorylocked=256M/' /etc/login.conf
      chroot ${uzip} cap_mkdb /etc/login.conf
      ;;
    'xfce')
      ;;
  esac
}

# Generate transient packages for the selected overlays
pkg()
{
  mkdir -p "${packages}/transient"
  cd "${packages}/transient"
  rm -f *.txz # Make sure there are no leftover transient packages from earlier runs
  while read -r p; do
    sh -ex "${cwd}/scripts/build-pkg.sh" -m "${cwd}/overlays/uzip/${p}"/manifest -d "${cwd}/overlays/uzip/${p}/files"
  done <"${cwd}"/settings/overlays.common
  if [ -f "${cwd}/settings/overlays.${desktop}" ] ; then
    while read -r p; do
      sh -ex "${cwd}/scripts/build-pkg.sh" -m "${cwd}/overlays/uzip/${p}"/manifest -d "${cwd}/overlays/uzip/${p}/files"
    done <"${cwd}/settings/overlays.${desktop}"
  fi
  cd -
}

# Put Nvidia driver at location in which initgfx expects it
initgfx()
{
  if [ "${arch}" != "i386" ] ; then
    MAJOR=$(uname -r | cut -d "." -f 1)
    if [ $MAJOR -lt 14 ] ; then
      PKGS="quarterly"
    else
      PKGS="latest"
    fi
    # for ver in '' 390 340 304; do
    for ver in ''; do # Only use NVIDIA version 440 for now to reduce ISO image size
        pkgfile=$(/usr/local/sbin/pkg-static -c ${uzip} rquery %n-%v.txz nvidia-driver${ver:+-$ver})
        fetch -o "${cache}/" "https://pkg.freebsd.org/FreeBSD:${MAJOR}:amd64/${PKGS}/All/${pkgfile}"
        mkdir -p "${uzip}/usr/local/nvidia/${ver:-440}/"
        tar xfC "${cache}"/${pkgfile} "${uzip}/usr/local/nvidia/${ver:-440}/"
        ls "${uzip}/usr/local/nvidia/${ver:-440}/+COMPACT_MANIFEST"
    done
  fi

  ls

  rm ${uzip}/etc/resolv.conf
  umount ${uzip}/var/cache/pkg
  umount ${uzip}/dev
}

script()
{
  if [ -e "${cwd}/settings/script.${desktop}" ] ; then
    # cp "${cwd}/settings/script.${desktop}" "${uzip}"/tmp/script
    # chmod +x "${uzip}"/tmp/script
    # chroot "${uzip}" /tmp/script
    # rm "${uzip}"/tmp/script
    "${cwd}/settings/script.${desktop}"
  fi
}

uzip() 
{
  install -o root -g wheel -m 755 -d "${cdroot}"
  sync ### Needed?
  cd ${cwd} && zpool export furybsd && while zpool status furybsd >/dev/null; do :; done 2>/dev/null
  sync ### Needed?
  mkuzip -S -d -o "${cdroot}/data/system.uzip" "${livecd}/pool.img"
}

ramdisk() 
{
  cp -R "${cwd}/overlays/ramdisk/" "${ramdisk_root}"
  sync ### Needed?
  cd ${cwd} && zpool import furybsd && zfs set mountpoint=/usr/local/furybsd/uzip furybsd
  sync ### Needed?
  cd "${uzip}" && tar -cf - rescue | tar -xf - -C "${ramdisk_root}"
  touch "${ramdisk_root}/etc/fstab"
  cp ${uzip}/etc/login.conf ${ramdisk_root}/etc/login.conf
  makefs -b '10%' "${cdroot}/data/ramdisk.ufs" "${ramdisk_root}"
  gzip -f "${cdroot}/data/ramdisk.ufs"
  rm -rf "${ramdisk_root}"
}

boot() 
{
  cp -R "${cwd}/overlays/boot/" "${cdroot}"
  cd "${uzip}" && tar -cf - boot | tar -xf - -C "${cdroot}"
  # Remove all modules from the ISO that is not required before the root filesystem is mounted
  # The whole directory /boot/modules is unnecessary
  rm -rf "${cdroot}"/boot/modules/*
  # Remove modules in /boot/kernel that are not loaded at boot time
  find "${cdroot}"/boot/kernel -name '*.ko' \
    -not -name 'cryptodev.ko' \
    -not -name 'firewire.ko' \
    -not -name 'geom_uzip.ko' \
    -not -name 'opensolaris.ko' \
    -not -name 'tmpfs.ko' \
    -not -name 'xz.ko' \
    -not -name 'zfs.ko' \
    -delete
  # Compress the kernel
  gzip "${cdroot}"/boot/kernel/kernel
  # Compress the remaining modules
  find "${cdroot}"/boot/kernel -type f -name '*.ko' -exec gzip {} \;
  sync ### Needed?
  cd ${cwd} && zpool export furybsd && mdconfig -d -u 0
  sync ### Needed?
  # The name of a dependency for zfs.ko changed, violating POLA
  # If we are loading both modules, then at least 13 cannot boot, hence only load one based on the FreeBSD major version
  MAJOR=$(uname -r | cut -d "." -f 1)
  if [ $MAJOR -lt 13 ] ; then
    echo "Major version < 13, hence using opensolaris.ko"
    sed -i -e 's|opensolaris_load=".*"|opensolaris_load="YES"|g' "${cdroot}"/boot/loader.conf
    rm -f "${cdroot}"/boot/loader.conf-e
    sed -i -e 's|cryptodev_load=".*"|cryptodev_load="NO"|g' "${cdroot}"/boot/loader.conf
    rm -f "${cdroot}"/boot/loader.conf-e
    sed -i -e 's|tmpfs_load=".*"|tmpfs_load="YES"|g' "${cdroot}"/boot/loader.conf
    rm -f "${cdroot}"/boot/loader.conf-e
  else
    echo "Major version >= 13, hence using cryptodev.ko"
    sed -i -e 's|cryptodev_load=".*"|cryptodev_load="YES"|g' "${cdroot}"/boot/loader.conf
    rm -f "${cdroot}"/boot/loader.conf-e
    sed -i -e 's|opensolaris_load=".*"|opensolaris_load="NO"|g' "${cdroot}"/boot/loader.conf
    rm -f "${cdroot}"/boot/loader.conf-e
    sed -i -e 's|tmpfs_load=".*"|tmpfs_load="NO"|g' "${cdroot}"/boot/loader.conf
    rm -f "${cdroot}"/boot/loader.conf-e
  fi
  sync ### Needed?
}

tag()
{
  if [ -n "$CIRRUS_CI" ] ; then
    SHA=$(echo "${CIRRUS_CHANGE_IN_REPO}" | head -c 7)
    URL="https://${CIRRUS_REPO_CLONE_HOST}/${CIRRUS_REPO_FULL_NAME}/commit/${SHA}"
    echo "${URL}"
    echo "${URL}" > "${cdroot}/.url"
    echo "${URL}" > "${uzip}/.url"
    echo "Setting extended attributes 'url' and 'sha' on '/.url'"
    setextattr user sha "${SHA}" "${uzip}/.url"
    setextattr user url "${URL}" "${uzip}/.url"
    setextattr user build "${BUILDNUMBER}" "${uzip}/.url"
  fi
}

image()
{
  sh "${cwd}/scripts/mkisoimages-${arch}.sh" -b "${label}" "${isopath}" "${cdroot}"
  sync ### Needed?
  md5 "${isopath}" > "${isopath}.md5"
  echo "$isopath created"
}

split()
{
  # units -o "%0.f" -t "2 gigabytes" "bytes"
  THRESHOLD_BYTES=2147483647
  # THRESHOLD_BYTES=1999999999
  ISO_SIZE=$(stat -f%z "${isopath}")
  if [ $ISO_SIZE -gt $THRESHOLD_BYTES ] ; then
    echo "Size exceeds GitHub Releases file size limit; splitting the ISO"
    sudo split -d -b "$THRESHOLD_BYTES" -a 1 "${isopath}" "${isopath}.part"
    echo "Split the ISO, deleting the original"
    rm "${isopath}"
    ls -l "${isopath}"*
  fi
}

cleanup
workspace
repos
pkg
base
packages
initgfx
rc
user
dm
script
tag
uzip
ramdisk
boot
image

if [ -n "$CIRRUS_CI" ] ; then
  # On Cirrus CI we want to upload to GitHub Releases which has a 2 GB file size limit,
  # hence we need to split the ISO there if it is too large
  split
fi
