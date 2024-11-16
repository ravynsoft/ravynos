#!/bin/bash

# Check for root privileges
if [[ $EUID -ne 0 ]]; then
   echo -e "\033[38;5;9mThe ravynOS Installer must be run with sudo.\033[0m"
   exit 1
fi

# ANSI escape codes for "Space Gray" styling
GRAY='\033[38;5;246m'
DARK_GRAY='\033[38;5;236m'
WHITE='\033[38;5;255m'
BLUE='\033[38;5;81m'
CYAN='\033[38;5;51m'
GREEN='\033[38;5;82m'
RED='\033[38;5;196m'
YELLOW='\033[38;5;226m'
RESET='\033[0m'

show_disks_and_partitions() {
    echo -e "${CYAN}Available Disks and Partitions:${RESET}"
    echo -e "${GRAY}------------------------------------------${RESET}"

    echo -e "DISKS:"
    # Применяем ${GRAY} для всего вывода команды geom disk list
    geom disk list | while IFS= read -r line; do
        echo -e "${GRAY}$line${RESET}"
    done

    echo -e ""
    # Используем ${WHITE} и ${GRAY} для стилизации вывода gpart show
    gpart show | awk 'BEGIN {print "'${WHITE}'PARTITIONS:'${RESET}'"} 
        {if ($0 ~ /^[0-9]/) {print "'${GREEN}'" $0 "'${RESET}'"} 
        else {print "'${GRAY}'" $0 "'${RESET}'"}}'

    echo -e "${GRAY}------------------------------------------${RESET}"
}

echo -e "${CYAN}"
echo -e "=========================================="
echo -e "    ${WHITE}Welcome to${CYAN} ravynOS ${WHITE}Setup for Developers${RESET}"
echo -e "=========================================="

# Show disks and partitions for reference
show_disks_and_partitions

# Prompt for the disk device name
echo -e "${CYAN}Enter the disk device name (e.g., ada0, da0, etc.): ${RESET}"
read device
confirm_msg="${GRAY}You selected the device:${CYAN} $device${RESET}"

# Check for existing ravynOS partition on the selected disk
echo -e "${CYAN}Checking for existing ravynOS partition on $device...${RESET}"
existing_partition=$(gpart show -l $device 2>/dev/null | grep -o 'ravynOS')

if [[ -n "$existing_partition" ]]; then
    echo -e "${RED}ravynOS partition already exists on this disk. Only a fresh installation is allowed.${RESET}"
    echo -e "${WHITE}1)${GRAY} [SELECTED] Fresh installation (wipes disk)${RESET}"
    install_mode=1
else
    echo -e "${CYAN}No existing ravynOS partition found on $device.${RESET}"

    # Mode selection
    echo -e "${GRAY}Select installation mode:${RESET}"
    echo -e "${WHITE}1)${GRAY} Fresh installation (wipes disk)${RESET}"
    echo -e "${WHITE}2)${GRAY} Dual-boot installation (uses existing EFI partition)${RESET}"
    echo -e "${CYAN}Enter your choice (1/2): ${RESET}"
    read install_mode
fi

# Validate mode selection
if [[ "$install_mode" -ne 1 && "$install_mode" -ne 2 ]]; then
    echo -e "${RED}Invalid selection. Exiting.${RESET}"
    exit 1
fi

# Confirm the selection
echo -e "$confirm_msg"
echo -e "${YELLOW}Continue? (y/n): ${RESET})"
read confirm
if [[ "$confirm" != "y" ]]; then
    echo -e "${RED}Installation aborted.${RESET}"
    exit 1
fi

# Prepare the disk or partition
if [[ "$install_mode" -eq 1 ]]; then
    echo -e "${GRAY}Preparing the device...${RESET}"
    gpart destroy -F $device &>/dev/null || true
    gpart create -s gpt $device
    echo -e "${GRAY}Creating EFI partition...${RESET}"
    gpart add -t efi -l efi -s 256M $device
    newfs_msdos /dev/${device}p1
    efi_partition="${device}p1"
fi

