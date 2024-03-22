#include "rusticl_mesa_inline_bindings_wrapper.h"
#include "git_sha1.h"

void
blob_finish(struct blob *blob)
{
    __blob_finish(blob);
}

bool
disk_cache_get_function_identifier(void *ptr, struct mesa_sha1 *ctx)
{
   return __disk_cache_get_function_identifier(ptr, ctx);
}

char *
mesa_bytes_to_hex(char *buf, const uint8_t *hex_id, unsigned size)
{
   return __mesa_bytes_to_hex(buf, hex_id, size);
}

nir_function_impl *
nir_shader_get_entrypoint(const nir_shader *shader)
{
   return __nir_shader_get_entrypoint_wraped(shader);
}

void
pipe_resource_reference(struct pipe_resource **dst, struct pipe_resource *src)
{
   __pipe_resource_reference_wraped(dst, src);
}

bool
should_skip_nir(const char *name)
{
    return __should_skip_nir(name);
}

bool
should_print_nir(nir_shader *shader)
{
    return __should_print_nir(shader);
}

void
util_format_pack_rgba(enum pipe_format format, void *dst, const void *src, unsigned w)
{
    return __util_format_pack_rgba(format, dst, src, w);
}

const char*
mesa_version_string(void)
{
    return PACKAGE_VERSION MESA_GIT_SHA1;
}

bool
glsl_type_is_sampler(const struct glsl_type *t)
{
    return __glsl_type_is_sampler(t);
}

bool
glsl_type_is_image(const struct glsl_type *t)
{
    return __glsl_type_is_image(t);
}

bool
glsl_type_is_texture(const struct glsl_type *t)
{
    return __glsl_type_is_texture(t);
}

const struct glsl_type *
glsl_uint_type(void)
{
    return __glsl_uint_type();
}

const struct glsl_type *
glsl_uint8_t_type(void)
{
    return __glsl_uint8_t_type();
}

const struct glsl_type *
glsl_uint64_t_type(void)
{
    return __glsl_uint64_t_type();
}

const struct glsl_type *
glsl_int16_t_type(void)
{
    return __glsl_int16_t_type();
}

const struct glsl_type *
glsl_vector_type(enum glsl_base_type base_type, unsigned components)
{
    return __glsl_vector_type(base_type, components);
}
