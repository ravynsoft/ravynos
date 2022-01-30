#!/bin/sh

# Exit on errors
set -e

# BSD ABI
VER="13.0"
MAJOR="13"

if [ -x /usr/sbin/pkg ]; then
  PKG=/usr/sbin/pkg
elif [ -x /usr/local/sbin/pkg ]; then
  PKG=/usr/local/sbin/pkg
elif [ -x ${PKG} ]; then
  PKG=${PKG}
else
  echo Cannot locate pkg command
  exit 127
fi


if [ -z "${AIRYX}" ]; then
  AIRYX=$(pwd)/..
fi
AIRYXVER=$(head -1 ${AIRYX}/version.txt)
version=${AIRYXVER}

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
label="AIRYX"
export DISTRIBUTIONS="kernel.txz base.txz airyx.txz"

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
  mkdir -p ${workdir}/furybsd
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
      isopath="${iso}/${tag}-F${VER}_h${HELLO_VERSION}_${BUILDNUMBER}-${arch}.iso"
    else
      isopath="${iso}/${tag}-F${VER}_h${HELLO_VERSION}_git${SHA}-${arch}.iso"
    fi
  fi
fi

if [ "${desktop}" = "airyx" ]; then
    MAJLABEL="f${MAJOR}"
    if [ ! -z "${CIRRUS_BUILD_ID}" ]; then
      MAJLABEL="${MAJLABEL}_${CIRRUS_BUILD_ID}"
    fi
    isopath="${iso}/${tag}_${MAJLABEL}_${arch}.iso"
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
  truncate -s 4g "${livecd}/pool.img"
  mdconfig -f "${livecd}/pool.img" -u 0
  gpart create -s GPT md0
  gpart add -t freebsd-zfs md0
  sync ### Needed?
  zpool create furybsd /dev/md0p1
  sync ### Needed?
  zfs set mountpoint="${uzip}" furybsd
  # From FreeBSD 13 on, zstd can be used with zfs in base
  if [ $MAJOR -lt 13 ] ; then
    zfs set compression=gzip-6 furybsd 
  else
    zfs set compression=zstd-9 furybsd 
  fi

}

base()
{
  # TODO: Signature checking
  if [ ! -f "${base}/base.txz" ] ; then 
    cd ${base}
    fetch -o base.txz https://dl.cloudsmith.io/public/airyx/13_0/raw/names/base_main.txz/files/base.txz
  fi
  
  if [ ! -f "${base}/kernel.txz" ] ; then
    cd ${base}
    fetch -o kernel.txz https://dl.cloudsmith.io/public/airyx/13_0/raw/names/kernel_main.txz/files/kernel.txz
  fi

  if [ ! -f "${base}/airyx.txz" ] ; then
    cd ${base}
    fetch https://api.cirrus-ci.com/v1/artifact/github/mszoek/airyx/airyx/airyx/dist/airyx.txz
  fi
  cd ${base}
  tar -zxvf base.txz -C ${uzip}
  tar -zxvf kernel.txz -C ${uzip}
  tar -zxvf airyx.txz -C ${uzip}
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
      IGNORE_OSVERSION=yes ${PKG} -c "${uzip}" install -y $(${PKG} query -F ${uzip}${pkg_cachedir}/${pkg_cachesubdir}/${pkgfile} %dn)
      IGNORE_OSVERSION=yes $abi ${PKG} -c "${uzip}" add ${pkg_cachedir}/${pkg_cachesubdir}/${pkgfile}
      IGNORE_OSVERSION=yes ${PKG} -c "${uzip}" lock -y $(${PKG} query -F ${uzip}${pkg_cachedir}/${pkg_cachesubdir}/${pkgfile} %o)
}

