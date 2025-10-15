#!/bin/sh

# Exit on errors
set -e

if [ -z "${RAVYNOS}" ]; then
  RAVYNOS=$(pwd)/..
fi
RAVYNOS_VERSION=$(head -1 ${RAVYNOS}/version.txt)
RAVYNOS_CODENAME=$(tail -1 ${RAVYNOS}/version.txt)
version=${RAVYNOS_VERSION}

export cwd=$(realpath | sed 's|/scripts||g')
if [ -z "${workdir}" ]; then
  workdir="/usr/local"
fi
livecd="${workdir}/furybsd"
if [ -z "${arch}" ] ; then
  arch="$(uname -m)"
fi
cache="${livecd}/${arch}/cache"
base="${cache}/${version}/base"
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
label="RAVYNOS"
export DISTRIBUTIONS="kernel.txz base.txz ravynos.txz"

# Only run as superuser
if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root" 1>&2
  exit 1
fi

cleanup() {
  if [ -n "$CIRRUS_CI" ] ; then
      # On CI systems there is no reason to clean up which takes time
      return 0
  else
      umount ${uzip}/dev >/dev/null 2>/dev/null || true
      zpool destroy -f furybsd >/dev/null 2>/dev/null || true
      mdconfig -d -u 0 >/dev/null 2>/dev/null || true
      rm ${livecd}/pool.img >/dev/null 2>/dev/null || true
      rm -rf ${cdroot} >/dev/null 2>/dev/null || true
  fi
}

mkdir -p ${workdir}/furybsd
rm ${workdir}/furybsd/version >/dev/null 2>/dev/null || true
tag="ravynOS_${version}"
echo $tag > ${workdir}/furybsd/tag

# Get the short git SHA
SHA=$(echo ${CIRRUS_CHANGE_IN_REPO}| cut -c1-7)

CILABEL=""
if [ ! -z "${CIRRUS_BUILD_ID}" ]; then
    CILABEL="${CIRRUS_BUILD_ID}_"
fi
isopath="${iso}/${tag}_${CILABEL}${arch}.iso"

cleanup

### Set up the workspace
mkdir -p "${livecd}" "${base}" "${iso}" "${uzip}" "${ramdisk_root}/dev" "${ramdisk_root}/etc" >/dev/null 2>/dev/null
truncate -s 4g "${livecd}/pool.img"
mdconfig -f "${livecd}/pool.img" -u 0
gpart create -s GPT md0
gpart add -t freebsd-zfs md0
sync ### Needed?
zpool create furybsd /dev/md0p1
sync ### Needed?
zfs set mountpoint="${uzip}" furybsd
zfs set compression=zstd-9 furybsd 

### Extract the packages we built
tar -zxvf /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/base.txz -C ${uzip}
tar -zxvf /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/kernel.txz -C ${uzip}
tar -zxvf ${CIRRUS_WORKING_DIR}/dist/ravynOS.txz -C ${uzip}
touch ${uzip}/etc/fstab
rm ${uzip}/etc/resolv.conf
cp ${CIRRUS_WORKING_DIR}/resolv.conf ${uzip}/etc/resolv.conf
mkdir -p ${uzip}/dev
mount -t devfs devfs ${uzip}/dev

### Set up rc.conf
cp -fv "${cwd}/settings/rc.conf" "${uzip}/etc/rc.conf"

### Create the LiveCD user account
mkdir -p ${uzip}/Users/liveuser/Desktop
chroot ${uzip} pw useradd liveuser -u 1000 \
  -c "Live User" -d "/Users/liveuser" \
  -g wheel -G operator -m -s /bin/zsh -k /usr/share/skel -w none
chroot ${uzip} pw groupadd liveuser -g 1000
chroot ${uzip} chown -R 1000:1000 /Users/liveuser
chroot ${uzip} pw groupmod wheel -m liveuser
chroot ${uzip} pw groupmod video -m liveuser

### Install the overlays now
cd overlays/uzip
cp -rpvf * "${uzip}/"
cd ../..

if [ -e "${cwd}/settings/script.ravynOS" ] ; then
    "${cwd}/settings/script.ravynOS"
fi

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

### Create the uzip from our ZFS volume
umount "${uzip}/dev"
umount furybsd
install -o root -g wheel -m 755 -d "${cdroot}"
zfs set mountpoint=/sysroot furybsd
cd ${cwd} && zpool export furybsd && while zpool status furybsd >/dev/null; do :; done 2>/dev/null
mkuzip -S -d -o "${cdroot}/data/system.uzip" "${livecd}/pool.img"

### Build the ramdisk image
cp -R "${cwd}/overlays/ramdisk/" "${ramdisk_root}"
sync ### Needed?
cd ${cwd} && zpool import furybsd && zfs set mountpoint=${workdir}/furybsd/uzip furybsd
sync ### Needed?
cc --sysroot=${uzip} -static -o "${ramdisk_root}/raminit" \
    "${cwd}/raminit.c" -lutil
cd "${uzip}" && tar -cf - rescue | tar -xf - -C "${ramdisk_root}"
mkdir -p "${ramdisk_root}/etc"
touch "${ramdisk_root}/etc/fstab"
cp ${uzip}/etc/login.conf ${ramdisk_root}/etc/login.conf
makefs -b '10%' "${cdroot}/data/ramdisk.ufs" "${ramdisk_root}"
gzip -f "${cdroot}/data/ramdisk.ufs"
rm -rf "${ramdisk_root}"

### Set up the bootloader
cd "${uzip}" && tar -cf - boot | tar -xf - -C "${cdroot}"
cp -R "${cwd}/overlays/boot/" "${cdroot}"
cd ${cwd} && zpool export furybsd && mdconfig -d -u 0

### Finally, build the ISO image from the components
sh "${cwd}/scripts/mkisoimages-${arch}.sh" -b "${label}" "${isopath}" "${cdroot}"
sync ### Needed?
md5 "${isopath}" > "${isopath}.md5"
echo "$isopath created"

cleanup