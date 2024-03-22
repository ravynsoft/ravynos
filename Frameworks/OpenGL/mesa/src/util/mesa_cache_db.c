/*
 * Copyright © 2022 Collabora, Ltd.
 *
 * Based on Fossilize DB:
 * Copyright © 2020 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "detect_os.h"

#if DETECT_OS_WINDOWS == 0

#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include "crc32.h"
#include "disk_cache.h"
#include "hash_table.h"
#include "mesa-sha1.h"
#include "mesa_cache_db.h"
#include "os_time.h"
#include "ralloc.h"
#include "u_debug.h"
#include "u_qsort.h"

#define MESA_CACHE_DB_VERSION          1
#define MESA_CACHE_DB_MAGIC            "MESA_DB"

struct PACKED mesa_db_file_header {
   char magic[8];
   uint32_t version;
   uint64_t uuid;
};

struct PACKED mesa_cache_db_file_entry {
   cache_key key;
   uint32_t crc;
   uint32_t size;
};

struct PACKED mesa_index_db_file_entry {
   uint64_t hash;
   uint32_t size;
   uint64_t last_access_time;
   uint64_t cache_db_file_offset;
};

struct mesa_index_db_hash_entry {
   uint64_t cache_db_file_offset;
   uint64_t index_db_file_offset;
   uint64_t last_access_time;
   uint32_t size;
   bool evicted;
};

static inline bool mesa_db_seek_end(FILE *file)
{
   return !fseek(file, 0, SEEK_END);
}

static inline bool mesa_db_seek(FILE *file, long pos)
{
   return !fseek(file, pos, SEEK_SET);
}

static inline bool mesa_db_seek_cur(FILE *file, long pos)
{
   return !fseek(file, pos, SEEK_CUR);
}

static inline bool mesa_db_read_data(FILE *file, void *data, size_t size)
{
   return fread(data, 1, size, file) == size;
}
#define mesa_db_read(file, var) mesa_db_read_data(file, var, sizeof(*(var)))

static inline bool mesa_db_write_data(FILE *file, const void *data, size_t size)
{
   return fwrite(data, 1, size, file) == size;
}
#define mesa_db_write(file, var) mesa_db_write_data(file, var, sizeof(*(var)))

static inline bool mesa_db_truncate(FILE *file, long pos)
{
   return !ftruncate(fileno(file), pos);
}

static bool
mesa_db_lock(struct mesa_cache_db *db)
{
   simple_mtx_lock(&db->flock_mtx);

   if (flock(fileno(db->cache.file), LOCK_EX) == -1)
      goto unlock_mtx;

   if (flock(fileno(db->index.file), LOCK_EX) == -1)
      goto unlock_cache;

   return true;

unlock_cache:
   flock(fileno(db->cache.file), LOCK_UN);
unlock_mtx:
   simple_mtx_unlock(&db->flock_mtx);

   return false;
}

static void
mesa_db_unlock(struct mesa_cache_db *db)
{
   flock(fileno(db->index.file), LOCK_UN);
   flock(fileno(db->cache.file), LOCK_UN);
   simple_mtx_unlock(&db->flock_mtx);
}

static uint64_t to_mesa_cache_db_hash(const uint8_t *cache_key_160bit)
{
   uint64_t hash = 0;

   for (unsigned i = 0; i < 8; i++)
      hash |= ((uint64_t)cache_key_160bit[i]) << i * 8;

   return hash;
}

static uint64_t
mesa_db_generate_uuid(void)
{
   /* This simple UUID implementation is sufficient for our needs
    * because UUID is updated rarely. It's nice to make UUID meaningful
    * and incremental by adding the timestamp to it, which also prevents
    * the potential collisions. */
   return ((os_time_get() / 1000000) << 32) | rand();
}

static bool
mesa_db_read_header(FILE *file, struct mesa_db_file_header *header)
{
   rewind(file);
   fflush(file);

   if (!mesa_db_read(file, header))
      return false;

   if (strncmp(header->magic, MESA_CACHE_DB_MAGIC, sizeof(header->magic)) ||
       header->version != MESA_CACHE_DB_VERSION || !header->uuid)
      return false;

   return true;
}

