#!/bin/bash
set -e -x
DSTDIR="${DSTROOT}/${PUBLIC_HEADERS_FOLDER_PATH}"
DSTDIR_PRIV="${DSTROOT}/${PRIVATE_HEADERS_FOLDER_PATH}"

install \
    -d \
    -g "$INSTALL_GROUP" \
    -o "$INSTALL_OWNER" \
    "$DSTDIR"

install \
    -g "$INSTALL_GROUP" \
    -m "$INSTALL_MODE_FLAG" \
    -o "$INSTALL_OWNER" \
    "$MODULEMAP_FILE" \
    "$DSTDIR"/module.modulemap

install \
    -g "$INSTALL_GROUP" \
    -m "$INSTALL_MODE_FLAG" \
    -o "$INSTALL_OWNER" \
    "$MODULEMAP_PRIVATE_FILE" \
    "$DSTDIR_PRIV"/module.modulemap
