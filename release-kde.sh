#!/bin/sh

# This script will build FuryBSD iso from latest github tag for furybsd-wallpapers

# Only run as superuser
if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root" 1>&2
  exit 1
fi

cwd="`realpath | sed 's|/scripts||g'`"
gitcheck=$(git ls-remote --tags https://github.com/furybsd/furybsd-wallpapers | awk '{ print $2}' | cut -d'/' -f3 | sed 's/[^a-zA-Z0-9]//g' | tail -n 1)

rm /usr/local/furybsd/iso/FuryBSD-12.0-KDE-${gitcheck}.iso || true
cd ${cwd} && ./build.sh kde ${gitcheck}
md5 /usr/local/furybsd/iso/FuryBSD-12.0-KDE-${gitcheck}.iso > /usr/local/furybsd/iso/FuryBSD-12.0-KDE-${gitcheck}.iso.md5
