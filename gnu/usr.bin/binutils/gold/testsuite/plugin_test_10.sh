#!/bin/sh

# plugin_test_10.sh -- a test case for the plugin API.

# Copyright (C) 2010-2023 Free Software Foundation, Inc.
# Written by Rafael Ávila de Espíndola <rafael.espindola@gmail.com>

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

# This file goes with plugin_common_test_1.c and plugin_common_test_2.c.
# The first file is claimed by the plugin, the second one is not. We test
# the bigger alignment in plugin_common_test_2.c is used.

set -e

grep -q ".bss.* 8$" plugin_test_10.sections

exit 0
