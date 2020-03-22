#!/bin/sh

PATH="/rescue"

mount -u -w / >/dev/null 2>/dev/null
mkdir -p /cdrom /union /usr >/dev/null 2>/dev/null
mount_cd9660 /dev/iso9660/FURYBSD /cdrom >/dev/null 2>/dev/null
mdmfs -P -F /cdrom/data/usr.uzip -o ro md.uzip /usr >/dev/null 2>/dev/null
mdmfs -s 512m md /union >/dev/null 2>/dev/null
mount -t unionfs /union /usr >/dev/null 2>/dev/null

if [ -d "/opt/local/bin" ] ; then
  /opt/local/bin/furybsd-init-helper
fi

kenv init_shell="/bin/sh" >/dev/null 2>/dev/null
exit 0
