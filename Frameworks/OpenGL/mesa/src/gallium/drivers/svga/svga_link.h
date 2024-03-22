
#ifndef SVGA_LINK_H
#define SVGA_LINK_H

#include "pipe/p_defines.h"
#include "tgsi/tgsi_scan.h"

struct svga_context;

struct shader_linkage
{
   unsigned num_inputs;     /* number of inputs in the current shader */
   unsigned position_index; /* position register index */
   unsigned input_map_max;  /* highest index of mapped inputs */
   uint8_t input_map[PIPE_MAX_SHADER_INPUTS];

   struct {
      unsigned num_outputs;
      uint8_t output_map[PIPE_MAX_SHADER_OUTPUTS];
   } prevShader;
};

void
svga_link_shaders(const struct tgsi_shader_info *outshader_info,
                  const struct tgsi_shader_info *inshader_info,
                  struct shader_linkage *linkage);

#endif /* SVGA_LINK_H */