static bool
mesa_db_load_header(struct mesa_cache_db_file *db_file)
{
   struct mesa_db_file_header header;

   if (!mesa_db_read_header(db_file->file, &header))
      return false;

   db_file->uuid = header.uuid;

   return true;
}

static bool mesa_db_uuid_changed(struct mesa_cache_db *db)
{
   struct mesa_db_file_header cache_header;
   struct mesa_db_file_header index_header;

   if (!mesa_db_read_header(db->cache.file, &cache_header) ||
       !mesa_db_read_header(db->index.file, &index_header) ||
       cache_header.uuid != index_header.uuid ||
       cache_header.uuid != db->uuid)
      return true;

   return false;
}

static bool
mesa_db_write_header(struct mesa_cache_db_file *db_file,
                     uint64_t uuid, bool reset)
{
   struct mesa_db_file_header header;

   rewind(db_file->file);

   sprintf(header.magic, "MESA_DB");
   header.version = MESA_CACHE_DB_VERSION;
   header.uuid = uuid;

   if (!mesa_db_write(db_file->file, &header))
      return false;

   if (reset) {
      if (!mesa_db_truncate(db_file->file, ftell(db_file->file)))
         return false;
   }

   fflush(db_file->file);

   return true;
}

/* Wipe out all database cache files.
 *
 * Whenever we get an unmanageable error on reading or writing to the
 * database file, wipe out the whole database and start over. All the
 * cached entries will be lost, but the broken cache will be auto-repaired
 * reliably. Normally cache shall never get corrupted and losing cache
 * entries is acceptable, hence it's more practical to repair DB using
 * the simplest method.
 */
static bool
mesa_db_zap(struct mesa_cache_db *db)
{
   /* Disable cache to prevent the recurring faults */
   db->alive = false;

   /* Zap corrupted database files to start over from a clean slate */
   if (!mesa_db_truncate(db->cache.file, 0) ||
       !mesa_db_truncate(db->index.file, 0))
      return false;

   fflush(db->cache.file);
   fflush(db->index.file);

   return true;
}

static bool
mesa_db_index_entry_valid(struct mesa_index_db_file_entry *entry)
{
   return entry->size && entry->hash &&
          (int64_t)entry->cache_db_file_offset >= sizeof(struct mesa_db_file_header);
}

static bool
mesa_db_cache_entry_valid(struct mesa_cache_db_file_entry *entry)
{
   return entry->size && entry->crc;
}

static bool
mesa_db_update_index(struct mesa_cache_db *db)
{
   struct mesa_index_db_hash_entry *hash_entry;
   struct mesa_index_db_file_entry index_entry;
   size_t file_length;

   if (!mesa_db_seek_end(db->index.file))
      return false;

   file_length = ftell(db->index.file);

   if (!mesa_db_seek(db->index.file, db->index.offset))
      return false;

   while (db->index.offset < file_length) {
      if (!mesa_db_read(db->index.file, &index_entry))
         break;

      /* Check whether the index entry looks valid or we have a corrupted DB */
      if (!mesa_db_index_entry_valid(&index_entry))
         break;

      hash_entry = ralloc(db->mem_ctx, struct mesa_index_db_hash_entry);
      if (!hash_entry)
         break;

      hash_entry->cache_db_file_offset = index_entry.cache_db_file_offset;
      hash_entry->index_db_file_offset = db->index.offset;
      hash_entry->last_access_time = index_entry.last_access_time;
      hash_entry->size = index_entry.size;

      _mesa_hash_table_u64_insert(db->index_db, index_entry.hash, hash_entry);

      db->index.offset += sizeof(index_entry);
   }

   if (!mesa_db_seek(db->index.file, db->index.offset))
      return false;

   return db->index.offset == file_length;
}

static void
mesa_db_hash_table_reset(struct mesa_cache_db *db)
{
   _mesa_hash_table_u64_clear(db->index_db);
   ralloc_free(db->mem_ctx);
   db->mem_ctx = ralloc_context(NULL);
}

static bool
mesa_db_recreate_files(struct mesa_cache_db *db)
{
   db->uuid = mesa_db_generate_uuid();

   if (!mesa_db_write_header(&db->cache, db->uuid, true) ||
       !mesa_db_write_header(&db->index, db->uuid, true))
         return false;

   return true;
}

