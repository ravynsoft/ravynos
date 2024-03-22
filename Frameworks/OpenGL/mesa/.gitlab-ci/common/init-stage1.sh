#!/bin/sh

# Very early init, used to make sure devices and network are set up and
# reachable.

set -ex

cd /

findmnt --mountpoint /proc || mount -t proc none /proc
findmnt --mountpoint /sys || mount -t sysfs none /sys
mount -t debugfs none /sys/kernel/debug
findmnt --mountpoint /dev || mount -t devtmpfs none /dev
mkdir -p /dev/pts
mount -t devpts devpts /dev/pts
mkdir /dev/shm
mount -t tmpfs -o noexec,nodev,nosuid tmpfs /dev/shm
mount -t tmpfs tmpfs /tmp

echo "nameserver 8.8.8.8" > /etc/resolv.conf
[ -z "$NFS_SERVER_IP" ] || echo "$NFS_SERVER_IP caching-proxy" >> /etc/hosts

# Set the time so we can validate certificates before we fetch anything;
# however as not all DUTs have network, make this non-fatal.
for _ in 1 2 3; do sntp -sS pool.ntp.org && break || sleep 2; done || true
