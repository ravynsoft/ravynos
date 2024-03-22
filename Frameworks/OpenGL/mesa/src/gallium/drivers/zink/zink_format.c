#include "util/format/u_format.h"
#include "zink_format.h"
#include "util/u_math.h"

enum pipe_format
zink_decompose_vertex_format(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);
   unsigned first_non_void = util_format_get_first_non_void_channel(format);
   enum pipe_format new_format;
   assert(first_non_void == 0);
   if (!desc->is_array)
      return PIPE_FORMAT_NONE;
   if (desc->is_unorm) {
      enum pipe_format unorm_formats[] = {
         PIPE_FORMAT_R8_UNORM,
         PIPE_FORMAT_R16_UNORM,
         PIPE_FORMAT_R32_UNORM
      };
      return unorm_formats[desc->channel[first_non_void].size >> 4];
   } else if (desc->is_snorm) {
      enum pipe_format snorm_formats[] = {
         PIPE_FORMAT_R8_SNORM,
         PIPE_FORMAT_R16_SNORM,
         PIPE_FORMAT_R32_SNORM
      };
      return snorm_formats[desc->channel[first_non_void].size >> 4];
   } else {
      enum pipe_format uint_formats[][3] = {
         {PIPE_FORMAT_R8_USCALED, PIPE_FORMAT_R16_USCALED, PIPE_FORMAT_R32_USCALED},
         {PIPE_FORMAT_R8_UINT, PIPE_FORMAT_R16_UINT, PIPE_FORMAT_R32_UINT},
      };
      enum pipe_format sint_formats[][3] = {
         {PIPE_FORMAT_R8_SSCALED, PIPE_FORMAT_R16_SSCALED, PIPE_FORMAT_R32_SSCALED},
         {PIPE_FORMAT_R8_SINT, PIPE_FORMAT_R16_SINT, PIPE_FORMAT_R32_SINT},
      };
      switch (desc->channel[first_non_void].type) {
      case UTIL_FORMAT_TYPE_UNSIGNED:
         return uint_formats[desc->channel[first_non_void].pure_integer][desc->channel[first_non_void].size >> 4];
      case UTIL_FORMAT_TYPE_SIGNED:
         return sint_formats[desc->channel[first_non_void].pure_integer][desc->channel[first_non_void].size >> 4];
      case UTIL_FORMAT_TYPE_FLOAT:
         return desc->channel[first_non_void].size == 16 ? PIPE_FORMAT_R16_FLOAT : PIPE_FORMAT_R32_FLOAT;
         break;
      default:
         return PIPE_FORMAT_NONE;
      }
   }
   return new_format;
}

bool
zink_format_is_red_alpha(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_R4A4_UNORM:
   case PIPE_FORMAT_R8A8_SINT:
   case PIPE_FORMAT_R8A8_SNORM:
   case PIPE_FORMAT_R8A8_UINT:
   case PIPE_FORMAT_R8A8_UNORM:
   case PIPE_FORMAT_R16A16_SINT:
   case PIPE_FORMAT_R16A16_SNORM:
   case PIPE_FORMAT_R16A16_UINT:
   case PIPE_FORMAT_R16A16_UNORM:
   case PIPE_FORMAT_R16A16_FLOAT:
   case PIPE_FORMAT_R32A32_SINT:
   case PIPE_FORMAT_R32A32_UINT:
   case PIPE_FORMAT_R32A32_FLOAT:
      return true;
   default: break;
   }
   return false;
}

bool
zink_format_is_emulated_alpha(enum pipe_format format)
{
   return util_format_is_alpha(format) ||
          util_format_is_luminance(format) ||
          util_format_is_luminance_alpha(format) ||
          zink_format_is_red_alpha(format);
}

static enum pipe_format
emulate_alpha(enum pipe_format format)
{
   if (format == PIPE_FORMAT_A8_UNORM)
      return PIPE_FORMAT_R8_UNORM;
   if (format == PIPE_FORMAT_A8_UINT)
      return PIPE_FORMAT_R8_UINT;
   if (format == PIPE_FORMAT_A8_SNORM)
      return PIPE_FORMAT_R8_SNORM;
   if (format == PIPE_FORMAT_A8_SINT)
      return PIPE_FORMAT_R8_SINT;
   if (format == PIPE_FORMAT_A16_UNORM)
      return PIPE_FORMAT_R16_UNORM;
   if (format == PIPE_FORMAT_A16_UINT)
      return PIPE_FORMAT_R16_UINT;
   if (format == PIPE_FORMAT_A16_SNORM)
      return PIPE_FORMAT_R16_SNORM;
   if (format == PIPE_FORMAT_A16_SINT)
      return PIPE_FORMAT_R16_SINT;
   if (format == PIPE_FORMAT_A16_FLOAT)
      return PIPE_FORMAT_R16_FLOAT;
   if (format == PIPE_FORMAT_A32_UINT)
      return PIPE_FORMAT_R32_UINT;
   if (format == PIPE_FORMAT_A32_SINT)
      return PIPE_FORMAT_R32_SINT;
   if (format == PIPE_FORMAT_A32_FLOAT)
      return PIPE_FORMAT_R32_FLOAT;
   return format;
}

