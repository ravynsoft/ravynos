# furybsd-livecd
LiveCD builder for FuryBSD

## Recommend System Requirements

* FreeBSD 12.1-RELEASE or later
* 4 GB memory

Note other configurations may work but have not been qualified

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
liveuser: furybsd

root: furybsd

## Push tags to github (release engineering only)
```
./tagports.sh
```

## Build a release from tags (release engineering only)

```
./release.sh
```
