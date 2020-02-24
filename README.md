# furybsd-livecd
LiveCD builder for FuryBSD

## Recommend System Requirements

* FreeBSD 12.0-RELEASE or later
* 64 GB memory
* 32 cores
* ZFS on root installation using pool name zroot with at least 100GB free
* 2 GB swap or greater

Note other configurations may work but have not been qualified

## Install bash and git required for furybsd-ports

```
pkg install bash
pkg install git
pkg install poudriere
```

## Configure poudriere

```
edit /usr/local/etc/poudriere.conf
```

Set the following parameters:

```
ZPOOL=zroot
```

```
BASEFS=/zroot/poudriere
```

Make distfiles location

```
zfs create zroot/usr/ports/distfiles
```

## Install nginx to monitor ports build (recommended)

```
pkg install nginx
```

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
cdrecord /usr/local/furybsd/iso/FuryBSD-12.0-XFCE.iso
```

Write the XFCE image to usb stick:
```
sudo dd if=/dev/usr/local/furybsd/iso/FuryBSD-12.0-XFCE.iso of=/dev/da0 bs=4m
```

## Credentials for live media
liveuser: furybsd

root: furybsd