static bool
mesa_db_load(struct mesa_cache_db *db, bool reload)
{
   /* reloading must be done under the held lock */
   if (!reload) {
      if (!mesa_db_lock(db))
         return false;
   }

   /* If file headers are invalid, then zap database files and start over */
   if (!mesa_db_load_header(&db->cache) ||
       !mesa_db_load_header(&db->index) ||
       db->cache.uuid != db->index.uuid) {

      /* This is unexpected to happen on reload, bail out */
      if (reload)
         goto fail;

      if (!mesa_db_recreate_files(db))
         goto fail;
   } else {
      db->uuid = db->cache.uuid;
   }

   db->index.offset = ftell(db->index.file);

   if (reload)
      mesa_db_hash_table_reset(db);

   if (!mesa_db_update_index(db))
      goto fail;

   if (!reload)
      mesa_db_unlock(db);

   db->alive = true;

   return true;

fail:
   if (!reload)
      mesa_db_unlock(db);

   return false;
}

static bool
mesa_db_reload(struct mesa_cache_db *db)
{
   fflush(db->cache.file);
   fflush(db->index.file);

   return mesa_db_load(db, true);
}

static void
touch_file(const char* path)
{
   close(open(path, O_CREAT | O_CLOEXEC, 0644));
}

static bool
mesa_db_open_file(struct mesa_cache_db_file *db_file,
                  const char *cache_path,
                  const char *filename)
{
   if (asprintf(&db_file->path, "%s/%s", cache_path, filename) == -1)
      return false;

   /* The fopen("r+b") mode doesn't auto-create new file, hence we need to
    * explicitly create the file first.
    */
   touch_file(db_file->path);

   db_file->file = fopen(db_file->path, "r+b");
   if (!db_file->file) {
      free(db_file->path);
      return false;
   }

   return true;
}

static void
mesa_db_close_file(struct mesa_cache_db_file *db_file)
{
   fclose(db_file->file);
   free(db_file->path);
}

static bool
mesa_db_remove_file(struct mesa_cache_db_file *db_file,
                  const char *cache_path,
                  const char *filename)
{
   if (asprintf(&db_file->path, "%s/%s", cache_path, filename) == -1)
      return false;

   unlink(db_file->path);

   return true;
}

static int
entry_sort_lru(const void *_a, const void *_b, void *arg)
{
   const struct mesa_index_db_hash_entry *a = *((const struct mesa_index_db_hash_entry **)_a);
   const struct mesa_index_db_hash_entry *b = *((const struct mesa_index_db_hash_entry **)_b);

   /* In practice it's unlikely that we will get two entries with the
    * same timestamp, but technically it's possible to happen if OS
    * timer's resolution is low. */
   if (a->last_access_time == b->last_access_time)
      return 0;

   return a->last_access_time > b->last_access_time ? 1 : -1;
}

static int
entry_sort_offset(const void *_a, const void *_b, void *arg)
{
   const struct mesa_index_db_hash_entry *a = *((const struct mesa_index_db_hash_entry **)_a);
   const struct mesa_index_db_hash_entry *b = *((const struct mesa_index_db_hash_entry **)_b);
   struct mesa_cache_db *db = arg;

   /* Two entries will never have the identical offset, otherwise DB is
    * corrupted. */
   if (a->cache_db_file_offset == b->cache_db_file_offset)
      mesa_db_zap(db);

   return a->cache_db_file_offset > b->cache_db_file_offset ? 1 : -1;
}

static uint32_t blob_file_size(uint32_t blob_size)
{
   return sizeof(struct mesa_cache_db_file_entry) + blob_size;
}

static bool
mesa_db_compact(struct mesa_cache_db *db, int64_t blob_size,
                struct mesa_index_db_hash_entry *remove_entry)
{
   uint32_t num_entries, buffer_size = sizeof(struct mesa_index_db_file_entry);
   struct mesa_db_file_header cache_header, index_header;
   FILE *compacted_cache = NULL, *compacted_index = NULL;
   struct mesa_index_db_file_entry index_entry;
   struct mesa_index_db_hash_entry **entries;
   bool success = false, compact = false;
   void *buffer = NULL;
   unsigned int i = 0;

