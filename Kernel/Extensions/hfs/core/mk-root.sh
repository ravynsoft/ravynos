#!/bin/bash

#  mk-root.sh
#  hfs
#
#  Created by Chris Suter on 5/3/15.
#

shopt -s nocasematch

set -e

if [[ "$SDKROOT" =~ macosx ]] ; then
  if [ ! "$KERNEL_PATH" ] ; then
    KERNEL_PATH=$SDKROOT/System/Library/Kernels/kernel.development
  fi

  EXTS_PATH="`dirname \"$KERNEL_PATH\"`"/../Extensions

  kextutil -no-load -t -k "$KERNEL_PATH" -no-authentication "$BUILT_PRODUCTS_DIR/HFSEncodings.kext" -d "$EXTS_PATH/System.kext"
  kextutil -no-load -t -k "$KERNEL_PATH" -no-authentication "$BUILT_PRODUCTS_DIR/HFS.kext" -d "$EXTS_PATH/System.kext" -d "$BUILT_PRODUCTS_DIR/HFSEncodings.kext"

  if [ "$XNU_PATH" ] ; then
    extra_args=(-C "$XNU_PATH/BUILD/dst" .)
  fi
  gnutar --owner 0 --group 0 --transform 's|^([^/]+.kext)|System/Library/Extensions/\1|x' -C "$BUILT_PRODUCTS_DIR" HFS.kext HFSEncodings.kext "${extra_args[@]}" -cjf "$BUILT_PRODUCTS_DIR/hfs-root.tbz"
  echo "Created $BUILT_PRODUCTS_DIR/hfs-root.tbz"
  ln -sf $BUILT_PRODUCTS_DIR/hfs-root.tbz /tmp/
else
  ~/bin/copy-kernel-cache-builder
  pushd /tmp/KernelCacheBuilder
  if [ "$XNU_PATH" ] ; then
    extra_args=(KERNEL_PATH="$XNU_DST_PATH")
    extra_kext_paths="$BUILT_PRODUCTS_DIR $XNU_PATH/BUILD/dst/System/Library/Extensions"
  else
    extra_kext_paths="$BUILT_PRODUCTS_DIR"
  fi
  env -i make TARGETS="$DEVICES" "${extra_args[@]}" BUILDS=development VERBOSE=YES SDKROOT=iphoneos.internal EXTRA_KEXT_PATHS="$BUILT_PRODUCTS_DIR $XNU_PATH/BUILD/dst/System/Library/Extensions" EXTRA_BUNDLES=com.apple.filesystems.hfs.kext 2> >(sed -E '/^.*duplicate BUNDLE_IDS$/d' 1>&2)
fi
