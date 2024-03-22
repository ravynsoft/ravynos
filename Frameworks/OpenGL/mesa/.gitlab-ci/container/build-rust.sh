#!/bin/bash

# Note that this script is not actually "building" rust, but build- is the
# convention for the shared helpers for putting stuff in our containers.

set -ex

# cargo (and rustup) wants to store stuff in $HOME/.cargo, and binaries in
# $HOME/.cargo/bin.  Make bin a link to a public bin directory so the commands
# are just available to all build jobs.
mkdir -p "$HOME"/.cargo
ln -s /usr/local/bin "$HOME"/.cargo/bin

# Rusticl requires at least Rust 1.66.0 and NAK requires 1.73.0
#
# Also, pick a specific snapshot from rustup so the compiler doesn't drift on
# us.
RUST_VERSION=1.73.0-2023-10-05

# For rust in Mesa, we use rustup to install.  This lets us pick an arbitrary
# version of the compiler, rather than whatever the container's Debian comes
# with.
curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
    --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- \
      --default-toolchain $RUST_VERSION \
      --profile minimal \
      -y

rustup component add clippy rustfmt

# Set up a config script for cross compiling -- cargo needs your system cc for
# linking in cross builds, but doesn't know what you want to use for system cc.
cat > /root/.cargo/config <<EOF
[target.armv7-unknown-linux-gnueabihf]
linker = "arm-linux-gnueabihf-gcc"

[target.aarch64-unknown-linux-gnu]
linker = "aarch64-linux-gnu-gcc"
EOF
