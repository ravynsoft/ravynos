# hello Live ISO builder [![Build Status](https://api.cirrus-ci.com/github/helloSystem/ISO.svg)](https://cirrus-ci.com/github/helloSystem/ISO)

This Live ISO builder builds Live ISOs from FreeBSD ingredients. It is based on [furybsd-livecd](https://github.com/furybsd/furybsd-livecd/) by Joe Maloney.

## Continuous builds

Continuous builds can be downloaded [here](../../releases/). __CAUTION:__ These are meant for development only. Use at your own risk. Do not use in production environments.

To minimize the amount of data when going from build to build, `.zsync` files are also provided. [More information](https://askubuntu.com/questions/54241/how-do-i-update-an-iso-with-zsync)

It is possible to directly download and write straight to a USB stick in one go. __Caution:__ This will OVERWRITE the entire contents of the USB stick.

```
root@FreeBSD:/ # umount /dev/daX*
root@FreeBSD:/ # curl -L "https://github.com/probonopd/furybsd-livecd/releases/download/continuous/...iso" | dd of=/dev/daX bs=4m
```

## System Requirements for live media

* 2 GHz dual core processor
* 4 GiB RAM (system memory for physical and viritualized installs)
* VGA capable of 1024x768 screen resolution 
* Either a CD/DVD drive or a USB port for booting the installer media

## Credentials for live media

There is no password for `liveuser`. The `liveuser` account is removed upon install.  There is also no root password until it is set in the installer.

## Acknowledgements

Please see https://hellosystem.github.io/docs/developer/acknowledgements.
These builds would not be possible without the infrastructure generously provided by [Cirrus CI](https://cirrus-ci.com/).
