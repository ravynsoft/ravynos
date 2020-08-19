# furybsd-livecd
LiveCD builder for FuryBSD

## System Requirements for building LiveCD

* 2 GHz dual core processor
* 8 GiB RAM (system memory)
* 50 GB of hard-drive space
* Either a CD-RW/DVD-RW drive or a USB port for writing the installer media
* FreeBSD 12.1-RELEASE or later

## System Requirements for using LiveCD

* 2 GHz dual core processor
* 8 GiB RAM (system memory for physical and viritualized installs)
* VGA capable of 1024x768 screen resolution 
* Either a CD/DVD drive or a USB port for booting the installer media

## Install packages required for build system

```
pkg install git
```

## Customize (optional)
Add more packages to XFCE edition:
```
edit settings/packages.xfce
```

Enable more services:
```
edit settings/rc.conf.xfce
```

## Build a new release 
Generate an ISO with XFCE:
```
./build.sh
```
Generate an ISO with Gnome3:
```
./build.sh gnome
```
Generate an ISO with KDE Plasma 5:
```
./build.sh kde
```

## Burn

Burn the XFCE image to cd:
```
pkg install cdrtools
cdrecord /dev/usr/local/furybsd/iso/FuryBSD-12.1-XFCE.iso
```

Write the XFCE image to usb stick:
```
sudo dd if=/dev/usr/local/furybsd/iso/FuryBSD-12.1-XFCE.iso of=/dev/da0 bs=4m
```

## Credentials for live media
The username for the livecd is `liveuser`.  The password for `liveuser` is `furybsd`.  The `liveuser` account is removed upon install.  The root user password is also `furybsd` until it is changed in the installer.

## Push tags to github (release engineering only)
```
./tagports.sh
```

## Build a release from tags (release engineering only)

```
./release.sh
```
