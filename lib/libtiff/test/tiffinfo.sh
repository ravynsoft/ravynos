#!/bin/sh
#
# Basic sanity check for tiffinfo.
#
. ${srcdir:-.}/common.sh
f_test_reader "${TIFFINFO} -c -D -d -j -s " "${IMG_MINISBLACK_1C_16B}"
f_test_reader "${TIFFINFO}  " "${IMG_TIFF_WITH_SUBIFD_CHAIN}"
