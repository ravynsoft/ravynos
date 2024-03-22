#ifndef __NV30_SCREEN_H__
#define __NV30_SCREEN_H__

#include <stdio.h>

#include "util/list.h"

#include "nouveau_debug.h"
#include "nouveau_screen.h"
#include "nouveau_fence.h"
#include "nouveau_heap.h"
#include "nv30/nv30_resource.h"
#include "compiler/nir/nir.h"

struct nv30_context;

struct nv30_screen {
   struct nouveau_screen base;

   struct nv30_context *cur_ctx;

   struct nouveau_bo *notify;

   struct nouveau_object *ntfy;
   struct nouveau_object *fence;

   struct nouveau_object *query;
   struct nouveau_heap *query_heap;
   struct list_head queries;

   struct nouveau_object *null;
   struct nouveau_object *eng3d;
   struct nouveau_object *m2mf;
   struct nouveau_object *surf2d;
   struct nouveau_object *swzsurf;
   struct nouveau_object *sifm;

   /*XXX: nvfx state */
   struct nouveau_heap *vp_exec_heap;
   struct nouveau_heap *vp_data_heap;

   nir_shader_compiler_options fs_compiler_options;

   unsigned max_sample_count;
};

static inline struct nv30_screen *
nv30_screen(struct pipe_screen *pscreen)
{
   return (struct nv30_screen *)pscreen;
}

#endif
