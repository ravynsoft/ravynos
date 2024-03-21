/* Plugin support for BFD.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

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

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

void bfd_plugin_set_program_name (const char *);
int bfd_plugin_open_input (bfd *, struct ld_plugin_input_file *);
void bfd_plugin_set_plugin (const char *);
bool bfd_plugin_target_p (const bfd_target *);
bool bfd_plugin_specified_p (void);
bool bfd_link_plugin_object_p (bfd *);
void register_ld_plugin_object_p (bfd_cleanup (*object_p) (bfd *, bool));
void bfd_plugin_close_file_descriptor (bfd *, int);

typedef struct plugin_data_struct
{
  int nsyms;
  const struct ld_plugin_symbol *syms;
}
plugin_data_struct;

#endif
