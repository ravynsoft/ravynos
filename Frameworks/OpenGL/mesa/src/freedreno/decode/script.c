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

#define LUA_COMPAT_APIINTCASTS

#include <assert.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/u_math.h"

#include "cffdec.h"
#include "rnnutil.h"
#include "script.h"

static lua_State *L;

#if 0
#define DBG(fmt, ...)                                                          \
   do {                                                                        \
      printf(" ** %s:%d ** " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);     \
   } while (0)
#else
#define DBG(fmt, ...)                                                          \
   do {                                                                        \
   } while (0)
#endif

/* An rnn based decoder, which can either be decoding current register
 * values, or domain based decoding of a pm4 packet.
 *
 */
struct rnndec {
   struct rnn base;

   /* for pm4 packet decoding: */
   uint32_t sizedwords;
   uint32_t *dwords;
};

static inline struct rnndec *
to_rnndec(struct rnn *rnn)
{
   return (struct rnndec *)rnn;
}

static uint32_t
rnn_val(struct rnn *rnn, uint32_t regbase)
{
   struct rnndec *rnndec = to_rnndec(rnn);

   if (!rnndec->sizedwords) {
      return reg_val(regbase);
   } else if (regbase < rnndec->sizedwords) {
      return rnndec->dwords[regbase];
   } else {
      // XXX throw an error
      return -1;
   }
}

/* does not return */
static void
error(const char *fmt)
{
   fprintf(stderr, fmt, lua_tostring(L, -1));
   exit(1);
}

/*
 * An enum type that can be used as string or number:
 */

struct rnndenum {
   const char *str;
   int val;
};

static int
l_meta_rnn_enum_tostring(lua_State *L)
{
   struct rnndenum *e = lua_touserdata(L, 1);
   if (e->str) {
      lua_pushstring(L, e->str);
   } else {
      char buf[32];
      sprintf(buf, "%u", e->val);
      lua_pushstring(L, buf);
   }
   return 1;
}

/* so, this doesn't actually seem to be implemented yet, but hopefully
 * some day lua comes to it's senses
 */
static int
l_meta_rnn_enum_tonumber(lua_State *L)
{
   struct rnndenum *e = lua_touserdata(L, 1);
   lua_pushinteger(L, e->val);
   return 1;
}

static const struct luaL_Reg l_meta_rnn_enum[] = {
   {"__tostring", l_meta_rnn_enum_tostring},
   {"__tonumber", l_meta_rnn_enum_tonumber},
   {NULL, NULL} /* sentinel */
};

static void
pushenum(struct lua_State *L, int val, struct rnnenum *info)
{
   struct rnndenum *e = lua_newuserdata(L, sizeof(*e));

   e->val = val;
   e->str = NULL;

   for (int i = 0; i < info->valsnum; i++) {
      if (info->vals[i]->valvalid && (info->vals[i]->value == val)) {
         e->str = info->vals[i]->name;
         break;
      }
   }

   luaL_newmetatable(L, "rnnmetaenum");
   luaL_setfuncs(L, l_meta_rnn_enum, 0);
   lua_pop(L, 1);

   luaL_setmetatable(L, "rnnmetaenum");
}

/* Expose rnn decode to script environment as "rnn" library:
 */

struct rnndoff {
   struct rnn *rnn;
   struct rnndelem *elem;
   uint64_t offset;
};

static void
push_rnndoff(lua_State *L, struct rnn *rnn, struct rnndelem *elem,
             uint64_t offset)
{
   struct rnndoff *rnndoff = lua_newuserdata(L, sizeof(*rnndoff));
   rnndoff->rnn = rnn;
   rnndoff->elem = elem;
   rnndoff->offset = offset;
}

static int l_rnn_etype_array(lua_State *L, struct rnn *rnn,
                             struct rnndelem *elem, uint64_t offset);
static int l_rnn_etype_reg(lua_State *L, struct rnn *rnn, struct rnndelem *elem,
                           uint64_t offset);

