# furybsd-livecd
LiveCD builder for FuryBSD

## Recommend System Requirements

* FreeBSD 12.0-RELEASE or later
* 64 GB memory
* 32 cores
* ZFS on root installation using pool name zroot with at least 100GB free
* 2 GB swap or greater

Note other configurations may work but have not been qualified

## Install packages required for build system

```
pkg install bash
pkg install git
pkg install poudriere
```

## Configure poudriere

Edit poudriere default configuration:

```
edit /usr/local/etc/poudriere.conf
```

Define to the pool to be used for building packages:

```
ZPOOL=zroot
```

Define the local path for creating jails, ports trees:

```
BASEFS=/zroot/poudriere
```

Add parameter to prevent llvm, webkit, rust and others from taking hours:

```
ALLOW_MAKE_JOBS_PACKAGES="pkg ccache py* llvm* rust* node* firefox* webkit*"
```

Save configuration then fetch FreeBSD ports for building ports:

```
git clone -b branches/2020Q1 https://github.com/freebsd/freebsd-ports.git --depth=1 /usr/ports
```
Note this will go away when production poudriere supports overlay.  For now fetching a ports tree in /usr/ports is required to overlay our custom furybsd-ports.  Poudriere will mount this custom tree with nullfs.

Clone the furybsd-ports overlay

```
git clone https://github.com/furybsd/furybsd-ports.git
```

Install the ports overlays for furybsd ports

```
./mkport.sh x11-drivers/furybsd-xorg-tool
./mkport.sh x11-themes/furybsd-wallpapers
./mkport.sh x11/furybsd-common-settings
./mkport.sh x11/furybsd-gnome-desktop
./mkport.sh x11/furybsd-kde-desktop
./mkport.sh x11/furybsd-xfce-desktop
./mkport.sh x11/furybsd-xfce-settings
```

## Install nginx to monitor ports build (recommended)

Install the nginx package:

```
pkg install nginx
```

Edit the default configuration:

```
edit /usr/local/etc/nginx.conf
```

Set root parameter, add data alias, and enable autoindex:

```
    server {
        listen       80;
        server_name  localhost;
        root         /usr/local/share/poudriere/html;

        #charset koi8-r;

        #access_log  logs/host.access.log  main;

        location /data {
            alias /data/logs/bulk;
            autoindex on;
        }
```

Save configuration then enable nginx service:

```
sysrc nginx_enable="YES"
```

Start nginx service:

```
service nginx start
```

## Customize
Add more packages to XFCE edition:
```
edit settings/packages.xfce
```

Enable more services:
```
edit settings/rc.conf.xfce
```

## Build a new release (required to push tags)
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

## Push tags to github (release engineering only)
```
git clone git@github.com:furybsd/furybsd-ports.git
```

```
cd furybsd-ports && ./tagports.sh
```

## Build a release from tags (always uses the newest)

```
./release.sh
```

## Burn

Burn the XFCE image to cd:
```
pkg install cdrtools
cdrecord /data/images/FuryBSD-12.0-XFCE.iso
```

Write the XFCE image to usb stick:
```
sudo dd if=/data/images/FuryBSD-12.0-XFCE.iso of=/dev/da0 bs=4m
```

## Credentials for live media
liveuser: furybsd

root: furybsd
