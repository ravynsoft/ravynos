# furybsd-livecd
LiveCD builder for FuryBSD

## Customize
Add more packages:
```
edit settings/packages.XFCE
```

Enable more services:
```
edit settings/rc.XFCE
```

## Build
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
./build.sh kde5
```

## Burn

Burn the image to cd:
```
pkg install crdtools
cdrecord /usr/local/furybsd/iso/FuryBSD-12.1-RC2-XFCE.iso
```

Write the image to usb stick:
```
sudo dd if=/dev/usr/local/furybsd/iso/FuryBSD-12.1-RC2-XFCE.iso of=/dev/da0 bs=4m
```

## Credentials for live media
liveuser: furybsd

root: furybsd
