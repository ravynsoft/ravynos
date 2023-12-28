#!/bin/sh -- # This comment tells perl not to loop!

#   Copyright (C) 2021-2023 Free Software Foundation, Inc.
#
# This file is part of the GNU Binutils.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.

eval 'exec ${PERL:=/usr/bin/perl} -S $0 ${1+"$@"}'
if 0;

use strict;
use File::Basename;
require "acct.pm";

# XXX This needs better documentation. Or any, really.
# e.g. what does (1, 2, 3) signify?
sub read_acct
{
  my ($fname) = @_;
  my(@checkTime, $nlines);
  @checkTime = (1, 2, 3);
  acct::readAcct($fname, @checkTime);
  if (exists $acct::Acct{"*"})
    {
      printf "Signal lost\n";
      exit 1;
    }
}

read_acct($ARGV[0]);
acct::read_er_print_out($ARGV[1], -1);
exit acct::createDiff();
