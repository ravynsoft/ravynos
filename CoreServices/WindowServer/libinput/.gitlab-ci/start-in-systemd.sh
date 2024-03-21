#!/usr/bin/env bash

set -x

systemd_target=basic.target
post_command="/usr/bin/systemctl exit \$EXIT_STATUS"

while [[ $# -gt 0 ]]; do
	case $1 in
		--debug-mode)
			shift
			systemd_target=multi-user.target
			post_command="echo you can now log in as root (no password) and then turn off by running \'/usr/bin/systemctl exit \$EXIT_STATUS\'"
			;;
		*)
			echo "Unknow commandline argument $1"
			exit 1
			;;
	esac
done

WORKDIR=${FDO_DISTRIBUTION_WORKINGDIR:-$PWD}
B2C_WORKDIR=${FDO_B2C_WORKDIR:-/app}

# remove root password for debugging
sed -i 's/root:!locked::/root:::/' /etc/shadow

# create a libinput test suite service
cat <<EOF > /etc/systemd/system/libinput-testsuite.service

[Unit]
Description=Libinput test suite
After=$systemd_target

[Service]
Type=simple
StandardOutput=journal+console
EnvironmentFile=$B2C_WORKDIR/.b2c_env
WorkingDirectory=$WORKDIR
ExecStart=$WORKDIR/.gitlab-ci/meson-build.sh --skip-setup --skip-build --run-test

# exit the container on termination
ExecStopPost=$post_command

[Install]
WantedBy=default.target
EOF

cat /etc/systemd/system/libinput-testsuite.service

# enable the service
systemctl enable libinput-testsuite.service

# disable some services we don't need in the CI
systemctl mask network-online.target
systemctl mask network-pre.target
systemctl mask timers.target
systemctl mask dnf-makecache.timer
systemctl mask systemd-logind.service
systemctl mask rpmdb-migrate.service
systemctl mask systemd-network-generator.service
systemctl mask cryptsetup-pre.target
systemctl mask cryptsetup.target

#change default target
systemctl set-default $systemd_target

# start the system
exec /usr/sbin/init
