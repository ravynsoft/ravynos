/*
 * Copyright © 2022 Collabora, Ltd.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef MESA_CACHE_DB_MULTIPART_H
#define MESA_CACHE_DB_MULTIPART_H

#include "mesa_cache_db.h"

struct mesa_cache_db_multipart {
   struct mesa_cache_db *parts;
   unsigned int num_parts;
   volatile unsigned int last_read_part;
   volatile unsigned int last_written_part;
};

bool
mesa_cache_db_multipart_open(struct mesa_cache_db_multipart *db,
                             const char *cache_path);

void
mesa_cache_db_multipart_close(struct mesa_cache_db_multipart *db);

void
mesa_cache_db_multipart_set_size_limit(struct mesa_cache_db_multipart *db,
                                       uint64_t max_cache_size);

void *
mesa_cache_db_multipart_read_entry(struct mesa_cache_db_multipart *db,
                                   const uint8_t *cache_key_160bit,
                                   size_t *size);

bool
mesa_cache_db_multipart_entry_write(struct mesa_cache_db_multipart *db,
                                    const uint8_t *cache_key_160bit,
                                    const void *blob, size_t blob_size);

void
mesa_cache_db_multipart_entry_remove(struct mesa_cache_db_multipart *db,
                                     const uint8_t *cache_key_160bit);

#endif /* MESA_CACHE_DB_MULTIPART_H */
