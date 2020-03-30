# furybsd-livecd
LiveCD builder for FuryBSD

## Recommend System Requirements

* FreeBSD 12.1-RELEASE or later
* 4 GB memory
* ZFS on root installation using pool name zroot with at least 50GB free

Note other configurations may work but have not been qualified

## Install packages required for build system

```
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

Save configuration then make distfiles location for building ports:

```
zfs create zroot/usr/ports/distfiles
```

## Install nginx to monitor ports build (optional)

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
