#!/rescue/sh

PATH="/rescue"

if [ "`ps -o command 1 | tail -n 1 | ( read c o; echo ${o} )`" = "-s" ]; then
	echo "==> Running in single-user mode"
	SINGLE_USER="true"
	kenv boot_mute="NO"
fi

if [ "`ps -o command 1 | tail -n 1 | ( read c o; echo ${o} )`" = "-v" ]; then
	echo "==> Running in verbose mode"
	kenv boot_mute="NO"
fi

# Silence messages if boot_mute="YES" is set
if [ "$(kenv boot_mute)" = "YES" ] ; then
      exec 1>>/dev/null 2>&1
fi

set -x

AIRYX_VERSION=$(head -1 /version)
AIRYX_CODENAME=$(tail -1 /version)
echo "Hello. This is airyxOS ${AIRYX_VERSION} (${AIRYX_CODENAME})" > /dev/tty

echo "==> Ramdisk /init.sh running"

if [ "$SINGLE_USER" = "true" ]; then
	echo "Starting interactive shell before doing anything ..."
	sh
fi

echo "==> Remount rootfs as read-write"
mount -u -w /

echo "==> Make mountpoints"
mkdir -p /cdrom

echo "Waiting for Live media to appear"
while : ; do
    [ -e "/dev/iso9660/AIRYX" ] && echo "found /dev/iso9660/AIRYX" && break
    sleep 1
done

echo "==> Mount /cdrom"
mount_cd9660 /dev/iso9660/AIRYX /cdrom

echo "==> Configure md from system.uzip"
mdconfig -u 1 -f /cdrom/data/system.uzip

echo "==> Importing ZFS pool"
zpool import furybsd -o readonly=on

echo "Setting up the live environment..." > /dev/tty

echo "==> Mounting /tmp and /proc"
mkdir /tmp /proc
mount -t tmpfs tmpfs /tmp
chmod 1777 /tmp
mount -t procfs procfs /proc

echo "==> Creating root symlinks"
rmdir /bin /sbin /usr/libexec /usr
mkdir /root /mnt /media /Volumes
for d in bin sbin lib libexec boot usr Applications System Library; do
	echo -n "$d "; ln -s /sysroot/$d /$d
done

echo "==> Populating etc and var"
mkdir -p /etc /var
mount -t tmpfs tmpfs /etc
mount -t tmpfs tmpfs /var
tar -C /sysroot/etc -cpf - . | tar -C /etc -xf -
tar -C /sysroot/var -cpf - . | tar -C /var -xf -

echo "==> User directory"
mkdir -p /Users/liveuser
mount -t tmpfs tmpfs /Users/liveuser
tar -C /sysroot/Users/liveuser -cpf - . | tar -C /Users/liveuser -xf -

echo "==> Loading important modules"
for mod in ums utouch firewire; do
	echo -n "$mod "; kldload $mod
done

/usr/bin/furybsd-init-helper

if [ "$SINGLE_USER" = "true" ]; then
        echo "Starting interactive shell..."
        sh
fi

ps ax | tee /tmp/ps.txt

kenv init_path="/sbin/launchd"
kenv init_shell="/rescue/sh"
kenv init_script="/init.sh"
kenv init_chroot="/"

exec /sbin/launchd

echo "==> Exit ramdisk init.sh"
exit 0

