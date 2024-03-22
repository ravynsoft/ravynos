/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VN_CS_H
#define VN_CS_H

#include "vn_common.h"

#include "venus-protocol/vn_protocol_driver_info.h"

#define VN_CS_ENCODER_BUFFER_INITIALIZER(storage)                            \
   (struct vn_cs_encoder_buffer)                                             \
   {                                                                         \
      .base = storage,                                                       \
   }

/* note that buffers points to an unamed local variable */
#define VN_CS_ENCODER_INITIALIZER_LOCAL(storage, size)                       \
   (struct vn_cs_encoder)                                                    \
   {                                                                         \
      .storage_type = VN_CS_ENCODER_STORAGE_POINTER,                         \
      .buffers = &VN_CS_ENCODER_BUFFER_INITIALIZER(storage),                 \
      .buffer_count = 1, .buffer_max = 1, .current_buffer_size = size,       \
      .cur = storage, .end = (const void *)(storage) + (size),               \
   }

#define VN_CS_ENCODER_INITIALIZER(buf, size)                                 \
   (struct vn_cs_encoder)                                                    \
   {                                                                         \
      .storage_type = VN_CS_ENCODER_STORAGE_POINTER, .buffers = (buf),       \
      .buffer_count = 1, .buffer_max = 1, .current_buffer_size = size,       \
      .cur = (buf)->base, .end = (buf)->base + (size),                       \
   }

#define VN_CS_DECODER_INITIALIZER(storage, size)                             \
   (struct vn_cs_decoder)                                                    \
   {                                                                         \
      .cur = storage, .end = (const void *)(storage) + (size),               \
   }

enum vn_cs_encoder_storage_type {
   /* a pointer to an externally-managed storage */
   VN_CS_ENCODER_STORAGE_POINTER,
   /* an array of dynamically allocated shmems */
   VN_CS_ENCODER_STORAGE_SHMEM_ARRAY,
   /* same as above, but shmems are suballocated from a pool */
   VN_CS_ENCODER_STORAGE_SHMEM_POOL,
};

struct vn_cs_encoder_buffer {
   struct vn_renderer_shmem *shmem;
   size_t offset;
   void *base;
   size_t committed_size;
};

struct vn_cs_encoder {
   struct vn_instance *instance; /* TODO shmem cache */
   enum vn_cs_encoder_storage_type storage_type;
   size_t min_buffer_size;

   bool fatal_error;

   struct vn_cs_encoder_buffer *buffers;
   uint32_t buffer_count;
   uint32_t buffer_max;
   size_t total_committed_size;

   /* the current buffer is buffers[buffer_count - 1].shmem */
   size_t current_buffer_size;

   /* cur is the write pointer.  When cur passes end, the slow path is
    * triggered.
    */
   void *cur;
   const void *end;
};

struct vn_cs_decoder {
   const void *cur;
   const void *end;
};

struct vn_cs_renderer_protocol_info {
   simple_mtx_t mutex;
   bool init_once;
   uint32_t api_version;
   BITSET_DECLARE(extension_bitset, VN_INFO_EXTENSION_MAX_NUMBER + 1);
};

extern struct vn_cs_renderer_protocol_info _vn_cs_renderer_protocol_info;

static inline bool
vn_cs_renderer_protocol_has_api_version(uint32_t api_version)
{
   return _vn_cs_renderer_protocol_info.api_version >= api_version;
}

static inline bool
vn_cs_renderer_protocol_has_extension(uint32_t ext_number)
{
   return BITSET_TEST(_vn_cs_renderer_protocol_info.extension_bitset,
                      ext_number);
}

void
vn_cs_renderer_protocol_info_init(struct vn_instance *instance);

void
vn_cs_encoder_init(struct vn_cs_encoder *enc,
                   struct vn_instance *instance,
                   enum vn_cs_encoder_storage_type storage_type,
                   size_t min_size);

