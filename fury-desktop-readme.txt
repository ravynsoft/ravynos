## Setup Networking

# List all adapters seen in pciconf
pciconf -lv | grep -A1 -B3 network

# Check for ethernet devices regonized by a driver
ifconfig

# Check for wifi devices recognized by a driver
sysctl -b net.wlan.devices

# Configure network (Only when ifconfig lists a device)
bsdconfig netdev

# Configure wifi (Only when sysctl lists a device)
bsdconfig wifi

## Install and configure Graphics Drivers

# Procedure to nVidia driver 390
rm /etc/X11/xorg.conf
pkg install nvidia-driver
sysrc kld_list+="nvidia-modeset"
cp /home/liveuser/xorg.conf.d/driver-nvidia.conf /etc/X11/xorg.conf
service kld restart
killall Xorg

# Procedure to install latest intel driver
rm /etc/X11/xorg.conf
pkg install drm-fbsd12.0-kmod
sysrc kld_list+="/boot/modules/i915kms.ko"
service kld restart
killall Xorg

# Procedure to install virtualbox drivers
rm /etc/X11/xorg.conf
pkg install virtualbox-ose-additions
sysrc vboxguest_enable="YES"
sysrc vboxservice_enable="YES"
service vboxguest start
service vboxservice start
killall Xorg
