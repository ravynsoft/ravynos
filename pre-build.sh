#!/bin/sh

# TODO: Convert into settings/script.hello

# Exit on errors
set -e

#################

rm -rf overlays/uzip/hello/files/
mkdir -p overlays/uzip/hello/files/

cat > overlays/uzip/hello/files/.hidden <<\EOF
bin
boot
compat
COPYRIGHT
dev
entropy
etc
home
lib
libexec
media
mnt
net
opt
proc
rescue
root
sbin
sys
tmp
usr
var
zroot
EOF

mkdir -p overlays/uzip/hello/files/System
mkdir -p overlays/uzip/hello/files/Applications

# Get wallpaper
# TODO: Also check
# https://papers.co/desktop/vg41-ribbon-abstract-art-blue-pattern/
# https://papers.co/desktop/vm16-abstract-blue-rhytm-pattern/
# wget "https://res.allmacwallpaper.com/get/iMac-21-inch-wallpapers/Minimalist-blue-1920x1080/1686-9.jpg" -O "${uzip}"/usr/local/share/slim/themes/default/background.jpg

#################

cd overlays/uzip/hello/files/System

# Filer
wget https://github.com/helloSystem/Filer/releases/download/continuous/Filer_FreeBSD.zip
unzip Filer_FreeBSD.zip
rm -f Filer_FreeBSD.zip

# QtPlugin
wget https://github.com/helloSystem/QtPlugin/releases/download/continuous/QtPlugin_FreeBSD.zip
unzip QtPlugin_FreeBSD.zip
cp -Rf QtPlugin/ /
rm -f QtPlugin_FreeBSD.zip
rm -rf QtPlugin/

# Menu
wget https://github.com/helloSystem/Menu/releases/download/continuous/Menu_FreeBSD.zip
unzip Menu_FreeBSD.zip
rm -f Menu_FreeBSD.zip

# Dock
wget https://github.com/helloSystem/Dock/releases/download/continuous/Dock_FreeBSD.zip
unzip Dock_FreeBSD.zip
rm -f Dock_FreeBSD.zip

cd -