   /* reload index to sync the last access times */
   if (!remove_entry && !mesa_db_reload(db))
      return false;

   num_entries = _mesa_hash_table_num_entries(db->index_db->table);
   entries = calloc(num_entries, sizeof(*entries));
   if (!entries)
      return false;

   compacted_cache = fopen(db->cache.path, "r+b");
   compacted_index = fopen(db->index.path, "r+b");
   if (!compacted_cache || !compacted_index)
      goto cleanup;

   /* The database file has been replaced if UUID changed. We opened
    * some other cache, stop processing this database. */
   if (!mesa_db_read_header(compacted_cache, &cache_header) ||
       !mesa_db_read_header(compacted_index, &index_header) ||
       cache_header.uuid != db->uuid ||
       index_header.uuid != db->uuid)
      goto cleanup;

   hash_table_foreach(db->index_db->table, entry) {
      entries[i] = entry->data;
      entries[i]->evicted = (entries[i] == remove_entry);
      buffer_size = MAX2(buffer_size, blob_file_size(entries[i]->size));
      i++;
   }

   util_qsort_r(entries, num_entries, sizeof(*entries),
                entry_sort_lru, db);

   for (i = 0; blob_size > 0 && i < num_entries; i++) {
      blob_size -= blob_file_size(entries[i]->size);
      entries[i]->evicted = true;
   }

   util_qsort_r(entries, num_entries, sizeof(*entries),
                entry_sort_offset, db);

   /* entry_sort_offset() may zap the database */
   if (!db->alive)
      goto cleanup;

   buffer = malloc(buffer_size);
   if (!buffer)
      goto cleanup;

   /* Mark cache file invalid by writing zero-UUID header. If compaction will
    * fail, then the file will remain to be invalid since we can't repair it. */
   if (!mesa_db_write_header(&db->cache, 0, false) ||
       !mesa_db_write_header(&db->index, 0, false))
      goto cleanup;

   /* Sync the file pointers */
   if (!mesa_db_seek(compacted_cache, ftell(db->cache.file)) ||
       !mesa_db_seek(compacted_index, ftell(db->index.file)))
      goto cleanup;

   /* Do the compaction */
   for (i = 0; i < num_entries; i++) {
      blob_size = blob_file_size(entries[i]->size);

      /* Sanity-check the cache-read offset */
      if (ftell(db->cache.file) != entries[i]->cache_db_file_offset)
         goto cleanup;

      if (entries[i]->evicted) {
         /* Jump over the evicted entry */
         if (!mesa_db_seek_cur(db->cache.file, blob_size) ||
             !mesa_db_seek_cur(db->index.file, sizeof(index_entry)))
            goto cleanup;

         compact = true;
         continue;
      }

      if (compact) {
         /* Compact the cache file */
         if (!mesa_db_read_data(db->cache.file,   buffer, blob_size) ||
             !mesa_db_cache_entry_valid(buffer) ||
             !mesa_db_write_data(compacted_cache, buffer, blob_size))
            goto cleanup;

         /* Compact the index file */
         if (!mesa_db_read(db->index.file, &index_entry) ||
             !mesa_db_index_entry_valid(&index_entry) ||
             index_entry.cache_db_file_offset != entries[i]->cache_db_file_offset ||
             index_entry.size != entries[i]->size)
            goto cleanup;

         index_entry.cache_db_file_offset = ftell(compacted_cache) - blob_size;

         if (!mesa_db_write(compacted_index, &index_entry))
            goto cleanup;
      } else {
         /* Sanity-check the cache-write offset */
         if (ftell(compacted_cache) != entries[i]->cache_db_file_offset)
            goto cleanup;

         /* Jump over the unchanged entry */
         if (!mesa_db_seek_cur(db->index.file,  sizeof(index_entry)) ||
             !mesa_db_seek_cur(compacted_index, sizeof(index_entry)) ||
             !mesa_db_seek_cur(db->cache.file,  blob_size) ||
             !mesa_db_seek_cur(compacted_cache, blob_size))
            goto cleanup;
      }
   }

