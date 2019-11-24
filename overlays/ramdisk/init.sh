#!/rescue/sh

PATH="/rescue"

if [ "`ps -o command 1 | tail -n 1 | ( read c o; echo ${o} )`" = "-s" ]; then
	echo "==> Running in single-user mode"
	SINGLE_USER="true"
fi

echo "==> Remount rootfs as read-write"
mount -u -w /

echo "==> Make mountpoints"
mkdir -p /cdrom /memdisk /memusr /mnt /sysroot /usr /tmp

echo "Waiting for FURYBSD media to initialize"
while : ; do
    [ -e "/dev/iso9660/FURYBSD" ] && echo "found /dev/iso9660/FURYBSD" && break
    sleep 1
done

echo "==> Mount cdrom"
mount_cd9660 /dev/iso9660/FURYBSD /cdrom
mdmfs -P -F /cdrom/data/system.uzip -o ro md.uzip /sysroot

# Make room for backup in /tmp
mount -t tmpfs tmpfs /tmp

echo "==> Mount swap-based memdisk"
mdmfs -s 1024m md /memdisk || exit 1
dump -0f - /dev/md1.uzip | (cd /memdisk; restore -rf -)
rm /memdisk/restoresymtable

kenv vfs.root.mountfrom=ufs:/dev/md2
kenv init_script="/init-reroot.sh"

if [ "$SINGLE_USER" = "true" ]; then
	echo "Starting interactive shell in temporary rootfs ..."
	exit 0
fi

kenv init_shell="/rescue/sh"
exit 0
