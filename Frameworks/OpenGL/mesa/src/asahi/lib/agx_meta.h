/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_META_H
#define __AGX_META_H

#include "asahi/compiler/agx_compile.h"
#include "agx_tilebuffer.h"
#include "pool.h"

struct agx_meta_cache {
   struct agx_device *dev;
   struct agx_pool pool;

   /* Map from agx_meta_key to agx_meta_shader */
   struct hash_table *ht;
};

enum agx_meta_op {
   AGX_META_OP_NONE,
   AGX_META_OP_CLEAR,
   AGX_META_OP_LOAD,
   AGX_META_OP_STORE,
};

struct agx_meta_key {
   struct agx_tilebuffer_layout tib;
   enum agx_meta_op op[8];
   unsigned reserved_preamble;
};

struct agx_meta_shader {
   struct agx_meta_key key;
   struct agx_shader_info info;
   struct agx_bo *bo;
   uint32_t ptr;
};

struct agx_meta_shader *agx_get_meta_shader(struct agx_meta_cache *cache,
                                            struct agx_meta_key *key);

void agx_meta_init(struct agx_meta_cache *cache, struct agx_device *dev);
void agx_meta_cleanup(struct agx_meta_cache *cache);

#endif
