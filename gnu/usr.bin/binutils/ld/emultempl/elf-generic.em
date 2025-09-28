# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2006-2023 Free Software Foundation, Inc.
#
# This file is part of the GNU Binutils.
#
# This program is free software; you can redistribute it and/or modify
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
#

# This file is sourced from elf.em and from ELF targets that use
# generic.em.
#
fragment <<EOF

EOF
# Put these extra routines in ld${EMULATION_NAME}_emulation
#
LDEMUL_EMIT_CTF_EARLY=ldelf_emit_ctf_early
LDEMUL_ACQUIRE_STRINGS_FOR_CTF=ldelf_acquire_strings_for_ctf
LDEMUL_NEW_DYNSYM_FOR_CTF=ldelf_new_dynsym_for_ctf
