#ifndef NOUVEAU_CONTEXT
#define NOUVEAU_CONTEXT 1

#include "nouveau_private.h"

#ifdef __cplusplus
extern "C" {
#endif

struct nouveau_ws_device;

struct nouveau_ws_object {
   uint16_t cls;
};

struct nouveau_ws_context {
   struct nouveau_ws_device *dev;

   int channel;

   struct nouveau_ws_object copy;
   struct nouveau_ws_object eng2d;
   struct nouveau_ws_object eng3d;
   struct nouveau_ws_object m2mf;
   struct nouveau_ws_object compute;
};

int nouveau_ws_context_create(struct nouveau_ws_device *, struct nouveau_ws_context **out);
bool nouveau_ws_context_killed(struct nouveau_ws_context *);
void nouveau_ws_context_destroy(struct nouveau_ws_context *);

#ifdef __cplusplus
}
#endif

#endif
