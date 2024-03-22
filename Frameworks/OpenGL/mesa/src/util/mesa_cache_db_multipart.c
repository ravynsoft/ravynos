/*
 * Copyright © 2022 Collabora, Ltd.
 *
 * SPDX-License-Identifier: MIT
 */

#include <sys/stat.h>

#include "detect_os.h"
#include "string.h"
#include "mesa_cache_db_multipart.h"
#include "u_debug.h"

bool
mesa_cache_db_multipart_open(struct mesa_cache_db_multipart *db,
                             const char *cache_path)
{
#if DETECT_OS_WINDOWS
   return false;
#else
   char *part_path = NULL;
   unsigned int i;

   db->num_parts = debug_get_num_option("MESA_DISK_CACHE_DATABASE_NUM_PARTS", 50);

   db->parts = calloc(db->num_parts, sizeof(*db->parts));
   if (!db->parts)
      return false;

   for (i = 0; i < db->num_parts; i++) {
      bool db_opened = false;

      if (asprintf(&part_path, "%s/part%u", cache_path, i) == -1)
         goto close_db;

      if (mkdir(part_path, 0755) == -1 && errno != EEXIST)
         goto free_path;

      /* DB opening may fail only in a case of a severe problem,
       * like IO error.
       */
      db_opened = mesa_cache_db_open(&db->parts[i], part_path);
      if (!db_opened)
         goto free_path;

      free(part_path);
   }

   /* remove old pre multi-part cache */
   mesa_db_wipe_path(cache_path);

   return true;

free_path:
   free(part_path);
close_db:
   while (i--)
      mesa_cache_db_close(&db->parts[i]);

   free(db->parts);

   return false;
#endif
}

void
mesa_cache_db_multipart_close(struct mesa_cache_db_multipart *db)
{
   while (db->num_parts--)
      mesa_cache_db_close(&db->parts[db->num_parts]);

   free(db->parts);
}

void
mesa_cache_db_multipart_set_size_limit(struct mesa_cache_db_multipart *db,
                                       uint64_t max_cache_size)
{
   for (unsigned int i = 0; i < db->num_parts; i++)
      mesa_cache_db_set_size_limit(&db->parts[i],
                                   max_cache_size / db->num_parts);
}

void *
mesa_cache_db_multipart_read_entry(struct mesa_cache_db_multipart *db,
                                   const uint8_t *cache_key_160bit,
                                   size_t *size)
{
   unsigned last_read_part = db->last_read_part;

   for (unsigned int i = 0; i < db->num_parts; i++) {
      unsigned int part = (last_read_part + i) % db->num_parts;

      void *cache_item = mesa_cache_db_read_entry(&db->parts[part],
                                                  cache_key_160bit, size);
      if (cache_item) {
         /* Likely that the next entry lookup will hit the same DB part. */
         db->last_read_part = part;
         return cache_item;
      }
   }

   return NULL;
}

static unsigned
mesa_cache_db_multipart_select_victim_part(struct mesa_cache_db_multipart *db)
{
   double best_score = 0, score;
   unsigned victim = 0;

   for (unsigned int i = 0; i < db->num_parts; i++) {
      score = mesa_cache_db_eviction_score(&db->parts[i]);
      if (score > best_score) {
         best_score = score;
         victim = i;
      }
   }

   return victim;
}

bool
mesa_cache_db_multipart_entry_write(struct mesa_cache_db_multipart *db,
                                    const uint8_t *cache_key_160bit,
                                    const void *blob, size_t blob_size)
{
   unsigned last_written_part = db->last_written_part;
   int wpart = -1;

   for (unsigned int i = 0; i < db->num_parts; i++) {
      unsigned int part = (last_written_part + i) % db->num_parts;

      /* Note that each DB part has own locking. */
      if (mesa_cache_db_has_space(&db->parts[part], blob_size)) {
         wpart = part;
         break;
      }
   }

   /* All DB parts are full. Writing to a full DB part will auto-trigger
    * eviction of LRU cache entries from the part. Select DB part that
    * contains majority of LRU cache entries.
    */
   if (wpart < 0)
      wpart = mesa_cache_db_multipart_select_victim_part(db);

   db->last_written_part = wpart;

   return mesa_cache_db_entry_write(&db->parts[wpart], cache_key_160bit,
                                    blob, blob_size);
}

void
mesa_cache_db_multipart_entry_remove(struct mesa_cache_db_multipart *db,
                                     const uint8_t *cache_key_160bit)
{
   for (unsigned int i = 0; i < db->num_parts; i++)
      mesa_cache_db_entry_remove(&db->parts[i], cache_key_160bit);
}
