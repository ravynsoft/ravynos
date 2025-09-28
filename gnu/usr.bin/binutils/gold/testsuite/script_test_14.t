/* script_test_14.t -- test SORT_BY_INIT_PRIORITY.

   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   Written by Igor Kudrin <ikudrin@accesssoftek.com>.

   This file is part of gold.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

SECTIONS
{
    .init_array : { *(SORT_BY_INIT_PRIORITY(.init_array*)) }
    .fini_array : { *(SORT_BY_INIT_PRIORITY(.fini_array*)) }
    .ctors : { *(SORT_BY_INIT_PRIORITY(.ctors*)) }
    .dtors : { *(SORT_BY_INIT_PRIORITY(.dtors*)) }
    .sec : { *(SORT_BY_INIT_PRIORITY(.sec*)) }
}