static int
pushdecval(struct lua_State *L, struct rnn *rnn, uint64_t regval,
           struct rnntypeinfo *info)
{
   union rnndecval val;
   switch (rnn_decodelem(rnn, info, regval, &val)) {
   case RNN_TTYPE_ENUM:
   case RNN_TTYPE_INLINE_ENUM:
      pushenum(L, val.i, info->eenum);
      return 1;
   case RNN_TTYPE_INT:
      lua_pushinteger(L, val.i);
      return 1;
   case RNN_TTYPE_UINT:
   case RNN_TTYPE_HEX:
      lua_pushunsigned(L, val.u);
      return 1;
   case RNN_TTYPE_FLOAT:
      lua_pushnumber(L, uif(val.u));
      return 1;
   case RNN_TTYPE_BOOLEAN:
      lua_pushboolean(L, val.u);
      return 1;
   case RNN_TTYPE_INVALID:
   default:
      return 0;
   }
}

static int
l_rnn_etype(lua_State *L, struct rnn *rnn, struct rnndelem *elem,
            uint64_t offset)
{
   int ret;
   uint64_t regval;
   DBG("elem=%p (%d), offset=%lu", elem, elem->type, offset);
   switch (elem->type) {
   case RNN_ETYPE_REG:
      /* if a register has no bitfields, just return
       * the raw value:
       */
      regval = rnn_val(rnn, offset);
      if (elem->width == 64)
         regval |= (uint64_t)rnn_val(rnn, offset + 1) << 32;
      regval <<= elem->typeinfo.shr;
      ret = pushdecval(L, rnn, regval, &elem->typeinfo);
      if (ret)
         return ret;
      return l_rnn_etype_reg(L, rnn, elem, offset);
   case RNN_ETYPE_ARRAY:
      return l_rnn_etype_array(L, rnn, elem, offset);
   default:
      /* hmm.. */
      printf("unhandled type: %d\n", elem->type);
      return 0;
   }
}

/*
 * Struct Object:
 * To implement stuff like 'RB_MRT[n].CONTROL' we need a struct-object
 * to represent the current array index (ie. 'RB_MRT[n]')
 */

static int
l_rnn_struct_meta_index(lua_State *L)
{
   struct rnndoff *rnndoff = lua_touserdata(L, 1);
   const char *name = lua_tostring(L, 2);
   struct rnndelem *elem = rnndoff->elem;
   int i;

   for (i = 0; i < elem->subelemsnum; i++) {
      struct rnndelem *subelem = elem->subelems[i];
      if (!strcmp(name, subelem->name)) {
         return l_rnn_etype(L, rnndoff->rnn, subelem,
                            rnndoff->offset + subelem->offset);
      }
   }

   return 0;
}

static const struct luaL_Reg l_meta_rnn_struct[] = {
   {"__index", l_rnn_struct_meta_index}, {NULL, NULL} /* sentinel */
};

static int
l_rnn_etype_struct(lua_State *L, struct rnn *rnn, struct rnndelem *elem,
                   uint64_t offset)
{
   push_rnndoff(L, rnn, elem, offset);

   luaL_newmetatable(L, "rnnmetastruct");
   luaL_setfuncs(L, l_meta_rnn_struct, 0);
   lua_pop(L, 1);

   luaL_setmetatable(L, "rnnmetastruct");

   return 1;
}

/*
 * Array Object:
 */

static int
l_rnn_array_meta_index(lua_State *L)
{
   struct rnndoff *rnndoff = lua_touserdata(L, 1);
   int idx = lua_tointeger(L, 2);
   struct rnndelem *elem = rnndoff->elem;
   uint64_t offset = rnndoff->offset + (elem->stride * idx);

   DBG("rnndoff=%p, idx=%d, numsubelems=%d", rnndoff, idx,
       rnndoff->elem->subelemsnum);

   /* if just a single sub-element, it is directly a register,
    * otherwise we need to accumulate the array index while
    * we wait for the register name within the array..
    */
   if (elem->subelemsnum == 1) {
      return l_rnn_etype(L, rnndoff->rnn, elem->subelems[0], offset);
   } else {
      return l_rnn_etype_struct(L, rnndoff->rnn, elem, offset);
   }

   return 0;
}

static const struct luaL_Reg l_meta_rnn_array[] = {
   {"__index", l_rnn_array_meta_index}, {NULL, NULL} /* sentinel */
};

static int
l_rnn_etype_array(lua_State *L, struct rnn *rnn, struct rnndelem *elem,
                  uint64_t offset)
{
   push_rnndoff(L, rnn, elem, offset);

   luaL_newmetatable(L, "rnnmetaarray");
   luaL_setfuncs(L, l_meta_rnn_array, 0);
   lua_pop(L, 1);

   luaL_setmetatable(L, "rnnmetaarray");

   return 1;
}

/*
 * Register element:
 */

