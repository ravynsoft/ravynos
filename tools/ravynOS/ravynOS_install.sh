#!/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo -e "\033[0;31mravynOS Installer must be run with sudo.\033[0m"
   exit 1
fi

# ANSI escape codes for styling
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
WHITE='\033[0;97m'
RESET='\033[0m'

# Beautiful welcome message
echo -e "${CYAN}"
echo -e "=========================================="
echo -e "  ${GREEN}Welcome to${CYAN} ravynOS ${GREEN}Setup for Developers${RESET}"
echo -e "=========================================="
echo -e "${YELLOW}Preparing your environment...${RESET}"
echo -e "${CYAN}------------------------------------------${RESET}"
echo -e "${RED}WARNING:${RESET} Before proceeding, make sure to run the command ${YELLOW}gpart destroy -F${RESET} (e.g., for devices like ${CYAN}ada0, da0, etc.${RESET})."

# Prompt the user for the disk device name
read -p "Enter the disk device name (e.g., ada0, da0, etc.): " device

# Confirm the input
echo "You selected the device: $device"
read -p "Continue? (y/n): " confirm
if [[ "$confirm" != "y" ]]; then
    echo "Installation aborted."
    exit 1
fi

# Delete existing partitions and create a new GPT
echo "Preparing the device..."
gpart create -s gpt $device

# Create an EFI partition
echo "Creating EFI partition..."
gpart add -t efi -l efi -s 256M $device
newfs_msdos /dev/${device}p1

# Create a swap partition
echo "Creating swap partition..."
gpart add -t freebsd-swap -s 4G $device

# Create a ZFS partition
echo "Creating ZFS partition..."
gpart add -t freebsd-zfs -l ravynOS $device

# Initialize the ZFS pool
echo "Initializing ZFS pool..."
zpool create -f -R /mnt -O mountpoint=/ -O atime=off -O canmount=off -O compression=on ravynOS /dev/${device}p3

# Create ZFS datasets
echo "Creating ZFS datasets..."
zfs create -o canmount=off -o mountpoint=none ravynOS/ROOT
zfs create -o mountpoint=/ ravynOS/ROOT/default

# Prepare EFI boot files
echo "Preparing EFI boot files..."
zpool set bootfs=ravynOS/ROOT/default ravynOS
mkdir /tmp/efi
mount -t msdosfs /dev/${device}p1 /tmp/efi
mkdir -p /tmp/efi/efi/boot
cp /boot/loader.efi /tmp/efi/efi/boot/bootx64.efi
cp /boot/loader.efi /tmp/efi/efi/boot/loader.efi
umount /tmp/efi

# Exclude unnecessary directories
cat >> /tmp/excludes <<EOF
/dev
/proc
/tmp
EOF

echo "Installing ravynOS... This process may take several minutes, please wait."
echo "Relax and have a coffee while ravynOS is installing on your computer."
cd /sysroot
cpdup -uIof -X /tmp/excludes . /mnt

# Enter the chroot environment
chroot /mnt /bin/sh <<'EOF'

# Perform configuration
/usr/sbin/bsdinstall config
/usr/sbin/bsdinstall entropy

# Remove temporary user liveuser
/usr/sbin/pw userdel -n liveuser
/usr/sbin/pw groupdel -n liveuser
rm -rf /Users/liveuser

# Create a new user 'dev' with password 'temp'
echo "Creating user dev with password temp..."
/usr/sbin/pw useradd -n dev -s /bin/sh -m -G wheel -h 0 <<'USER_PASS'
temp
USER_PASS

# Move rc.conf.local file
mv /etc/rc.conf.local /etc/rc.conf

# Configure /etc/rc.conf using sysrc
echo "Configuring ravynOS..."
sysrc zfs_enable=YES
sysrc zfsd_enable=YES

# Configure /boot/loader.conf
echo "Configuring ravynOS bootloader..."
cat <<'LOADER_CONF' >> /boot/loader.conf
cryptodev_load="YES"
zfs_load="YES"
beastie_disable="YES"
autoboot_delay="3"
vfs.root.mountfrom.options="rw"
vfs.root.mountfrom="zfs:ravynOS/ROOT/default"
LOADER_CONF

# Set the timezone
echo "Setting the timezone..."
cp /usr/share/zoneinfo/UTC /etc/localtime
EOF

# Finish installation
echo -e "${YELLOW}Attention!${RESET} User: ${CYAN}dev${RESET}. Password: ${CYAN}temp${RESET}. Please change the password after rebooting using ${CYAN}passwd dev${RESET}."
echo -e "Installation completed."

# Prompt the user to reboot
read -p "Would you like to reboot now? (y/N): " reboot_choice
if [ "$reboot_choice" = "y" ] || [ "$reboot_choice" = "Y" ]; then
    echo "Rebooting..."
    reboot
else
    echo "Reboot canceled. Please reboot manually when ready."
fi
