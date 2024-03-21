#!/bin/sh
# Unit tests for vc-list-files
# Copyright (C) 2008-2022 Free Software Foundation, Inc.
# This file is part of the GNUlib Library.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

: ${srcdir=.}
. "$srcdir/init.sh"; path_prepend_ .

tmpdir=vc-git-$$
GIT_DIR= GIT_WORK_TREE=; unset GIT_DIR GIT_WORK_TREE

fail=1
mkdir $tmpdir && cd $tmpdir &&
  # without git, skip the test
  # The double use of 'exit' is needed for the reference to $? inside the trap.
  { ( git init -q ) > /dev/null 2>&1 \
    || skip_ "git not found in PATH"; } &&
  mkdir d &&
  touch d/a b c &&
  git config user.email "you@example.com" &&
  git config user.name "Your Name" &&
  git add . > /dev/null &&
  git commit -q -a -m log &&
  printf '%s\n' b c d/a > expected &&
  $BOURNE_SHELL "$abs_aux_dir/vc-list-files" > actual &&
  compare expected actual &&
  fail=0

Exit $fail