static int
l_rnn_reg_meta_index(lua_State *L)
{
   struct rnndoff *rnndoff = lua_touserdata(L, 1);
   const char *name = lua_tostring(L, 2);
   struct rnndelem *elem = rnndoff->elem;
   struct rnntypeinfo *info = &elem->typeinfo;
   struct rnnbitfield **bitfields;
   int bitfieldsnum;
   int i;

   switch (info->type) {
   case RNN_TTYPE_BITSET:
      bitfields = info->ebitset->bitfields;
      bitfieldsnum = info->ebitset->bitfieldsnum;
      break;
   case RNN_TTYPE_INLINE_BITSET:
      bitfields = info->bitfields;
      bitfieldsnum = info->bitfieldsnum;
      break;
   default:
      printf("invalid register type: %d\n", info->type);
      return 0;
   }

   for (i = 0; i < bitfieldsnum; i++) {
      struct rnnbitfield *bf = bitfields[i];
      if (!strcmp(name, bf->name)) {
         uint32_t regval = rnn_val(rnndoff->rnn, rnndoff->offset);

         regval &= typeinfo_mask(&bf->typeinfo);
         regval >>= bf->typeinfo.low;
         regval <<= bf->typeinfo.shr;

         DBG("name=%s, info=%p, subelemsnum=%d, type=%d, regval=%x", name, info,
             rnndoff->elem->subelemsnum, bf->typeinfo.type, regval);

         return pushdecval(L, rnndoff->rnn, regval, &bf->typeinfo);
      }
   }

   printf("invalid member: %s\n", name);
   return 0;
}

static int
l_rnn_reg_meta_tostring(lua_State *L)
{
   struct rnndoff *rnndoff = lua_touserdata(L, 1);
   uint32_t regval = rnn_val(rnndoff->rnn, rnndoff->offset);
   struct rnndecaddrinfo *info = rnn_reginfo(rnndoff->rnn, rnndoff->offset);
   char *decoded;
   if (info && info->typeinfo) {
      decoded = rnndec_decodeval(rnndoff->rnn->vc, info->typeinfo, regval);
   } else {
      asprintf(&decoded, "%08x", regval);
   }
   lua_pushstring(L, decoded);
   free(decoded);
   rnn_reginfo_free(info);
   return 1;
}

static int
l_rnn_reg_meta_tonumber(lua_State *L)
{
   struct rnndoff *rnndoff = lua_touserdata(L, 1);
   uint32_t regval = rnn_val(rnndoff->rnn, rnndoff->offset);

   regval <<= rnndoff->elem->typeinfo.shr;

   lua_pushnumber(L, regval);
   return 1;
}

static const struct luaL_Reg l_meta_rnn_reg[] = {
   {"__index", l_rnn_reg_meta_index},
   {"__tostring", l_rnn_reg_meta_tostring},
   {"__tonumber", l_rnn_reg_meta_tonumber},
   {NULL, NULL} /* sentinel */
};

static int
l_rnn_etype_reg(lua_State *L, struct rnn *rnn, struct rnndelem *elem,
                uint64_t offset)
{
   push_rnndoff(L, rnn, elem, offset);

   luaL_newmetatable(L, "rnnmetareg");
   luaL_setfuncs(L, l_meta_rnn_reg, 0);
   lua_pop(L, 1);

   luaL_setmetatable(L, "rnnmetareg");

   return 1;
}

/*
 *
 */

static int
l_rnn_meta_index(lua_State *L)
{
   struct rnn *rnn = lua_touserdata(L, 1);
   const char *name = lua_tostring(L, 2);
   struct rnndelem *elem;

   elem = rnn_regelem(rnn, name);
   if (!elem)
      return 0;

   return l_rnn_etype(L, rnn, elem, elem->offset);
}

static int
l_rnn_meta_gc(lua_State *L)
{
   // TODO
   // struct rnn *rnn = lua_touserdata(L, 1);
   // rnn_deinit(rnn);
   return 0;
}

static const struct luaL_Reg l_meta_rnn[] = {
   {"__index", l_rnn_meta_index},
   {"__gc", l_rnn_meta_gc},
   {NULL, NULL} /* sentinel */
};