# Create a swap partition and a ZFS partition
echo -e "${GRAY}Creating swap partition...${RESET}"
gpart add -t freebsd-swap -s 4G $device
echo -e "${GRAY}Creating ZFS partition...${RESET}"
gpart add -t freebsd-zfs -l ravynOS $device

# Initialize the ZFS pool
echo -e "${GRAY}Initializing ZFS pool...${RESET}"
zpool create -f -R /mnt -O mountpoint=/ -O atime=off -O canmount=off -O compression=on ravynOS /dev/${device}p3

# Create ZFS datasets
echo -e "${GRAY}Creating ZFS datasets...${RESET}"
zfs create -o canmount=off -o mountpoint=none ravynOS/ROOT
zfs create -o mountpoint=/ ravynOS/ROOT/default

#Set BOOTFS
zpool set bootfs=ravynOS/ROOT/default ravynOS

# Mount the EFI partition
echo -e "${GRAY}Preparing EFI boot files...${RESET}"
mkdir /tmp/efi
mount -t msdosfs /dev/$efi_partition /tmp/efi

# Add ravynOS bootloader
mkdir -p /tmp/efi/EFI/RAVYNOS
cp /boot/loader.efi /tmp/efi/EFI/RAVYNOS/bootx64.efi

# Check for existing bootloader entries
if [[ "$install_mode" -eq 2 ]]; then
    echo -e "${GRAY}Adding ravynOS to the existing EFI partition...${RESET}"
    
    # Extract the EFI partition number using awk instead of grep -P
    efi_partition=$(gpart show $device | awk '/efi/ {print $3}')
    
    # Ensure we got a valid partition number
    if [[ -z "$efi_partition" ]]; then
        echo -e "${RED}Error: EFI partition not found on $device.${RESET}"
        exit 1
    fi
    
    efibootmgr -c -d /dev/$device -p $efi_partition -L "ravynOS" -l '/tmp/efi/EFI/RAVYNOS/bootx64.efi'
    efibootmgr -t 10
    echo -e "\033[0;32mTimeout of 10 seconds has been set.\033[0m"
    echo -e "To change it, use: \033[0;36mefibootmgr -t 5\033[0m"
else
    echo -e "${GRAY}Setting up new EFI boot entry...${RESET}"
    mkdir -p /tmp/efi/EFI/BOOT
    cp /boot/loader.efi /tmp/efi/EFI/BOOT/bootx64.efi
fi

# Clean up
umount /tmp/efi

echo "Installing ravynOS... This process may take several minutes, please wait."
echo "Relax and have a coffee while ravynOS is installing on your computer."

cat >> /tmp/excludes <<EOF
/dev
/proc
/tmp
EOF

cd /sysroot
cpdup -uIof -X /tmp/excludes . /mnt

# Enter the chroot environment
chroot /mnt /bin/sh <<'EOF'
/usr/sbin/bsdinstall config
/usr/sbin/bsdinstall entropy

/usr/sbin/pw userdel -n liveuser
/usr/sbin/pw groupdel -n liveuser
rm -rf /Users/liveuser

/usr/sbin/pw useradd -n dev -s /bin/sh -m -G wheel -h 0 <<'USER_PASS'
temp
USER_PASS

echo "Configuring ravynOS..."
sysrc zfs_enable=YES
sysrc zfsd_enable=YES
sysrc sshd_enable=YES

ssh-keygen -A >/dev/null 2>&1

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
cp /usr/share/zoneinfo/UTC /etc/localtime
EOF

# Finish installation
echo -e "${GREEN}Attention!${RESET} User: ${CYAN}dev${RESET}. Password: ${CYAN}temp${RESET}. Please change the password after rebooting using ${CYAN}passwd dev${RESET}."
echo -e "Installation completed."
echo -e "${WHITE}Reboot your system to start using ravynOS.${RESET}"

echo -e "${YELLOW}Would you like to reboot now? (y/N): ${RESET})"
read reboot_choice
if [ "$reboot_choice" = "y" ] || [ "$reboot_choice" = "Y" ]; then
    echo -e "${GRAY}Rebooting...${RESET}"
    reboot
else
    echo -e "${GRAY}Reboot canceled. Please reboot manually when ready.${RESET}"
fi
