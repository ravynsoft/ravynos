## Note this readme is work in progress.  
## More notes to come in the future.
## Last updated 10-30-19 J.M.

# Check for recognized ethernet devices
ifconfig

# Check for recognized wifi devices
sysctl -b net.wlan.devices

# Load more modules for ethernet and wifi
cd /boot/kernel/ && kldload if*

# Configure network
bsdconfig netconfig

# Configure wifi
bsdconfig wifi

# Changing to nVidia driver 390
rm /etc/X11/xorg.conf
pkg install nvidia-driver
sysrc kld_list+="nvidia-modeset"
cp /home/liveuser/xorg.conf.d/nvidia-driver.conf /etc/X11/xorg.conf
service kld restart

# Changing to modern intel driver
rm /etc/X11/xorg.conf
pkg install drm-fbsd12.0-kmod
sysrc kld_list+="/boot/modules/i915kms.ko"