static int
l_rnn_init(lua_State *L)
{
   const char *gpuname = lua_tostring(L, 1);
   struct rnndec *rnndec = lua_newuserdata(L, sizeof(*rnndec));
   _rnn_init(&rnndec->base, 0);
   rnn_load(&rnndec->base, gpuname);
   rnndec->sizedwords = 0;

   luaL_newmetatable(L, "rnnmeta");
   luaL_setfuncs(L, l_meta_rnn, 0);
   lua_pop(L, 1);

   luaL_setmetatable(L, "rnnmeta");

   return 1;
}

static int
l_rnn_enumname(lua_State *L)
{
   struct rnn *rnn = lua_touserdata(L, 1);
   const char *name = lua_tostring(L, 2);
   uint32_t val = (uint32_t)lua_tonumber(L, 3);
   lua_pushstring(L, rnn_enumname(rnn, name, val));
   return 1;
}

static int
l_rnn_regname(lua_State *L)
{
   struct rnn *rnn = lua_touserdata(L, 1);
   uint32_t regbase = (uint32_t)lua_tonumber(L, 2);
   lua_pushstring(L, rnn_regname(rnn, regbase, 1));
   return 1;
}

static int
l_rnn_regval(lua_State *L)
{
   struct rnn *rnn = lua_touserdata(L, 1);
   uint32_t regbase = (uint32_t)lua_tonumber(L, 2);
   uint32_t regval = (uint32_t)lua_tonumber(L, 3);
   struct rnndecaddrinfo *info = rnn_reginfo(rnn, regbase);
   char *decoded;
   if (info && info->typeinfo) {
      decoded = rnndec_decodeval(rnn->vc, info->typeinfo, regval);
   } else {
      asprintf(&decoded, "%08x", regval);
   }
   lua_pushstring(L, decoded);
   free(decoded);
   rnn_reginfo_free(info);
   return 1;
}

static const struct luaL_Reg l_rnn[] = {
   {"init", l_rnn_init},
   {"enumname", l_rnn_enumname},
   {"regname", l_rnn_regname},
   {"regval", l_rnn_regval},
   {NULL, NULL} /* sentinel */
};

/* Expose the register state to script enviroment as a "regs" library:
 */

static int
l_reg_written(lua_State *L)
{
   uint32_t regbase = (uint32_t)lua_tonumber(L, 1);
   lua_pushnumber(L, reg_written(regbase));
   return 1;
}

static int
l_reg_lastval(lua_State *L)
{
   uint32_t regbase = (uint32_t)lua_tonumber(L, 1);
   lua_pushnumber(L, reg_lastval(regbase));
   return 1;
}

static int
l_reg_val(lua_State *L)
{
   uint32_t regbase = (uint32_t)lua_tonumber(L, 1);
   lua_pushnumber(L, reg_val(regbase));
   return 1;
}

static const struct luaL_Reg l_regs[] = {
   {"written", l_reg_written},
   {"lastval", l_reg_lastval},
   {"val", l_reg_val},
   {NULL, NULL} /* sentinel */
};

/* Expose API to lookup snapshot buffers:
 */

uint64_t gpubaseaddr(uint64_t gpuaddr);
unsigned hostlen(uint64_t gpuaddr);

/* given address, return base-address of buffer: */
static int
l_bo_base(lua_State *L)
{
   uint64_t addr = (uint64_t)lua_tonumber(L, 1);
   lua_pushnumber(L, gpubaseaddr(addr));
   return 1;
}

/* given address, return the remaining size of the buffer: */
static int
l_bo_size(lua_State *L)
{
   uint64_t addr = (uint64_t)lua_tonumber(L, 1);
   lua_pushnumber(L, hostlen(addr));
   return 1;
}

static const struct luaL_Reg l_bos[] = {
   {"base", l_bo_base}, {"size", l_bo_size}, {NULL, NULL} /* sentinel */
};

static void
openlib(const char *lib, const luaL_Reg *reg)
{
   lua_newtable(L);
   luaL_setfuncs(L, reg, 0);
   lua_setglobal(L, lib);
}

/* called at start to load the script: */
int
script_load(const char *file)
{
   int ret;

   assert(!L);

   L = luaL_newstate();
   luaL_openlibs(L);
   openlib("bos", l_bos);
   openlib("regs", l_regs);
   openlib("rnn", l_rnn);

   ret = luaL_loadfile(L, file);
   if (ret)
      error("%s\n");

   ret = lua_pcall(L, 0, LUA_MULTRET, 0);
   if (ret)
      error("%s\n");

   return 0;
}