   fflush(compacted_cache);
   fflush(compacted_index);

   /* Cut off the the freed space left after compaction */
   if (!mesa_db_truncate(db->cache.file, ftell(compacted_cache)) ||
       !mesa_db_truncate(db->index.file, ftell(compacted_index)))
      goto cleanup;

   /* Set the new UUID to let all cache readers know that the cache was changed */
   db->uuid = mesa_db_generate_uuid();

   if (!mesa_db_write_header(&db->cache, db->uuid, false) ||
       !mesa_db_write_header(&db->index, db->uuid, false))
      goto cleanup;

   success = true;

cleanup:
   free(buffer);
   if (compacted_index)
      fclose(compacted_index);
   if (compacted_cache)
      fclose(compacted_cache);
   free(entries);

   /* reload compacted index */
   if (success && !mesa_db_reload(db))
      success = false;

   return success;
}

bool
mesa_cache_db_open(struct mesa_cache_db *db, const char *cache_path)
{
   if (!mesa_db_open_file(&db->cache, cache_path, "mesa_cache.db"))
      return false;

   if (!mesa_db_open_file(&db->index, cache_path, "mesa_cache.idx"))
      goto close_cache;

   db->mem_ctx = ralloc_context(NULL);
   if (!db->mem_ctx)
      goto close_index;

   simple_mtx_init(&db->flock_mtx, mtx_plain);

   db->index_db = _mesa_hash_table_u64_create(NULL);
   if (!db->index_db)
      goto destroy_mtx;

   if (!mesa_db_load(db, false))
      goto destroy_hash;

   return true;

destroy_hash:
   _mesa_hash_table_u64_destroy(db->index_db);
destroy_mtx:
   simple_mtx_destroy(&db->flock_mtx);

   ralloc_free(db->mem_ctx);
close_index:
   mesa_db_close_file(&db->index);
close_cache:
   mesa_db_close_file(&db->cache);

   return false;
}

bool
mesa_db_wipe_path(const char *cache_path)
{
   struct mesa_cache_db db = {0};
   bool success = true;

   if (!mesa_db_remove_file(&db.cache, cache_path, "mesa_cache.db") ||
       !mesa_db_remove_file(&db.index, cache_path, "mesa_cache.idx"))
      success = false;

   free(db.cache.path);
   free(db.index.path);

   return success;
}

void
mesa_cache_db_close(struct mesa_cache_db *db)
{
   _mesa_hash_table_u64_destroy(db->index_db);
   simple_mtx_destroy(&db->flock_mtx);
   ralloc_free(db->mem_ctx);

   mesa_db_close_file(&db->index);
   mesa_db_close_file(&db->cache);
}

void
mesa_cache_db_set_size_limit(struct mesa_cache_db *db,
                             uint64_t max_cache_size)
{
   db->max_cache_size = max_cache_size;
}

unsigned int
mesa_cache_db_file_entry_size(void)
{
   return sizeof(struct mesa_cache_db_file_entry);
}

void *
mesa_cache_db_read_entry(struct mesa_cache_db *db,
                         const uint8_t *cache_key_160bit,
                         size_t *size)
{
   uint64_t hash = to_mesa_cache_db_hash(cache_key_160bit);
   struct mesa_cache_db_file_entry cache_entry;
   struct mesa_index_db_file_entry index_entry;
   struct mesa_index_db_hash_entry *hash_entry;
   void *data = NULL;

   if (!mesa_db_lock(db))
      return NULL;

   if (!db->alive)
      goto fail;

   if (mesa_db_uuid_changed(db) && !mesa_db_reload(db))
      goto fail_fatal;

   if (!mesa_db_update_index(db))
      goto fail_fatal;

   hash_entry = _mesa_hash_table_u64_search(db->index_db, hash);
   if (!hash_entry)
      goto fail;

   if (!mesa_db_seek(db->cache.file, hash_entry->cache_db_file_offset) ||
       !mesa_db_read(db->cache.file, &cache_entry) ||
       !mesa_db_cache_entry_valid(&cache_entry))
      goto fail_fatal;

   if (memcmp(cache_entry.key, cache_key_160bit, sizeof(cache_entry.key)))
      goto fail;

   data = malloc(cache_entry.size);
   if (!data)
      goto fail;

   if (!mesa_db_read_data(db->cache.file, data, cache_entry.size) ||
       util_hash_crc32(data, cache_entry.size) != cache_entry.crc)
      goto fail_fatal;

   if (!mesa_db_seek(db->index.file, hash_entry->index_db_file_offset) ||
       !mesa_db_read(db->index.file, &index_entry) ||
       !mesa_db_index_entry_valid(&index_entry) ||
       index_entry.cache_db_file_offset != hash_entry->cache_db_file_offset ||
       index_entry.size != hash_entry->size)
      goto fail_fatal;

   index_entry.last_access_time = os_time_get_nano();
   hash_entry->last_access_time = index_entry.last_access_time;

   if (!mesa_db_seek(db->index.file, hash_entry->index_db_file_offset) ||
       !mesa_db_write(db->index.file, &index_entry))
      goto fail_fatal;

   fflush(db->index.file);

   mesa_db_unlock(db);

   *size = cache_entry.size;

   return data;

fail_fatal:
   mesa_db_zap(db);
fail:
   free(data);

   mesa_db_unlock(db);

   return NULL;
}

