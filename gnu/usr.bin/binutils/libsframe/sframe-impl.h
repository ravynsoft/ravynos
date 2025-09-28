/* Implementation header.

   Copyright (C) 2022-2023 Free Software Foundation, Inc.

   This file is part of libsframe.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _SFRAME_IMPL_H
#define _SFRAME_IMPL_H

#include "sframe-api.h"

#ifdef  __cplusplus
extern "C"
{
#endif

#include <assert.h>
#define sframe_assert(expr) (assert (expr))

struct sframe_decoder_ctx
{
  /* SFrame header.  */
  sframe_header sfd_header;
  /* SFrame function desc entries table.  */
  sframe_func_desc_entry *sfd_funcdesc;
  /* SFrame FRE table.  */
  char *sfd_fres;
  /* Number of bytes needed for SFrame FREs.  */
  int sfd_fre_nbytes;
  /* Reference to the internally malloc'd buffer, if any, for endian flipping
     the original input buffer before decoding.  */
  void *sfd_buf;
};

typedef struct sf_fde_tbl sf_fde_tbl;
typedef struct sf_fre_tbl sf_fre_tbl;

struct sframe_encoder_ctx
{
  /* SFrame header.  */
  sframe_header sfe_header;
  /* SFrame function desc entries table.  */
  sf_fde_tbl *sfe_funcdesc;
  /* SFrame FRE table.  */
  sf_fre_tbl *sfe_fres;
  /* Number of bytes needed for SFrame FREs.  */
  uint32_t sfe_fre_nbytes;
  /* SFrame output data buffer.  */
  char *sfe_data;
  /* Size of the SFrame output data buffer.  */
  size_t sfe_data_size;
};

#ifdef  __cplusplus
}
#endif

#endif /* _SFRAME_IMPL_H */