/* called at start of each cmdstream file: */
void
script_start_cmdstream(const char *name)
{
   if (!L)
      return;

   lua_getglobal(L, "start_cmdstream");

   /* if no handler just ignore it: */
   if (!lua_isfunction(L, -1)) {
      lua_pop(L, 1);
      return;
   }

   lua_pushstring(L, name);

   /* do the call (1 arguments, 0 result) */
   if (lua_pcall(L, 1, 0, 0) != 0)
      error("error running function `f': %s\n");
}

/* called at each DRAW_INDX, calls script drawidx fxn to process
 * the current state
 */
void
script_draw(const char *primtype, uint32_t nindx)
{
   if (!L)
      return;

   lua_getglobal(L, "draw");

   /* if no handler just ignore it: */
   if (!lua_isfunction(L, -1)) {
      lua_pop(L, 1);
      return;
   }

   lua_pushstring(L, primtype);
   lua_pushnumber(L, nindx);

   /* do the call (2 arguments, 0 result) */
   if (lua_pcall(L, 2, 0, 0) != 0)
      error("error running function `f': %s\n");
}

static int
l_rnn_meta_dom_index(lua_State *L)
{
   struct rnn *rnn = lua_touserdata(L, 1);
   uint32_t offset = (uint32_t)lua_tonumber(L, 2);
   struct rnndelem *elem;

   /* TODO might be nicer if the arg isn't a number, to search the domain
    * for matching bitfields.. so that the script could do something like
    * 'pkt.WIDTH' insteadl of 'pkt[1].WIDTH', ie. not have to remember the
    * offset of the dword containing the bitfield..
    */

   elem = rnn_regoff(rnn, offset);
   if (!elem)
      return 0;

   return l_rnn_etype(L, rnn, elem, elem->offset);
}

/*
 * A wrapper object for rnndomain based decoding of an array of dwords
 * (ie. for pm4 packet decoding).  Mostly re-uses the register-value
 * decoding for the individual dwords and bitfields.
 */

static int
l_rnn_meta_dom_gc(lua_State *L)
{
   // TODO
   // struct rnn *rnn = lua_touserdata(L, 1);
   // rnn_deinit(rnn);
   return 0;
}

static const struct luaL_Reg l_meta_rnn_dom[] = {
   {"__index", l_rnn_meta_dom_index},
   {"__gc", l_rnn_meta_dom_gc},
   {NULL, NULL} /* sentinel */
};

/* called to general pm4 packet decoding, such as texture/sampler state
 */
void
script_packet(uint32_t *dwords, uint32_t sizedwords, struct rnn *rnn,
              struct rnndomain *dom)
{
   if (!L)
      return;

   lua_getglobal(L, dom->name);

   /* if no handler for the packet, just ignore it: */
   if (!lua_isfunction(L, -1)) {
      lua_pop(L, 1);
      return;
   }

   struct rnndec *rnndec = lua_newuserdata(L, sizeof(*rnndec));

   rnndec->base = *rnn;
   rnndec->base.dom[0] = dom;
   rnndec->base.dom[1] = NULL;
   rnndec->dwords = dwords;
   rnndec->sizedwords = sizedwords;

   luaL_newmetatable(L, "rnnmetadom");
   luaL_setfuncs(L, l_meta_rnn_dom, 0);
   lua_pop(L, 1);

   luaL_setmetatable(L, "rnnmetadom");

   lua_pushnumber(L, sizedwords);

   if (lua_pcall(L, 2, 0, 0) != 0)
      error("error running function `f': %s\n");
}

/* helper to call fxn that takes and returns void: */
static void
simple_call(const char *name)
{
   if (!L)
      return;

   lua_getglobal(L, name);

   /* if no handler just ignore it: */
   if (!lua_isfunction(L, -1)) {
      lua_pop(L, 1);
      return;
   }

   /* do the call (0 arguments, 0 result) */
   if (lua_pcall(L, 0, 0, 0) != 0)
      error("error running function `f': %s\n");
}

/* called at end of each cmdstream file: */
void
script_end_cmdstream(void)
{
   simple_call("end_cmdstream");
}

/* called at start of submit/issueibcmds: */
void
script_start_submit(void)
{
   simple_call("start_submit");
}

/* called at end of submit/issueibcmds: */
void
script_end_submit(void)
{
   simple_call("end_submit");
}

/* called after last cmdstream file: */
void
script_finish(void)
{
   if (!L)
      return;

   simple_call("finish");

   lua_close(L);
   L = NULL;
}
