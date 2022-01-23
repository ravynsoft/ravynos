# airyxOS ISO builder [![Build Status](https://api.cirrus-ci.com/github/mszoek/ISO.svg)](https://cirrus-ci.com/github/mszoek/ISO)

This Live ISO builder builds Live ISOs for airyxOS. It is based on [furybsd-livecd](https://github.com/furybsd/furybsd-livecd/) by Joe Maloney and enhancements from [helloSystem](https://hellosystem.github.io).

## Continuous builds

Continuous builds can be downloaded [here](https://dl.airyx.org/nightly/). __CAUTION:__ These are meant for development only. Use at your own risk.

## System Requirements for live media

* 2 GHz dual core processor
* 4 GiB RAM (system memory for physical and viritualized installs)
* VGA capable of 1024x768 screen resolution 
* Either a CD/DVD drive or a USB port for booting the installer media

airyxOS also runs well in virtual environments. It has been successfully run in VMware Fusion, VirtualBox, Proxmox, QEMU/KVM, and bhyve.

## Credentials for live media

There is no password for `liveuser`. The `liveuser` account is removed upon install.  There is also no root password until it is set in the installer. You can become root using `sudo su`.

## Acknowledgements

Please see https://hellosystem.github.io/docs/developer/acknowledgements.
These builds would not be possible without the infrastructure generously provided by [Cirrus CI](https://cirrus-ci.com/).
