#!/bin/sh
# Helper script which will tag github repos for ports / distfiles

# Only run as regular user with ssh keys
if [ "$(id -u)" = "0" ]; then
  echo "This script must be run by release engineer" 1>&2
  exit 1
fi

# Ask for authentication once
killall ssh-agent
eval $(ssh-agent)
ssh-add

repodir="/tmp/furybsd-tag"

rm -rf ${repodir}
mkdir -p ${repodir}

if [ ! -f "/usr/local/furybsd/version" ] ; then
  echo "FuryBSD must be built first"
  exit 1
fi

version=`cat /usr/local/furybsd/version`

git clone git@github.com:furybsd/furybsd-livecd.git ${repodir}/furybsd-livecd
git clone git@github.com:furybsd/furybsd-wifi-tool.git ${repodir}/furybsd-wifi-tool
git clone git@github.com:furybsd/furybsd-xorg-tool.git ${repodir}/furybsd-xorg-tool
git clone git@github.com:furybsd/furybsd-xfce-settings.git ${repodir}/furybsd-xfce-settings
git clone git@github.com:furybsd/furybsd-wallpapers.git ${repodir}/furybsd-wallpapers
git clone git@github.com:furybsd/furybsd-common-settings.git ${repodir}/furybsd-common-settings

cd ${repodir}/furybsd-livecd
git tag -a $version -m "{$version}"
git push origin --tags
cd ${repodir}/furybsd-wifi-tool
git tag -a $version -m "{$version}"
git push origin --tags
cd ${repodir}/furybsd-xorg-tool
git tag -a $version -m "{$version}"
git push origin --tags
cd ${repodir}/furybsd-xfce-settings
git tag -a $version -m "{$version}"
git push origin --tags
cd ${repodir}/furybsd-wallpapers
git tag -a $version -m "{$version}"
git push origin --tags
cd ${repodir}/furybsd-common-settings
git tag -a $version -m "{$version}"
git push origin --tags
rm -rf ${repodir}
