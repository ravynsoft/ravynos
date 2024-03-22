#ifndef U_BLEND_H
#define U_BLEND_H

#include "pipe/p_state.h"
#include "compiler/shader_enums.h"

static inline bool
util_blend_factor_uses_dest(enum pipe_blendfactor factor, bool alpha)
{
   switch (factor) {
      case PIPE_BLENDFACTOR_DST_ALPHA:
      case PIPE_BLENDFACTOR_DST_COLOR:
      case PIPE_BLENDFACTOR_INV_DST_ALPHA:
      case PIPE_BLENDFACTOR_INV_DST_COLOR:
         return true;
      case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
         return !alpha;
      default:
         return false;
   }
}

static inline bool
util_blend_uses_dest(struct pipe_rt_blend_state rt)
{
   return rt.blend_enable &&
      (util_blend_factor_uses_dest((enum pipe_blendfactor)rt.rgb_src_factor, false) ||
       util_blend_factor_uses_dest((enum pipe_blendfactor)rt.alpha_src_factor, true) ||
       rt.rgb_dst_factor != PIPE_BLENDFACTOR_ZERO ||
       rt.alpha_dst_factor != PIPE_BLENDFACTOR_ZERO);
}

#endif /* U_BLEND_H */