void
vn_cs_encoder_fini(struct vn_cs_encoder *enc);

void
vn_cs_encoder_reset(struct vn_cs_encoder *enc);

static inline void
vn_cs_encoder_set_fatal(const struct vn_cs_encoder *enc)
{
   /* This is fatal and should be treated as VK_ERROR_DEVICE_LOST or even
    * abort().  Note that vn_cs_encoder_reset does not clear this.
    */
   ((struct vn_cs_encoder *)enc)->fatal_error = true;
}

static inline bool
vn_cs_encoder_get_fatal(const struct vn_cs_encoder *enc)
{
   return enc->fatal_error;
}

static inline bool
vn_cs_encoder_is_empty(const struct vn_cs_encoder *enc)
{
   return !enc->buffer_count || enc->cur == enc->buffers[0].base;
}

static inline size_t
vn_cs_encoder_get_len(const struct vn_cs_encoder *enc)
{
   if (unlikely(!enc->buffer_count))
      return 0;

   size_t len = enc->total_committed_size;
   const struct vn_cs_encoder_buffer *cur_buf =
      &enc->buffers[enc->buffer_count - 1];
   if (!cur_buf->committed_size)
      len += enc->cur - cur_buf->base;
   return len;
}

bool
vn_cs_encoder_reserve_internal(struct vn_cs_encoder *enc, size_t size);

/**
 * Reserve space for commands.
 */
static inline bool
vn_cs_encoder_reserve(struct vn_cs_encoder *enc, size_t size)
{
   if (unlikely(size > enc->end - enc->cur)) {
      if (!vn_cs_encoder_reserve_internal(enc, size)) {
         vn_cs_encoder_set_fatal(enc);
         return false;
      }
      assert(size <= enc->end - enc->cur);
   }

   return true;
}

static inline void
vn_cs_encoder_write(struct vn_cs_encoder *enc,
                    size_t size,
                    const void *val,
                    size_t val_size)
{
   assert(val_size <= size);
   assert(size <= enc->end - enc->cur);

   /* we should not rely on the compiler to optimize away memcpy... */
   memcpy(enc->cur, val, val_size);
   enc->cur += size;
}

void
vn_cs_encoder_commit(struct vn_cs_encoder *enc);

static inline void
vn_cs_decoder_init(struct vn_cs_decoder *dec, const void *data, size_t size)
{
   *dec = VN_CS_DECODER_INITIALIZER(data, size);
}

static inline void
vn_cs_decoder_set_fatal(const struct vn_cs_decoder *dec)
{
   abort();
}

static inline bool
vn_cs_decoder_peek_internal(const struct vn_cs_decoder *dec,
                            size_t size,
                            void *val,
                            size_t val_size)
{
   assert(val_size <= size);

   if (unlikely(size > dec->end - dec->cur)) {
      vn_cs_decoder_set_fatal(dec);
      memset(val, 0, val_size);
      return false;
   }

   /* we should not rely on the compiler to optimize away memcpy... */
   memcpy(val, dec->cur, val_size);
   return true;
}

static inline void
vn_cs_decoder_read(struct vn_cs_decoder *dec,
                   size_t size,
                   void *val,
                   size_t val_size)
{
   if (vn_cs_decoder_peek_internal(dec, size, val, val_size))
      dec->cur += size;
}

static inline void
vn_cs_decoder_peek(const struct vn_cs_decoder *dec,
                   size_t size,
                   void *val,
                   size_t val_size)
{
   vn_cs_decoder_peek_internal(dec, size, val, val_size);
}

static inline vn_object_id
vn_cs_handle_load_id(const void **handle, VkObjectType type)
{
   return *handle ? vn_object_get_id(*handle, type) : 0;
}

static inline void
vn_cs_handle_store_id(void **handle, vn_object_id id, VkObjectType type)
{
   vn_object_set_id(*handle, id, type);
}

#endif /* VN_CS_H */
