#!/bin/sh

# aarch64_relocs.sh -- test AArch64 relocations.

# Copyright (C) 2016-2023 Free Software Foundation, Inc.
# Written by Igor Kudrin <ikudrin@accesssoftek.com>

# This file is part of gold.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.

check()
{
    file=$1
    lbl=$2
    line=$3
    pattern=$4

    found=`grep "<$lbl>:" $file`
    if test -z "$found"; then
        echo "Label $lbl not found."
        exit 1
    fi

    match_pattern=`grep "<$lbl>:" -A$line $file | tail -n 1 | grep -e "$pattern"`
    if test -z "$match_pattern"; then
        echo "Expected pattern did not found in line $line after label $lbl:"
        echo "    $pattern"
        echo ""
        echo "Extract:"
        grep "<$lbl>:" -A$line $file
        echo ""
        echo "Actual output below:"
        cat "$file"
        exit 1
    fi
}

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G0" 1 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x1234\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G0" 2 "\<R_AARCH64_MOVW_UABS_G0[[:space:]]\+abs_0x1234\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G0" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x1238\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G0" 4 "\<R_AARCH64_MOVW_UABS_G0[[:space:]]\+abs_0x1234+0x4\b"

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G0_NC" 1 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x1234\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G0_NC" 2 "\<R_AARCH64_MOVW_UABS_G0_NC[[:space:]]\+abs_0x1234\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G0_NC" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x6234\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G0_NC" 4 "\<R_AARCH64_MOVW_UABS_G0_NC[[:space:]]\+abs_0x1234+0x45000\b"

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1" 1 "\<movz[[:space:]]\+x4,[[:space:]]\+#0x0, lsl #16\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1" 2 "\<R_AARCH64_MOVW_UABS_G1[[:space:]]\+abs_0x1234-0x4\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x10000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1" 4 "\<R_AARCH64_MOVW_UABS_G1[[:space:]]\+abs_0x11000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1" 5 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x60000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1" 6 "\<R_AARCH64_MOVW_UABS_G1[[:space:]]\+abs_0x45000+0x20010\b"

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1_NC" 1 "\<movz[[:space:]]\+x4,[[:space:]]\+#0x0, lsl #16\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1_NC" 2 "\<R_AARCH64_MOVW_UABS_G1_NC[[:space:]]\+abs_0x1234-0x4\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1_NC" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x10000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1_NC" 4 "\<R_AARCH64_MOVW_UABS_G1_NC[[:space:]]\+abs_0x11000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1_NC" 5 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x60000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G1_NC" 6 "\<R_AARCH64_MOVW_UABS_G1_NC[[:space:]]\+abs_0x45000+0x100020010\b"

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G2" 1 "\<movz[[:space:]]\+x4,[[:space:]]\+#0x0, lsl #32\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G2" 2 "\<R_AARCH64_MOVW_UABS_G2[[:space:]]\+abs_0x45000+0x20010\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G2" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x3700000000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G2" 4 "\<R_AARCH64_MOVW_UABS_G2[[:space:]]\+abs_0x3600010000+0x100020010\b"

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G2_NC" 1 "\<movz[[:space:]]\+x4,[[:space:]]\+#0x0, lsl #32\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G2_NC" 2 "\<R_AARCH64_MOVW_UABS_G2_NC[[:space:]]\+abs_0x45000+0x20010\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G2_NC" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x3700000000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G2_NC" 4 "\<R_AARCH64_MOVW_UABS_G2_NC[[:space:]]\+abs_0x3600010000+0x3000100020010\b"

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G3" 1 "\<movz[[:space:]]\+x4,[[:space:]]\+#0x0, lsl #48\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G3" 2 "\<R_AARCH64_MOVW_UABS_G3[[:space:]]\+abs_0x3600010000+0x100020010\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G3" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x3000000000000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_UABS_G3" 4 "\<R_AARCH64_MOVW_UABS_G3[[:space:]]\+abs_0x3600010000+0x3000100020010\b"

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G0" 1 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x1238\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G0" 2 "\<R_AARCH64_MOVW_SABS_G0[[:space:]]\+abs_0x1234+0x4\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G0" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0xffffffffffffeeef\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G0" 4 "\<R_AARCH64_MOVW_SABS_G0[[:space:]]\+abs_0x1234-0x2345\b"

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G1" 1 "\<movn[[:space:]]\+x4,[[:space:]]\+#0x0, lsl #16\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G1" 2 "\<R_AARCH64_MOVW_SABS_G1[[:space:]]\+abs_0x1234-0x2345\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G1" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x60000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G1" 4 "\<R_AARCH64_MOVW_SABS_G1[[:space:]]\+abs_0x45000+0x20010\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G1" 5 "\<mov[[:space:]]\+x4,[[:space:]]\+#0xfffffffffffeffff\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G1" 6 "\<R_AARCH64_MOVW_SABS_G1[[:space:]]\+abs_0x45000-0x56000\b"

check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G2" 1 "\<movz[[:space:]]\+x4,[[:space:]]\+#0x0, lsl #32\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G2" 2 "\<R_AARCH64_MOVW_SABS_G2[[:space:]]\+abs_0x45000+0x20010\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G2" 3 "\<mov[[:space:]]\+x4,[[:space:]]\+#0x3700000000\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G2" 4 "\<R_AARCH64_MOVW_SABS_G2[[:space:]]\+abs_0x3600010000+0x100020010\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G2" 5 "\<mov[[:space:]]\+x4,[[:space:]]\+#0xfffffff2ffffffff\b"
check "aarch64_relocs.stdout" "test_R_AARCH64_MOVW_SABS_G2" 6 "\<R_AARCH64_MOVW_SABS_G2[[:space:]]\+abs_0x3600010000-0x4400010000\b"

exit 0