packages()
{
  rm -f "${uzip}/etc/pkg/FreeBSD.conf"
  cp /etc/resolv.conf ${uzip}/etc/resolv.conf
  mkdir ${uzip}/var/cache/pkg
  mount_nullfs ${packages} ${uzip}/var/cache/pkg
  mount -t devfs devfs ${uzip}/dev
  # FIXME: In the following line, the hardcoded "i386" needs to be replaced by "${arch}" - how?
  for p in common-${MAJOR} ${desktop}; do
    sed '/^#/d;/\!i386/d;/^cirrus:/d;/^https:/d' "${cwd}/settings/packages.$p" | \
      xargs ${PKG} -c "${uzip}" install -y
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
    # FIXME: Is there something like "${PKG} add" that can be used
    # to install local packages (not from a repository) that will
    # resolve dependencies from the repositories?
    # The following will just fail in the case of unmet dependencies
    ${PKG} -r ${uzip} add "${packages}/transient/${p}" # pkg-static add has no -y
  done <"${packages}/transient/transient-packages-list"
  
  # ${PKG} -c ${uzip} info > "${cdroot}/data/system.uzip.manifest"
  # Manifest of installed packages ordered by size in bytes
  ${PKG} -c ${uzip} query "%sb\t%n\t%v\t%c" | sort -r -s -n -k 1,1 > "${cdroot}/data/system.uzip.manifest"
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
  mkdir -p ${uzip}/Users/liveuser/Desktop
  # chroot ${uzip} echo furybsd | chroot ${uzip} pw mod user root -h 0
  chroot ${uzip} pw useradd liveuser -u 1000 \
  -c "Live User" -d "/Users/liveuser" \
  -g wheel -G operator -m -s /usr/bin/zsh -k /usr/share/skel -w none
  chroot ${uzip} pw groupadd liveuser -g 1000
  # chroot ${uzip} echo furybsd | chroot ${uzip} pw mod user liveuser -h 0
  chroot ${uzip} chown -R 1000:1000 /Users/liveuser
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
  rm -f *.pkg # Make sure there are no leftover transient packages from earlier runs
  splashqml="${cwd}/overlays/uzip/airyx/files/usr/share/plasma/look-and-feel/Airyx/contents/splash/Splash.qml" 
  sed -e "s@__CODENAME__@${AIRYX_CODENAME}@" -e "s@__VERSION__@${AIRYX_VERSION}@" < "${splashqml}.in" > "${splashqml}"
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
    if [ $MAJOR -lt 14 ] ; then
      PKGS="quarterly"
    else
      PKGS="release_2"
    fi
	ver=470
        pkgfile='nvidia-driver-470.86.txz' #$(${PKG} -c ${uzip} rquery %n-%v.txz nvidia-driver${ver:+-$ver})
        fetch -o "${cache}/" "https://pkg.freebsd.org/FreeBSD:${MAJOR}:amd64/${PKGS}/All/${pkgfile}"
        mkdir -p "${uzip}/usr/local/nvidia/${ver:-440}/"
        tar xfC "${cache}"/${pkgfile} "${uzip}/usr/local/nvidia/${ver:-440}/"
	ls "${uzip}/usr/local/nvidia/${ver:-440}/+COMPACT_MANIFEST"
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
  zfs set mountpoint=/sysroot furybsd
  cd ${cwd} && zpool export furybsd && while zpool status furybsd >/dev/null; do :; done 2>/dev/null
  #makefs "${iso}/system.ufs" "${uzip}"
  mkuzip -S -d -o "${cdroot}/data/system.uzip" "${livecd}/pool.img" #"${iso}/system.ufs"
  #rm -f "${iso}/system.ufs"
}

ramdisk() 
{
  cp -R "${cwd}/overlays/ramdisk/" "${ramdisk_root}"
  sync ### Needed?
  cd ${cwd} && zpool import furybsd && zfs set mountpoint=${workdir}/furybsd/uzip furybsd
  sync ### Needed?
  cd "${uzip}" && tar -cf - rescue | tar -xf - -C "${ramdisk_root}"
  mkdir -p "${ramdisk_root}/bin" "${ramdisk_root}/sbin" "${ramdisk_root}/usr/libexec"
  #ln -sf "../rescue/rescue" "${ramdisk_root}/bin/launchctl"
  #ln -sf "../rescue/rescue" "${ramdisk_root}/usr/libexec/launchproxy"
  #cp -vf "${uzip}/sbin/launchd" "${ramdisk_root}/sbin"
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
  cd ${cwd} && zpool export furybsd && mdconfig -d -u 0
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

# if [ -n "$CIRRUS_CI" ] ; then
#   # On Cirrus CI we want to upload to GitHub Releases which has a 2 GB file size limit,
#   # hence we need to split the ISO there if it is too large
#   split
# fi