static bool
mesa_cache_db_has_space_locked(struct mesa_cache_db *db, size_t blob_size)
{
   return ftell(db->cache.file) + blob_file_size(blob_size) -
          sizeof(struct mesa_db_file_header) <= db->max_cache_size;
}

static size_t
mesa_cache_db_eviction_size(struct mesa_cache_db *db)
{
   return db->max_cache_size / 2 - sizeof(struct mesa_db_file_header);
}

bool
mesa_cache_db_entry_write(struct mesa_cache_db *db,
                          const uint8_t *cache_key_160bit,
                          const void *blob, size_t blob_size)
{
   uint64_t hash = to_mesa_cache_db_hash(cache_key_160bit);
   struct mesa_index_db_hash_entry *hash_entry = NULL;
   struct mesa_cache_db_file_entry cache_entry;
   struct mesa_index_db_file_entry index_entry;

   if (!mesa_db_lock(db))
      return false;

   if (!db->alive)
      goto fail;

   if (mesa_db_uuid_changed(db) && !mesa_db_reload(db))
      goto fail_fatal;

   if (!mesa_db_seek_end(db->cache.file))
      goto fail_fatal;

   if (!mesa_cache_db_has_space_locked(db, blob_size)) {
      if (!mesa_db_compact(db, MAX2(blob_size, mesa_cache_db_eviction_size(db)),
                           NULL))
         goto fail_fatal;
   } else {
      if (!mesa_db_update_index(db))
         goto fail_fatal;
   }

   hash_entry = _mesa_hash_table_u64_search(db->index_db, hash);
   if (hash_entry) {
      hash_entry = NULL;
      goto fail;
   }

   if (!mesa_db_seek_end(db->cache.file) ||
       !mesa_db_seek_end(db->index.file))
      goto fail_fatal;

   memcpy(cache_entry.key, cache_key_160bit, sizeof(cache_entry.key));
   cache_entry.crc = util_hash_crc32(blob, blob_size);
   cache_entry.size = blob_size;

   index_entry.hash = hash;
   index_entry.size = blob_size;
   index_entry.last_access_time = os_time_get_nano();
   index_entry.cache_db_file_offset = ftell(db->cache.file);

   hash_entry = ralloc(db->mem_ctx, struct mesa_index_db_hash_entry);
   if (!hash_entry)
      goto fail;

   hash_entry->cache_db_file_offset = index_entry.cache_db_file_offset;
   hash_entry->index_db_file_offset = ftell(db->index.file);
   hash_entry->last_access_time = index_entry.last_access_time;
   hash_entry->size = index_entry.size;

   if (!mesa_db_write(db->cache.file, &cache_entry) ||
       !mesa_db_write_data(db->cache.file, blob, blob_size) ||
       !mesa_db_write(db->index.file, &index_entry))
      goto fail_fatal;

   fflush(db->cache.file);
   fflush(db->index.file);

   db->index.offset = ftell(db->index.file);

   _mesa_hash_table_u64_insert(db->index_db, hash, hash_entry);

   mesa_db_unlock(db);

   return true;

fail_fatal:
   mesa_db_zap(db);
fail:
   mesa_db_unlock(db);

   if (hash_entry)
      ralloc_free(hash_entry);

   return false;
}

