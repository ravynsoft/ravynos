/*
 * Copyright 2017-2019 Lyude Paul
 * Copyright 2017-2019 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef __AGX_DECODE_H__
#define __AGX_DECODE_H__

#include <sys/types.h>
#include "agx_bo.h"

void agxdecode_next_frame(void);

void agxdecode_close(void);

void agxdecode_cmdstream(unsigned cmdbuf_index, unsigned map_index,
                         bool verbose);

void agxdecode_dump_file_open(void);

void agxdecode_track_alloc(struct agx_bo *alloc);

void agxdecode_dump_mappings(unsigned map_index);

void agxdecode_track_free(struct agx_bo *bo);

struct libagxdecode_config {
   uint32_t chip_id;
   size_t (*read_gpu_mem)(uint64_t addr, size_t size, void *data);
   ssize_t (*stream_write)(const char *buffer, size_t size);
};

void libagxdecode_init(struct libagxdecode_config *config);
void libagxdecode_vdm(uint64_t addr, const char *label, bool verbose);
void libagxdecode_cdm(uint64_t addr, const char *label, bool verbose);
void libagxdecode_usc(uint64_t addr, const char *label, bool verbose);
void libagxdecode_shutdown(void);

#endif /* __AGX_DECODE_H__ */
