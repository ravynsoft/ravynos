/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <assert.h>
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rnnutil.h"

static struct rnndomain *
finddom(struct rnn *rnn, uint32_t regbase)
{
   if (rnndec_checkaddr(rnn->vc, rnn->dom[0], regbase, 0))
      return rnn->dom[0];
   return rnn->dom[1];
}

void
_rnn_init(struct rnn *rnn, int nocolor)
{
   rnn_init();

   rnn->db = rnn_newdb();
   rnn->vc_nocolor = rnndec_newcontext(rnn->db);
   rnn->vc_nocolor->colors = &envy_null_colors;
   if (nocolor) {
      rnn->vc = rnn->vc_nocolor;
   } else {
      rnn->vc = rnndec_newcontext(rnn->db);
      rnn->vc->colors = &envy_def_colors;
   }
}

struct rnn *
rnn_new(int nocolor)
{
   struct rnn *rnn = calloc(sizeof(*rnn), 1);

   if (!rnn)
      return NULL;

   _rnn_init(rnn, nocolor);

   return rnn;
}

static void
init(struct rnn *rnn, char *file, char *domain, char *variant)
{
   /* prepare rnn stuff for lookup */
   rnn_parsefile(rnn->db, file);
   rnn_prepdb(rnn->db);
   rnn->dom[0] = rnn_finddomain(rnn->db, domain);
   if ((strcmp(domain, "A2XX") == 0) || (strcmp(domain, "A3XX") == 0)) {
      rnn->dom[1] = rnn_finddomain(rnn->db, "AXXX");
   } else {
      rnn->dom[1] = rnn->dom[0];
   }
   if (!rnn->dom[0] && rnn->dom[1]) {
      fprintf(stderr, "Could not find domain %s in %s\n", domain, file);
   }
   rnn->variant = variant;

   rnndec_varadd(rnn->vc, "chip", variant);
   if (rnn->vc != rnn->vc_nocolor)
      rnndec_varadd(rnn->vc_nocolor, "chip", variant);
   if (rnn->db->estatus)
      errx(rnn->db->estatus, "failed to parse register database");
}

void
rnn_load_file(struct rnn *rnn, char *file, char *domain)
{
   init(rnn, file, domain, domain);
}

void
rnn_load(struct rnn *rnn, const char *gpuname)
{
   if (strstr(gpuname, "a2")) {
      init(rnn, "adreno/a2xx.xml", "A2XX", "A2XX");
   } else if (strstr(gpuname, "a3")) {
      init(rnn, "adreno/a3xx.xml", "A3XX", "A3XX");
   } else if (strstr(gpuname, "a4")) {
      init(rnn, "adreno/a4xx.xml", "A4XX", "A4XX");
   } else if (strstr(gpuname, "a5")) {
      init(rnn, "adreno/a5xx.xml", "A5XX", "A5XX");
   } else if (strstr(gpuname, "a6")) {
      init(rnn, "adreno/a6xx.xml", "A6XX", "A6XX");
   } else if (strstr(gpuname, "a7")) {
      init(rnn, "adreno/a6xx.xml", "A6XX", "A7XX");
   }
}

uint32_t
rnn_regbase(struct rnn *rnn, const char *name)
{
   uint32_t regbase = rnndec_decodereg(rnn->vc_nocolor, rnn->dom[0], name);
   if (!regbase)
      regbase = rnndec_decodereg(rnn->vc_nocolor, rnn->dom[1], name);
   return regbase;
}

const char *
rnn_regname(struct rnn *rnn, uint32_t regbase, int color)
{
   static char buf[128];
   struct rnndecaddrinfo *info;

   info = rnndec_decodeaddr(color ? rnn->vc : rnn->vc_nocolor,
                            finddom(rnn, regbase), regbase, 0);
   if (info) {
      strcpy(buf, info->name);
      free(info->name);
      free(info);
      return buf;
   }
   return NULL;
}

/* call rnn_reginfo_free() to free the result */
struct rnndecaddrinfo *
rnn_reginfo(struct rnn *rnn, uint32_t regbase)
{
   return rnndec_decodeaddr(rnn->vc, finddom(rnn, regbase), regbase, 0);
}

void
rnn_reginfo_free(struct rnndecaddrinfo *info)
{
   if (!info)
      return;
   free(info->name);
   free(info);
}

const char *
rnn_enumname(struct rnn *rnn, const char *name, uint32_t val)
{
   return rnndec_decode_enum(rnn->vc, name, val);
}

static struct rnndelem *
regelem(struct rnndomain *domain, const char *name)
{
   int i;
   for (i = 0; i < domain->subelemsnum; i++) {
      struct rnndelem *elem = domain->subelems[i];
      if (!strcmp(elem->name, name))
         return elem;
   }
   return NULL;
}

/* Lookup rnndelem by name: */
struct rnndelem *
rnn_regelem(struct rnn *rnn, const char *name)
{
   struct rnndelem *elem = regelem(rnn->dom[0], name);
   if (elem)
      return elem;
   return regelem(rnn->dom[1], name);
}

static struct rnndelem *
regoff(struct rnndomain *domain, uint32_t offset)
{
   int i;
   for (i = 0; i < domain->subelemsnum; i++) {
      struct rnndelem *elem = domain->subelems[i];
      if (elem->offset == offset)
         return elem;
   }
   return NULL;
}

/* Lookup rnndelem by offset: */
struct rnndelem *
rnn_regoff(struct rnn *rnn, uint32_t offset)
{
   struct rnndelem *elem = regoff(rnn->dom[0], offset);
   if (elem)
      return elem;
   return regoff(rnn->dom[1], offset);
}

enum rnnttype
rnn_decodelem(struct rnn *rnn, struct rnntypeinfo *info, uint64_t regval,
              union rnndecval *val)
{
   val->u = regval;
   switch (info->type) {
   case RNN_TTYPE_INLINE_ENUM:
   case RNN_TTYPE_ENUM:
   case RNN_TTYPE_HEX:
   case RNN_TTYPE_INT:
   case RNN_TTYPE_UINT:
   case RNN_TTYPE_FLOAT:
   case RNN_TTYPE_BOOLEAN:
      return info->type;
   case RNN_TTYPE_FIXED:
   case RNN_TTYPE_UFIXED:
      /* TODO */
   default:
      return RNN_TTYPE_INVALID;
   }
}
