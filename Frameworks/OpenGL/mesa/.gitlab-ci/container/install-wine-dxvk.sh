#!/usr/bin/env bash

set -e

overrideDll() {
  if ! wine reg add 'HKEY_CURRENT_USER\Software\Wine\DllOverrides' /v "$1" /d native /f; then
    echo -e "Failed to add override for $1"
    exit 1
  fi
}

dxvk_install_release() {
    local DXVK_VERSION=${1:?}

    curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
	-O "https://github.com/doitsujin/dxvk/releases/download/v${DXVK_VERSION}/dxvk-${DXVK_VERSION}.tar.gz"
    tar xzpf dxvk-"${DXVK_VERSION}".tar.gz
    cp "dxvk-${DXVK_VERSION}"/x64/*.dll "$WINEPREFIX/drive_c/windows/system32/"
    overrideDll d3d9
    overrideDll d3d10core
    overrideDll d3d11
    overrideDll dxgi
    rm -rf "dxvk-${DXVK_VERSION}"
    rm dxvk-"${DXVK_VERSION}".tar.gz
}

dxvk_install_release "2.1"
