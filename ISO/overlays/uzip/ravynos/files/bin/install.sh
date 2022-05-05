#!/bin/sh

pool=ravynOS
geom=$1
username=$2

usage() {
    echo $(basename $0) '<geom> <username>'
    echo -e \\t geom = a disk device, such as da0
    echo -e \\t username = your desired username
}

deletePartitions() {
    for part in $(/sbin/gpart list $geom|grep '   index:'|cut -c10-); do
        echo /sbin/gpart delete -i $part $geom
    done
}

createPartitions() {
    /sbin/gpart destroy $geom
    /sbin/gpart create -s gpt $geom
    /sbin/gpart add -t efi -s 1m -l efi $geom
    /sbin/newfs_msdos /dev/${geom}p1
    /sbin/gpart add -t freebsd-swap -l swap -a 1m -s 4096m $geom
    /sbin/gpart add -t freebsd-zfs -l $pool -a 1m $geom
}

createPools() {
    mkdir /tmp/pool
    /sbin/zpool create -f -R /tmp/pool -O mountpoint=/ -O atime=off -O canmount=off -O compression=on $pool ${geom}p3
    /sbin/zpool create -o canmount=off -o mountpoint=none ${pool}/ROOT
    /sbin/zpool create -o mountpoint=/ ${pool}/ROOT/default

    /sbin/zfs create -o canmount=off ${pool}/usr
    /sbin/zfs create -o canmount=off ${pool}/var
    for vol in "/Users" "/usr/local" "/usr/obj" "/usr/src" "/usr/ports" "/usr/ports/distfiles" "/tmp" "/var/jail" "/var/log" "/var/tmp"; do
        /sbin/zfs create ${pool}$vol
    done

    /sbin/zpool set bootfs=${pool}/ROOT/default ${pool}
}

initializeEFI() {
    mkdir /tmp/efi
    /sbin/mount_msdosfs /dev/${geom}p1 /tmp/efi
    mkdir /tmp/efi/efi
    mkdir /tmp/efi/efi/boot
    cp -f /boot/loader.efi /tmp/efi/efi/boot/bootx64.efi
    unmount /tmp/efi
}

copyFilesystem() {
    cat > /tmp/excludes <<EOT
/dev
/proc
/tmp
/Applications/Utilities/Install ravynOS.app
/bin/install.sh
EOT
    echo Filling the pool
    /usr/bin/cpdup -uIof -X/tmp/excludes / /tmp/pool

    export BSDINSTALL_CHROOT=/tmp/pool
    /usr/sbin/bsdinstall config
    /usr/sbin/bsdinstall entropy
    /usr/sbin/pw -R /tmp/pool usermod -n root -h -
    /usr/sbin/pw -R /tmp/pool userdel -n liveuser
    /usr/sbin/pw -R /tmp/pool groupdel -n liveuser
    /bin/rm -rf /tmp/pool/Users/liveuser

    /usr/sbin/pkg -c /tmp/pool remove -y furybsd-live-settings
    /usr/sbin/pkg -c /tmp/pool remove -y freebsd-installer

    echo Updating rc.conf
    rm /tmp/pool/etc/rc.conf.local
    for entry in $(cat /etc/rc.conf.local) 'root_rw_mount="YES"' 'zfs_enable="YES"' 'zfsd_enable="YES"' 'hostname="ravynOS"'; do
        echo $entry | /usr/bin/xargs /usr/sbin/sysrc -f /tmp/pool/etc/rc.conf
    done

    rm -f /tmp/pool/var/initgfx_config.id

    echo Configuring loader
    cat >>/tmp/pool/boot/loader.conf <<EOT
opensolaris_load="YES"
zfs_load="YES"
mach_load="YES"
init_path="/sbin/launchd"
#boot_mute="YES"
beastie_disable="YES"
autoboot_delay="3"
hw.psm.elantech_support="1"
hw.psm.synaptics_support="1"
vfs.root.mountfrom.options="rw""
vfs.root.mountfrom="zfs:${pool}/ROOT/default"
EOT

    userinfo="%${username}::::::${username}:/Users/${username}:/usr/bin/zsh:"
    echo "$userinfo" | chroot /tmp/pool /usr/sbin/adduser -f -
    chroot /tmp/pool passwd $username
    for group in wheel video webcamd; do
            chroot /tmp/pool /usr/sbin/pw groupmod $group -m $username
    done
    chmod 1777 /tmp/pool/tmp
}

if [ -z "$username" ] || [ -z "$geom" ]; then
    usage
    exit 1
fi

deletePartitions
createPartitions
createPools
initializeEFI
copyFilesystem

exit 0