bool
mesa_cache_db_entry_remove(struct mesa_cache_db *db,
                           const uint8_t *cache_key_160bit)
{
   uint64_t hash = to_mesa_cache_db_hash(cache_key_160bit);
   struct mesa_cache_db_file_entry cache_entry;
   struct mesa_index_db_hash_entry *hash_entry;

   if (!mesa_db_lock(db))
      return NULL;

   if (!db->alive)
      goto fail;

   if (mesa_db_uuid_changed(db) && !mesa_db_reload(db))
      goto fail_fatal;

   if (!mesa_db_update_index(db))
      goto fail_fatal;

   hash_entry = _mesa_hash_table_u64_search(db->index_db, hash);
   if (!hash_entry)
      goto fail;

   if (!mesa_db_seek(db->cache.file, hash_entry->cache_db_file_offset) ||
       !mesa_db_read(db->cache.file, &cache_entry) ||
       !mesa_db_cache_entry_valid(&cache_entry))
      goto fail_fatal;

   if (memcmp(cache_entry.key, cache_key_160bit, sizeof(cache_entry.key)))
      goto fail;

   if (!mesa_db_compact(db, 0, hash_entry))
      goto fail_fatal;

   mesa_db_unlock(db);

   return true;

fail_fatal:
   mesa_db_zap(db);
fail:
   mesa_db_unlock(db);

   return false;
}

bool
mesa_cache_db_has_space(struct mesa_cache_db *db, size_t blob_size)
{
   bool has_space;

   if (!mesa_db_lock(db))
      return false;

   if (!mesa_db_seek_end(db->cache.file))
      goto fail_fatal;

   has_space = mesa_cache_db_has_space_locked(db, blob_size);

   mesa_db_unlock(db);

   return has_space;

fail_fatal:
   mesa_db_zap(db);
   mesa_db_unlock(db);

   return false;
}

static uint64_t
mesa_cache_db_eviction_2x_score_period(void)
{
   const uint64_t nsec_per_sec = 1000000000ull;
   static uint64_t period = 0;

   if (period)
      return period;

   period = debug_get_num_option("MESA_DISK_CACHE_DATABASE_EVICTION_SCORE_2X_PERIOD",
                                 30 * 24 * 60 * 60) * nsec_per_sec;

   return period;
}

double
mesa_cache_db_eviction_score(struct mesa_cache_db *db)
{
   int64_t eviction_size = mesa_cache_db_eviction_size(db);
   struct mesa_index_db_hash_entry **entries;
   unsigned num_entries, i = 0;
   double eviction_score = 0;

   if (!mesa_db_lock(db))
      return 0;

   if (!db->alive)
      goto fail;

   if (!mesa_db_reload(db))
      goto fail_fatal;

   num_entries = _mesa_hash_table_num_entries(db->index_db->table);
   entries = calloc(num_entries, sizeof(*entries));
   if (!entries)
      goto fail;

   hash_table_foreach(db->index_db->table, entry)
      entries[i++] = entry->data;

   util_qsort_r(entries, num_entries, sizeof(*entries),
                entry_sort_lru, db);

   for (i = 0; eviction_size > 0 && i < num_entries; i++) {
      uint64_t entry_age = os_time_get_nano() - entries[i]->last_access_time;
      unsigned entry_size = blob_file_size(entries[i]->size);

      /* Eviction score is a sum of weighted cache entry sizes,
       * where weight doubles for each month of entry's age.
       */
      uint64_t period = mesa_cache_db_eviction_2x_score_period();
      double entry_scale = 1 + (double)entry_age / period;
      double entry_score = entry_size * entry_scale;

      eviction_score += entry_score;
      eviction_size -= entry_size;
   }

   free(entries);

   mesa_db_unlock(db);

   return eviction_score;

fail_fatal:
   mesa_db_zap(db);
fail:
   mesa_db_unlock(db);

   return 0;
}

#endif /* DETECT_OS_WINDOWS */
