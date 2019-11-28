#!/bin/sh

# This script will build FuryBSD iso from latest github tag for furybsd-wallpapers

# Only run as superuser
if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root" 1>&2
  exit 1
fi

cwd="`realpath | sed 's|/scripts||g'`"
gitcheck=$(git ls-remote --tags https://github.com/furybsd/furybsd-wallpapers | awk '{ print $2}' | cut -d'/' -f3 | head -n 1)

rm /usr/local/furybsd/iso/FuryBSD-12.0-XFCE-${gitcheck}.iso || true
cd ${cwd} && ./build.sh xfce ${gitcheck}
md5 /usr/local/furybsd/iso/FuryBSD-12.0-XFCE-${gitcheck}.iso > /usr/local/furybsd/iso/FuryBSD-12.0-XFCE-${gitcheck}.iso.md5
rm /usr/local/furybsd/tag || true
