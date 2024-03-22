#define blob_finish __blob_finish
#define disk_cache_get_function_identifier __disk_cache_get_function_identifier
#define mesa_bytes_to_hex __mesa_bytes_to_hex
#define nir_shader_get_entrypoint __nir_shader_get_entrypoint_wraped
#define pipe_resource_reference __pipe_resource_reference_wraped
#define should_print_nir __should_print_nir
#define should_skip_nir __should_skip_nir
#define util_format_pack_rgba __util_format_pack_rgba
#define glsl_type_is_sampler __glsl_type_is_sampler
#define glsl_type_is_image __glsl_type_is_image
#define glsl_type_is_texture __glsl_type_is_texture
#define glsl_uint_type __glsl_uint_type
#define glsl_uint8_t_type __glsl_uint8_t_type
#define glsl_uint64_t_type __glsl_uint64_t_type
#define glsl_int16_t_type __glsl_int16_t_type
#define glsl_vector_type __glsl_vector_type
#include "nir.h"
#include "util/blob.h"
#include "util/disk_cache.h"
#include "util/hex.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "compiler/glsl_types.h"
#undef blob_finish
#undef mesa_bytes_to_hex
#undef disk_cache_get_function_identifier
#undef nir_shader_get_entrypoint
#undef pipe_resource_reference
#undef should_print_nir
#undef should_skip_nir
#undef util_format_pack_rgba
#undef glsl_type_is_sampler
#undef glsl_type_is_image
#undef glsl_type_is_texture
#undef glsl_uint_type
#undef glsl_uint8_t_type
#undef glsl_uint64_t_type
#undef glsl_int16_t_type
#undef glsl_vector_type

void blob_finish(struct blob *);
char *mesa_bytes_to_hex(char *buf, const uint8_t *hex_id, unsigned size);
bool disk_cache_get_function_identifier(void *ptr, struct mesa_sha1 *ctx);
const char* mesa_version_string(void);
nir_function_impl *nir_shader_get_entrypoint(const nir_shader *shader);
void pipe_resource_reference(struct pipe_resource **dst, struct pipe_resource *src);
bool should_skip_nir(const char *);
bool should_print_nir(nir_shader *);
void util_format_pack_rgba(enum pipe_format format, void *dst, const void *src, unsigned w);
bool glsl_type_is_sampler(const struct glsl_type *t);
bool glsl_type_is_image(const struct glsl_type *t);
bool glsl_type_is_texture(const struct glsl_type *t);
const struct glsl_type *glsl_uint_type(void);
const struct glsl_type *glsl_uint8_t_type(void);
const struct glsl_type *glsl_uint64_t_type(void);
const struct glsl_type *glsl_int16_t_type(void);
const struct glsl_type *glsl_vector_type(enum glsl_base_type base_type, unsigned components);
