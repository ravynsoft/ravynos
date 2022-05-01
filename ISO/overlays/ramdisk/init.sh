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

RAVYNOS_VERSION=$(head -1 /version)
RAVYNOS_CODENAME=$(tail -1 /version)
echo "Hello. This is ravynOS ${RAVYNOS_VERSION} (${RAVYNOS_CODENAME})" > /dev/tty

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
    [ -e "/dev/iso9660/RAVYNOS" ] && echo "found /dev/iso9660/RAVYNOS" && break
    sleep 1
done

echo "==> Mount /cdrom"
mount_cd9660 /dev/iso9660/RAVYNOS /cdrom

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
mkdir /root /mnt /media /Volumes /System /System/Library /boot /compat /compat/linux
for d in bin sbin lib libexec usr Applications Library; do
	ln -s /sysroot/$d /$d
done

echo "==> Populating etc and var"
mkdir -p /etc /var
mount -t tmpfs tmpfs /etc
mount -t tmpfs tmpfs /var
mount -t tmpfs tmpfs /boot
tar -C /sysroot/etc -cpf - . | tar -C /etc -xf -
tar -C /sysroot/var -cpf - . | tar -C /var -xf -
chmod 1777 /var/tmp

cat > /etc/bootstrap <<EOT
#!/rescue/sh

rm -f /var/run/nologin
EOT
chmod 755 /etc/bootstrap

mkdir /private
for d in dev etc var; do
    ln -sf /$d /private/$d
done

echo "==> Populating /System/Library"
CWD=$(pwd)
cd /sysroot/System/Library
for d in *; do
    ln -s "/sysroot/System/Library/$d" "/System/Library/$d"
done
rm -f /System/Library/LaunchDaemons
mkdir -p /System/Library/LaunchDaemons
ln -s /sysroot/System/Library/LaunchDaemons/com.apple.auditd.json /System/Library/LaunchDaemons/
ln -s /sysroot/System/Library/LaunchDaemons/com.apple.notifyd.json /System/Library/LaunchDaemons/
for tty in 0 1 2 3; do
    cat > /System/Library/LaunchDaemons/org.freebsd.ttyv${tty}.json <<EOT
{
	"Label": "org.freebsd.getty.ttyv${tty}",
	"ProgramArguments": [
		"/usr/libexec/getty",
		"Pc",
		"ttyv${tty}"
	],
	"RunAtLoad": true,
	"KeepAlive": true
}
EOT
done

cd /sysroot/boot
for d in *; do
    ln -s /sysroot/boot/$d /boot/$d
done
rm -f /boot/entropy
cd $CWD

echo "==> User directory"
mkdir -p /Users/liveuser
mount -t tmpfs tmpfs /Users/liveuser
tar -C /sysroot/Users/liveuser -cpf - . | tar -C /Users/liveuser -xf -

echo "==> Loading important modules"
for mod in ums utouch firewire; do
	echo -n "$mod "; kldload $mod
done

/usr/bin/furybsd-init-helper

echo "==> Exit ramdisk init.sh"
exit 0
