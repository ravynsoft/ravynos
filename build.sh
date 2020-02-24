#!/bin/sh

cwd="`realpath | sed 's|/scripts||g'`"
workdir="/usr/local"
livecd="${workdir}/furybsd"
cache="${livecd}/cache"
version="12.0"
pkgset="branches/2020Q1"
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
tag=$2

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

# Make sure bash is installed
if [ ! -f "/usr/local/bin/bash" ] ; then
  echo "Bash is required"
  echo "Please install bash with pkg install bash first"
  exit 1
fi

# Make sure poudriere is installed
if [ ! -f "/usr/local/bin/poudriere" ] ; then
  echo "Poudriere is required"
  echo "Please install poudriere with pkg install poudriere or pkg install poudriere-devel first"
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

# Get the version tag
if [ -z "$2" ] ; then
  echo "tag not set"
  rm /usr/local/furybsd/tag >/dev/null 2>/dev/null
  export vol="FuryBSD-${version}-${edition}"
else
  echo "tag is set"
  rm /usr/local/furybsd/version >/dev/null 2>/dev/null
  echo "${2}" > /usr/local/furybsd/tag
  export vol="FuryBSD-${version}-${edition}-${tag}"
fi

label="FURYBSD"
isopath="${iso}/${vol}.iso"

jail()
{
  # Check if jail exists
  poudriere jail -l | grep -q furybsd
  if [ $? -eq 1 ] ; then
    # If jail does not exist create it
    poudriere jail -c -j furybsd -v ${version}-RELEASE -K GENERIC
  else
    # Update jail if it exists
    poudriere jail -u -j furybsd
  fi
}

ports()
{
  # Check if ports tree exists
  poudriere ports -l | grep -q furybsd
  if [ $? -eq 1 ] ; then
    # If ports tree does not exist create it
    poudriere ports -c -p furybsd-ports -B ${pkgset} -m git
  else
    # Update ports tree if it exists
    poudriere ports -u -p furybsd-ports -B ${pkgset} -m git
  fi
}

build()
{
  poudriere bulk -j furybsd -p furybsd-ports -f ${cwd}/settings/packages.${desktop}
}
image()
{
  poudriere image -t iso -j furybsd -s 4g -p furybsd-ports -h furybsd -n ${vol} -f ${cwd}/settings/packages.${desktop}
}

jail
ports
build
image