static enum pipe_format
emulate_red_alpha(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_R8A8_SINT:
      return PIPE_FORMAT_R8G8_SINT;
   case PIPE_FORMAT_R8A8_SNORM:
      return PIPE_FORMAT_R8G8_SNORM;
   case PIPE_FORMAT_R8A8_UINT:
      return PIPE_FORMAT_R8G8_UINT;
   case PIPE_FORMAT_R8A8_UNORM:
      return PIPE_FORMAT_R8G8_UNORM;
   case PIPE_FORMAT_R16A16_SINT:
      return PIPE_FORMAT_R16G16_SINT;
   case PIPE_FORMAT_R16A16_SNORM:
      return PIPE_FORMAT_R16G16_SNORM;
   case PIPE_FORMAT_R16A16_UINT:
      return PIPE_FORMAT_R16G16_UINT;
   case PIPE_FORMAT_R16A16_UNORM:
      return PIPE_FORMAT_R16G16_UNORM;
   case PIPE_FORMAT_R16A16_FLOAT:
      return PIPE_FORMAT_R16G16_FLOAT;
   case PIPE_FORMAT_R32A32_SINT:
      return PIPE_FORMAT_R32G32_SINT;
   case PIPE_FORMAT_R32A32_UINT:
      return PIPE_FORMAT_R32G32_UINT;
   case PIPE_FORMAT_R32A32_FLOAT:
      return PIPE_FORMAT_R32G32_FLOAT;
   default: break;
   }
   return format;
}

enum pipe_format
zink_format_get_emulated_alpha(enum pipe_format format)
{
   if (util_format_is_alpha(format))
      return emulate_alpha(format);
   if (util_format_is_luminance(format))
      return util_format_luminance_to_red(format);
   if (util_format_is_luminance_alpha(format)) {
      if (format == PIPE_FORMAT_LATC2_UNORM)
         return PIPE_FORMAT_RGTC2_UNORM;
      if (format == PIPE_FORMAT_LATC2_SNORM)
         return PIPE_FORMAT_RGTC2_SNORM;

      format = util_format_luminance_to_red(format);
   }

   return emulate_red_alpha(format);
}

bool
zink_format_is_voidable_rgba_variant(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);
   unsigned chan;

   if(desc->block.width != 1 ||
      desc->block.height != 1 ||
      (desc->block.bits != 32 && desc->block.bits != 64 &&
       desc->block.bits != 128))
      return false;

   if (desc->nr_channels != 4)
      return false;

   unsigned size = desc->channel[0].size;
   for(chan = 0; chan < 4; ++chan) {
      if(desc->channel[chan].size != size)
         return false;
   }

   return true;
}

void
zink_format_clamp_channel_color(const struct util_format_description *desc, union pipe_color_union *dst, const union pipe_color_union *src, unsigned i)
{
   int non_void = util_format_get_first_non_void_channel(desc->format);
   unsigned channel = desc->swizzle[i];

   if (channel > PIPE_SWIZZLE_W || desc->channel[channel].type == UTIL_FORMAT_TYPE_VOID) {
      if (non_void != -1) {
         if (desc->channel[non_void].type == UTIL_FORMAT_TYPE_FLOAT) {
            dst->f[i] = uif(UINT32_MAX);
         } else {
            if (desc->channel[non_void].normalized)
               dst->f[i] = 1.0;
            else if (desc->channel[non_void].type == UTIL_FORMAT_TYPE_SIGNED)
               dst->i[i] = INT32_MAX;
            else
               dst->ui[i] = UINT32_MAX;
         }
      } else {
         dst->ui[i] = src->ui[i];
      }
      return;
   }

   switch (desc->channel[channel].type) {
   case UTIL_FORMAT_TYPE_VOID:
      unreachable("handled above");
      break;
   case UTIL_FORMAT_TYPE_SIGNED:
      if (desc->channel[channel].normalized)
         dst->i[i] = src->i[i];
      else {
         dst->i[i] = MAX2(src->i[i], -(1<<(desc->channel[channel].size - 1)));
         dst->i[i] = MIN2(dst->i[i], (1 << (desc->channel[channel].size - 1)) - 1);
      }
      break;
   case UTIL_FORMAT_TYPE_UNSIGNED:
      if (desc->channel[channel].normalized)
         dst->ui[i] = src->ui[i];
      else
         dst->ui[i] = MIN2(src->ui[i], BITFIELD_MASK(desc->channel[channel].size));
      break;
   case UTIL_FORMAT_TYPE_FIXED:
   case UTIL_FORMAT_TYPE_FLOAT:
      dst->ui[i] = src->ui[i];
      break;
   }
}

void
zink_format_clamp_channel_srgb(const struct util_format_description *desc, union pipe_color_union *dst, const union pipe_color_union *src, unsigned i)
{
   unsigned channel = desc->swizzle[i];
   if (desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB &&
       channel <= PIPE_SWIZZLE_W) {
      switch (desc->channel[channel].type) {
      case UTIL_FORMAT_TYPE_SIGNED:
      case UTIL_FORMAT_TYPE_UNSIGNED:
         dst->f[i] = CLAMP(src->f[i], 0.0, 1.0);
         return;
      default:
         break;
      }
   }

   dst->ui[i] = src->ui[i];
}
