# furybsd-livecd
LiveCD builder for FuryBSD

## Customize
Add more packages:
```
edit settings/packages
```

Enable more services:
```
edit settings/rc
```

## Build
Build an ISO with packages:
```
./build.sh
```

## Burn

Burn the image to cd:
```
pkg install crdtools
cdrecord /usr/local/furybsd/iso/furybsd.iso
```

Write the image to usb stick:
```
sudo dd if=/dev/usr/local/furybsd/iso/furybsd.iso of=/dev/da0 bs=4m
```

## Credentials for live media
liveuser: furybsd

root: furybsd
