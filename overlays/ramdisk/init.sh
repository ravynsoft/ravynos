#!/rescue/sh

PATH="/rescue"

if [ "`ps -o command 1 | tail -n 1 | ( read c o; echo ${o} )`" = "-s" ]; then
	echo "==> Running in single-user mode"
	SINGLE_USER="true"
fi

echo "==> Remount rootfs as read-write"
mount -u -w /

echo "==> Make mountpoints"
mkdir -p /cdrom

echo "Waiting for FURYBSD media to initialize"
while : ; do
    [ -e "/dev/iso9660/FURYBSD" ] && echo "found /dev/iso9660/FURYBSD" && break
    sleep 1
done

echo "==> Mount cdrom"
mount_cd9660 /dev/iso9660/FURYBSD /cdrom
mdconfig -f /cdrom/data/system.uzip -u 1
zpool import furybsd -o readonly=on

if [ "$SINGLE_USER" = "true" ]; then
        echo "Starting interactive shell in temporary rootfs ..."
        exit 0
fi

# Ensure the system has more than enough memory for memdisk
 x=3163787264
 y=$(sysctl -n hw.physmem)
 echo "Required memory ${x} for memdisk"
 echo "Detected memory ${y} for memdisk"
 if [ $x -gt $y ] ; then 
  echo "FuryBSD requires 4GB of memory for memdisk, and operation!"
  echo "Type exit, and press enter after entering the rescue shell to power off."
  exit 1
 fi

echo "==> Mount swap-based memdisk"
mdconfig -a -t swap -s 3g -u 2 >/dev/null 2>/dev/null
gpart create -s GPT md2 >/dev/null 2>/dev/null
gpart add -t freebsd-zfs md2 >/dev/null 2>/dev/null
zpool create livecd /dev/md2p1 >/dev/null 2>/dev/null
zfs set compression=gzip livecd 
zfs set primarycache=none livecd

echo "==> Replicate system image to swap-based memdisk"
zfs send -c -e furybsd | dd status=progress bs=1M | zfs recv -F livecd

mount -t devfs devfs /livecd/dev
chroot /livecd /usr/local/bin/furybsd-init-helper

kenv init_shell="/rescue/sh"
exit 0
