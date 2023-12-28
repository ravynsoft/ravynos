/* Alpha VMS external format of Extended Image Header Version.

   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Written by Tristan Gingold <gingold@adacore.com>, AdaCore.

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

#ifndef _VMS_EIHVN_H
#define _VMS_EIHVN_H

struct vms_eihvn
{
  unsigned char subsystem_mask[4];
};

struct vms_eihvn_subversion
{
  unsigned char minor[2];
  unsigned char major[2];
};

#define EIHVN__BASE_IMAGE_BIT		0
#define EIHVN__MEMORY_MANAGEMENT_BIT	1
#define EIHVN__IO_BIT			2
#define EIHVN__FILES_VOLUMES_BIT	3
#define EIHVN__PROCESS_SCHED_BIT	4
#define EIHVN__SYSGEN_BIT		5
#define EIHVN__CLUSTERS_LOCKMGR_BIT	6
#define EIHVN__LOGICAL_NAMES_BIT	7
#define EIHVN__SECURITY_BIT		8
#define EIHVN__IMAGE_ACTIVATOR_BIT	9
#define EIHVN__NETWORKS_BIT		10
#define EIHVN__COUNTERS_BIT		11
#define EIHVN__STABLE_BIT		12
#define EIHVN__MISC_BIT			13
#define EIHVN__CPU_BIT			14
#define EIHVN__VOLATILE_BIT		15
#define EIHVN__SHELL_BIT		16
#define EIHVN__POSIX_BIT		17
#define EIHVN__MULTI_PROCESSING_BIT	18
#define EIHVN__GALAXY_BIT		19

#endif /* _VMS_EIHVN_H */
