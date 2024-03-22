#!/usr/bin/env bash

if test -f /etc/debian_version; then
    apt-get autoremove -y --purge
fi

# Clean up any build cache
rm -rf /root/.cache
rm -rf /root/.cargo
rm -rf /.cargo

if test -x /usr/bin/ccache; then
    ccache --show-stats
fi
