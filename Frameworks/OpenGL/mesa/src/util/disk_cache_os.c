/*
 * Copyright © 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "util/compress.h"
#include "util/crc32.h"
#include "util/u_debug.h"
#include "util/disk_cache.h"
#include "util/disk_cache_os.h"

#if DETECT_OS_WINDOWS

#include <windows.h>

bool
disk_cache_get_function_identifier(void *ptr, struct mesa_sha1 *ctx)
{
   HMODULE mod = NULL;
   GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCWSTR)ptr,
                      &mod);
   if (!mod)
      return false;

   WCHAR filename[MAX_PATH];
   DWORD filename_length = GetModuleFileNameW(mod, filename, ARRAY_SIZE(filename));

   if (filename_length == 0 || filename_length == ARRAY_SIZE(filename))
      return false;

   HANDLE mod_as_file = CreateFileW(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
   if (mod_as_file == INVALID_HANDLE_VALUE)
      return false;

   FILETIME time;
   bool ret = GetFileTime(mod_as_file, NULL, NULL, &time);
   if (ret)
      _mesa_sha1_update(ctx, &time, sizeof(time));
   CloseHandle(mod_as_file);
   return ret;
}

#endif

#ifdef ENABLE_SHADER_CACHE

#if DETECT_OS_WINDOWS
/* TODO: implement disk cache support on windows */

#else

#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util/blob.h"
#include "util/crc32.h"
#include "util/u_debug.h"
#include "util/ralloc.h"
#include "util/rand_xor.h"

/* Create a directory named 'path' if it does not already exist.
 *
 * Returns: 0 if path already exists as a directory or if created.
 *         -1 in all other cases.
 */
static int
mkdir_if_needed(const char *path)
{
   struct stat sb;

   /* If the path exists already, then our work is done if it's a
    * directory, but it's an error if it is not.
    */
   if (stat(path, &sb) == 0) {
      if (S_ISDIR(sb.st_mode)) {
         return 0;
      } else {
         fprintf(stderr, "Cannot use %s for shader cache (not a directory)"
                         "---disabling.\n", path);
         return -1;
      }
   }

   int ret = mkdir(path, 0700);
   if (ret == 0 || (ret == -1 && errno == EEXIST))
     return 0;

   fprintf(stderr, "Failed to create %s for shader cache (%s)---disabling.\n",
           path, strerror(errno));

   return -1;
}

/* Create a directory named 'path' if it does not already exist,
 * including parent directories if required.
 *
 * Returns: 0 if path already exists as a directory or if created.
 *         -1 in all other cases.
 */
static int
mkdir_with_parents_if_needed(const char *path)
{
   char *p;
   const char *end;

   if (path[0] == '\0')
      return -1;

   p = strdup(path);
   end = p + strlen(p) + 1; /* end points to the \0 terminator */
   for (char *q = p; q != end; q++) {
      if (*q == '/' || q == end - 1) {
         if (q == p) {
            /* Skip the first / of an absolute path. */
            continue;
         }

         *q = '\0';

         if (mkdir_if_needed(p) == -1) {
            free(p);
            return -1;
         }

         *q = '/';
      }
   }
   free(p);

   return 0;
}

/* Concatenate an existing path and a new name to form a new path.  If the new
 * path does not exist as a directory, create it then return the resulting
 * name of the new path (ralloc'ed off of 'ctx').
 *
 * Returns NULL on any error, such as:
 *
 *      <path> does not exist or is not a directory
 *      <path>/<name> exists but is not a directory
 *      <path>/<name> cannot be created as a directory
 */
static char *
concatenate_and_mkdir(void *ctx, const char *path, const char *name)
{
   char *new_path;
   struct stat sb;

   if (stat(path, &sb) != 0 || ! S_ISDIR(sb.st_mode))
      return NULL;

   new_path = ralloc_asprintf(ctx, "%s/%s", path, name);

   if (mkdir_if_needed(new_path) == 0)
      return new_path;
   else
      return NULL;
}

struct lru_file {
   struct list_head node;
   char *lru_name;
   size_t lru_file_size;
   time_t lru_atime;
};

static void
free_lru_file_list(struct list_head *lru_file_list)
{
   struct lru_file *e, *next;
   LIST_FOR_EACH_ENTRY_SAFE(e, next, lru_file_list, node) {
      free(e->lru_name);
      free(e);
   }
   free(lru_file_list);
}

/* Given a directory path and predicate function, create a linked list of entrys
 * with the oldest access time in that directory for which the predicate
 * returns true.
 *
 * Returns: A malloc'ed linkd list for the paths of chosen files, (or
 * NULL on any error). The caller should free the linked list via
 * free_lru_file_list() when finished.
 */
static struct list_head *
choose_lru_file_matching(const char *dir_path,
                         bool (*predicate)(const char *dir_path,
                                           const struct stat *,
                                           const char *, const size_t))
{
   DIR *dir;
   struct dirent *dir_ent;

   dir = opendir(dir_path);
   if (dir == NULL)
      return NULL;

   const int dir_fd = dirfd(dir);

   /* First count the number of files in the directory */
   unsigned total_file_count = 0;
   while ((dir_ent = readdir(dir)) != NULL) {
#ifdef HAVE_DIRENT_D_TYPE
      if (dir_ent->d_type == DT_REG) { /* If the entry is a regular file */
         total_file_count++;
      }
#else
      struct stat st;

      if (fstatat(dir_fd, dir_ent->d_name, &st, AT_SYMLINK_NOFOLLOW) == 0) {
         if (S_ISREG(st.st_mode)) {
            total_file_count++;
         }
      }
#endif
   }

   /* Reset to the start of the directory */
   rewinddir(dir);

   /* Collect 10% of files in this directory for removal. Note: This should work
    * out to only be around 0.04% of total cache items.
    */
   unsigned lru_file_count = total_file_count > 10 ? total_file_count / 10 : 1;
   struct list_head *lru_file_list = malloc(sizeof(struct list_head));
   list_inithead(lru_file_list);

   unsigned processed_files = 0;
   while (1) {
      dir_ent = readdir(dir);
      if (dir_ent == NULL)
         break;

      struct stat sb;
      if (fstatat(dir_fd, dir_ent->d_name, &sb, 0) == 0) {
         struct lru_file *entry = NULL;
         if (!list_is_empty(lru_file_list))
            entry = list_first_entry(lru_file_list, struct lru_file, node);

         if (!entry|| sb.st_atime < entry->lru_atime) {
            size_t len = strlen(dir_ent->d_name);
            if (!predicate(dir_path, &sb, dir_ent->d_name, len))
               continue;

            bool new_entry = false;
            if (processed_files < lru_file_count) {
               entry = calloc(1, sizeof(struct lru_file));
               new_entry = true;
            }
            processed_files++;

            char *tmp = realloc(entry->lru_name, len + 1);
            if (tmp) {
               /* Find location to insert new lru item. We want to keep the
                * list ordering from most recently used to least recently used.
                * This allows us to just evict the head item from the list as
                * we process the directory and find older entrys.
                */
               struct list_head *list_node = lru_file_list;
               struct lru_file *e;
               LIST_FOR_EACH_ENTRY(e, lru_file_list, node) {
                  if (sb.st_atime < entry->lru_atime) {
                     list_node = &e->node;
                     break;
                  }
               }

               if (new_entry) {
                  list_addtail(&entry->node, list_node);
               } else {
                  if (list_node != lru_file_list) {
                     list_del(lru_file_list);
                     list_addtail(lru_file_list, list_node);
                  }
               }

               entry->lru_name = tmp;
               memcpy(entry->lru_name, dir_ent->d_name, len + 1);
               entry->lru_atime = sb.st_atime;
               entry->lru_file_size = sb.st_blocks * 512;
            }
         }
      }
   }

   if (list_is_empty(lru_file_list)) {
      closedir(dir);
      free(lru_file_list);
      return NULL;
   }

   /* Create the full path for the file list we found */
   struct lru_file *e;
   LIST_FOR_EACH_ENTRY(e, lru_file_list, node) {
      char *filename = e->lru_name;
      if (asprintf(&e->lru_name, "%s/%s", dir_path, filename) < 0)
         e->lru_name = NULL;

      free(filename);
   }

   closedir(dir);

   return lru_file_list;
}

/* Is entry a regular file, and not having a name with a trailing
 * ".tmp"
 */
static bool
is_regular_non_tmp_file(const char *path, const struct stat *sb,
                        const char *d_name, const size_t len)
{
   if (!S_ISREG(sb->st_mode))
      return false;

   if (len >= 4 && strcmp(&d_name[len-4], ".tmp") == 0)
      return false;

   return true;
}

/* Returns the size of the deleted file, (or 0 on any error). */
static size_t
unlink_lru_file_from_directory(const char *path)
{
   struct list_head *lru_file_list =
      choose_lru_file_matching(path, is_regular_non_tmp_file);
   if (lru_file_list == NULL)
      return 0;

   assert(!list_is_empty(lru_file_list));

   size_t total_unlinked_size = 0;
   struct lru_file *e;
   LIST_FOR_EACH_ENTRY(e, lru_file_list, node) {
      if (unlink(e->lru_name) == 0)
         total_unlinked_size += e->lru_file_size;
   }
   free_lru_file_list(lru_file_list);

   return total_unlinked_size;
}

/* Is entry a directory with a two-character name, (and not the
 * special name of ".."). We also return false if the dir is empty.
 */
static bool
is_two_character_sub_directory(const char *path, const struct stat *sb,
                               const char *d_name, const size_t len)
{
   if (!S_ISDIR(sb->st_mode))
      return false;

   if (len != 2)
      return false;

   if (strcmp(d_name, "..") == 0)
      return false;

   char *subdir;
   if (asprintf(&subdir, "%s/%s", path, d_name) == -1)
      return false;
   DIR *dir = opendir(subdir);
   free(subdir);

   if (dir == NULL)
     return false;

   unsigned subdir_entries = 0;
   struct dirent *d;
   while ((d = readdir(dir)) != NULL) {
      if(++subdir_entries > 2)
         break;
   }
   closedir(dir);

   /* If dir only contains '.' and '..' it must be empty */
   if (subdir_entries <= 2)
      return false;

   return true;
}

/* Create the directory that will be needed for the cache file for \key.
 *
 * Obviously, the implementation here must closely match
 * _get_cache_file above.
*/
static void
make_cache_file_directory(struct disk_cache *cache, const cache_key key)
{
   char *dir;
   char buf[41];

   _mesa_sha1_format(buf, key);
   if (asprintf(&dir, "%s/%c%c", cache->path, buf[0], buf[1]) == -1)
      return;

   mkdir_if_needed(dir);
   free(dir);
}

static ssize_t
read_all(int fd, void *buf, size_t count)
{
   char *in = buf;
   ssize_t read_ret;
   size_t done;

   for (done = 0; done < count; done += read_ret) {
      read_ret = read(fd, in + done, count - done);
      if (read_ret == -1 || read_ret == 0)
         return -1;
   }
   return done;
}

static ssize_t
write_all(int fd, const void *buf, size_t count)
{
   const char *out = buf;
   ssize_t written;
   size_t done;

   for (done = 0; done < count; done += written) {
      written = write(fd, out + done, count - done);
      if (written == -1)
         return -1;
   }
   return done;
}

/* Evict least recently used cache item */
void
disk_cache_evict_lru_item(struct disk_cache *cache)
{
   char *dir_path;

   /* With a reasonably-sized, full cache, (and with keys generated
    * from a cryptographic hash), we can choose two random hex digits
    * and reasonably expect the directory to exist with a file in it.
    * Provides pseudo-LRU eviction to reduce checking all cache files.
    */
   uint64_t rand64 = rand_xorshift128plus(cache->seed_xorshift128plus);
   if (asprintf(&dir_path, "%s/%02" PRIx64 , cache->path, rand64 & 0xff) < 0)
      return;

   size_t size = unlink_lru_file_from_directory(dir_path);

   free(dir_path);

   if (size) {
      p_atomic_add(&cache->size->value, - (uint64_t)size);
      return;
   }

   /* In the case where the random choice of directory didn't find
    * something, we choose the least recently accessed from the
    * existing directories.
    *
    * Really, the only reason this code exists is to allow the unit
    * tests to work, (which use an artificially-small cache to be able
    * to force a single cached item to be evicted).
    */
   struct list_head *lru_file_list =
      choose_lru_file_matching(cache->path, is_two_character_sub_directory);
   if (lru_file_list == NULL)
      return;

   assert(!list_is_empty(lru_file_list));

   struct lru_file *lru_file_dir =
      list_first_entry(lru_file_list, struct lru_file, node);

   size = unlink_lru_file_from_directory(lru_file_dir->lru_name);

   free_lru_file_list(lru_file_list);

   if (size)
      p_atomic_add(&cache->size->value, - (uint64_t)size);
}

void
disk_cache_evict_item(struct disk_cache *cache, char *filename)
{
   struct stat sb;
   if (stat(filename, &sb) == -1) {
      free(filename);
      return;
   }

   unlink(filename);
   free(filename);

   if (sb.st_blocks)
      p_atomic_add(&cache->size->value, - (uint64_t)sb.st_blocks * 512);
}

static void *
parse_and_validate_cache_item(struct disk_cache *cache, void *cache_item,
                              size_t cache_item_size, size_t *size)
{
   uint8_t *uncompressed_data = NULL;

   struct blob_reader ci_blob_reader;
   blob_reader_init(&ci_blob_reader, cache_item, cache_item_size);

   size_t header_size = cache->driver_keys_blob_size;
   const void *keys_blob = blob_read_bytes(&ci_blob_reader, header_size);
   if (ci_blob_reader.overrun)
      goto fail;

   /* Check for extremely unlikely hash collisions */
   if (memcmp(cache->driver_keys_blob, keys_blob, header_size) != 0) {
      assert(!"Mesa cache keys mismatch!");
      goto fail;
   }

   uint32_t md_type = blob_read_uint32(&ci_blob_reader);
   if (ci_blob_reader.overrun)
      goto fail;

   if (md_type == CACHE_ITEM_TYPE_GLSL) {
      uint32_t num_keys = blob_read_uint32(&ci_blob_reader);
      if (ci_blob_reader.overrun)
         goto fail;

      /* The cache item metadata is currently just used for distributing
       * precompiled shaders, they are not used by Mesa so just skip them for
       * now.
       * TODO: pass the metadata back to the caller and do some basic
       * validation.
       */
      const void UNUSED *metadata =
         blob_read_bytes(&ci_blob_reader, num_keys * sizeof(cache_key));
      if (ci_blob_reader.overrun)
         goto fail;
   }

   /* Load the CRC that was created when the file was written. */
   struct cache_entry_file_data *cf_data =
      (struct cache_entry_file_data *)
         blob_read_bytes(&ci_blob_reader, sizeof(struct cache_entry_file_data));
   if (ci_blob_reader.overrun)
      goto fail;

   size_t cache_data_size = ci_blob_reader.end - ci_blob_reader.current;
   const uint8_t *data = (uint8_t *) blob_read_bytes(&ci_blob_reader, cache_data_size);

   /* Check the data for corruption */
   if (cf_data->crc32 != util_hash_crc32(data, cache_data_size))
      goto fail;

   /* Uncompress the cache data */
   uncompressed_data = malloc(cf_data->uncompressed_size);
   if (!uncompressed_data)
      goto fail;

   if (cache->compression_disabled) {
      if (cf_data->uncompressed_size != cache_data_size)
         goto fail;

      memcpy(uncompressed_data, data, cache_data_size);
   } else {
      if (!util_compress_inflate(data, cache_data_size, uncompressed_data,
                                 cf_data->uncompressed_size))
         goto fail;
   }

   if (size)
      *size = cf_data->uncompressed_size;

   return uncompressed_data;

 fail:
   if (uncompressed_data)
      free(uncompressed_data);

   return NULL;
}

void *
disk_cache_load_item(struct disk_cache *cache, char *filename, size_t *size)
{
   uint8_t *data = NULL;

   int fd = open(filename, O_RDONLY | O_CLOEXEC);
   if (fd == -1)
      goto fail;

   struct stat sb;
   if (fstat(fd, &sb) == -1)
      goto fail;

   data = malloc(sb.st_size);
   if (data == NULL)
      goto fail;

   /* Read entire file into memory */
   int ret = read_all(fd, data, sb.st_size);
   if (ret == -1)
      goto fail;

    uint8_t *uncompressed_data =
       parse_and_validate_cache_item(cache, data, sb.st_size, size);
   if (!uncompressed_data)
      goto fail;

   free(data);
   free(filename);
   close(fd);

   return uncompressed_data;

 fail:
   if (data)
      free(data);
   if (filename)
      free(filename);
   if (fd != -1)
      close(fd);

   return NULL;
}

/* Return a filename within the cache's directory corresponding to 'key'.
 *
 * Returns NULL if out of memory.
 */
char *
disk_cache_get_cache_filename(struct disk_cache *cache, const cache_key key)
{
   char buf[41];
   char *filename;

   if (cache->path_init_failed)
      return NULL;

   _mesa_sha1_format(buf, key);
   if (asprintf(&filename, "%s/%c%c/%s", cache->path, buf[0],
                buf[1], buf + 2) == -1)
      return NULL;

   return filename;
}

static bool
create_cache_item_header_and_blob(struct disk_cache_put_job *dc_job,
                                  struct blob *cache_blob)
{

   /* Compress the cache item data */
   size_t max_buf = util_compress_max_compressed_len(dc_job->size);
   size_t compressed_size;
   void *compressed_data;

   if (dc_job->cache->compression_disabled) {
      compressed_size = dc_job->size;
      compressed_data = dc_job->data;
   } else {
      compressed_data = malloc(max_buf);
      if (compressed_data == NULL)
         return false;
      compressed_size =
         util_compress_deflate(dc_job->data, dc_job->size,
                              compressed_data, max_buf);
      if (compressed_size == 0)
         goto fail;
   }

   /* Copy the driver_keys_blob, this can be used find information about the
    * mesa version that produced the entry or deal with hash collisions,
    * should that ever become a real problem.
    */
   if (!blob_write_bytes(cache_blob, dc_job->cache->driver_keys_blob,
                         dc_job->cache->driver_keys_blob_size))
      goto fail;

   /* Write the cache item metadata. This data can be used to deal with
    * hash collisions, as well as providing useful information to 3rd party
    * tools reading the cache files.
    */
   if (!blob_write_uint32(cache_blob, dc_job->cache_item_metadata.type))
      goto fail;

   if (dc_job->cache_item_metadata.type == CACHE_ITEM_TYPE_GLSL) {
      if (!blob_write_uint32(cache_blob, dc_job->cache_item_metadata.num_keys))
         goto fail;

      size_t metadata_keys_size =
         dc_job->cache_item_metadata.num_keys * sizeof(cache_key);
      if (!blob_write_bytes(cache_blob, dc_job->cache_item_metadata.keys[0],
                            metadata_keys_size))
         goto fail;
   }

   /* Create CRC of the compressed data. We will read this when restoring the
    * cache and use it to check for corruption.
    */
   struct cache_entry_file_data cf_data;
   cf_data.crc32 = util_hash_crc32(compressed_data, compressed_size);
   cf_data.uncompressed_size = dc_job->size;

   if (!blob_write_bytes(cache_blob, &cf_data, sizeof(cf_data)))
      goto fail;

   /* Finally copy the compressed cache blob */
   if (!blob_write_bytes(cache_blob, compressed_data, compressed_size))
      goto fail;

   if (!dc_job->cache->compression_disabled)
      free(compressed_data);

   return true;

 fail:
   if (!dc_job->cache->compression_disabled)
      free(compressed_data);

   return false;
}

void
disk_cache_write_item_to_disk(struct disk_cache_put_job *dc_job,
                              char *filename)
{
   int fd = -1, fd_final = -1;
   struct blob cache_blob;
   blob_init(&cache_blob);

   /* Write to a temporary file to allow for an atomic rename to the
    * final destination filename, (to prevent any readers from seeing
    * a partially written file).
    */
   char *filename_tmp = NULL;
   if (asprintf(&filename_tmp, "%s.tmp", filename) == -1)
      goto done;

   fd = open(filename_tmp, O_WRONLY | O_CLOEXEC | O_CREAT, 0644);

   /* Make the two-character subdirectory within the cache as needed. */
   if (fd == -1) {
      if (errno != ENOENT)
         goto done;

      make_cache_file_directory(dc_job->cache, dc_job->key);

      fd = open(filename_tmp, O_WRONLY | O_CLOEXEC | O_CREAT, 0644);
      if (fd == -1)
         goto done;
   }

   /* With the temporary file open, we take an exclusive flock on
    * it. If the flock fails, then another process still has the file
    * open with the flock held. So just let that file be responsible
    * for writing the file.
    */
#ifdef HAVE_FLOCK
   int err = flock(fd, LOCK_EX | LOCK_NB);
#else
   struct flock lock = {
      .l_start = 0,
      .l_len = 0, /* entire file */
      .l_type = F_WRLCK,
      .l_whence = SEEK_SET
   };
   int err = fcntl(fd, F_SETLK, &lock);
#endif
   if (err == -1)
      goto done;

   /* Now that we have the lock on the open temporary file, we can
    * check to see if the destination file already exists. If so,
    * another process won the race between when we saw that the file
    * didn't exist and now. In this case, we don't do anything more,
    * (to ensure the size accounting of the cache doesn't get off).
    */
   fd_final = open(filename, O_RDONLY | O_CLOEXEC);
   if (fd_final != -1) {
      unlink(filename_tmp);
      goto done;
   }

   /* OK, we're now on the hook to write out a file that we know is
    * not in the cache, and is also not being written out to the cache
    * by some other process.
    */
   if (!create_cache_item_header_and_blob(dc_job, &cache_blob)) {
      unlink(filename_tmp);
      goto done;
   }

   /* Now, finally, write out the contents to the temporary file, then
    * rename them atomically to the destination filename, and also
    * perform an atomic increment of the total cache size.
    */
   int ret = write_all(fd, cache_blob.data, cache_blob.size);
   if (ret == -1) {
      unlink(filename_tmp);
      goto done;
   }

   ret = rename(filename_tmp, filename);
   if (ret == -1) {
      unlink(filename_tmp);
      goto done;
   }

   struct stat sb;
   if (stat(filename, &sb) == -1) {
      /* Something went wrong remove the file */
      unlink(filename);
      goto done;
   }

   p_atomic_add(&dc_job->cache->size->value, sb.st_blocks * 512);

 done:
   if (fd_final != -1)
      close(fd_final);
   /* This close finally releases the flock, (now that the final file
    * has been renamed into place and the size has been added).
    */
   if (fd != -1)
      close(fd);
   free(filename_tmp);
   blob_finish(&cache_blob);
}

/* Determine path for cache based on the first defined name as follows:
 *
 *   $MESA_SHADER_CACHE_DIR
 *   $XDG_CACHE_HOME/mesa_shader_cache
 *   <pwd.pw_dir>/.cache/mesa_shader_cache
 */
char *
disk_cache_generate_cache_dir(void *mem_ctx, const char *gpu_name,
                              const char *driver_id,
                              enum disk_cache_type cache_type)
{
   char *cache_dir_name = CACHE_DIR_NAME;
   if (cache_type == DISK_CACHE_SINGLE_FILE)
      cache_dir_name = CACHE_DIR_NAME_SF;
   else if (cache_type == DISK_CACHE_DATABASE)
      cache_dir_name = CACHE_DIR_NAME_DB;

   char *path = secure_getenv("MESA_SHADER_CACHE_DIR");

   if (!path) {
      path = secure_getenv("MESA_GLSL_CACHE_DIR");
      if (path)
         fprintf(stderr,
                 "*** MESA_GLSL_CACHE_DIR is deprecated; "
                 "use MESA_SHADER_CACHE_DIR instead ***\n");
   }

   if (path) {
      if (mkdir_with_parents_if_needed(path) == -1)
         return NULL;

      path = concatenate_and_mkdir(mem_ctx, path, cache_dir_name);
      if (!path)
         return NULL;
   }

   if (path == NULL) {
      char *xdg_cache_home = secure_getenv("XDG_CACHE_HOME");

      if (xdg_cache_home) {
         if (mkdir_if_needed(xdg_cache_home) == -1)
            return NULL;

         path = concatenate_and_mkdir(mem_ctx, xdg_cache_home, cache_dir_name);
         if (!path)
            return NULL;
      }
   }

   if (!path) {
      char *buf;
      size_t buf_size;
      struct passwd pwd, *result;

      buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
      if (buf_size == -1)
         buf_size = 512;

      /* Loop until buf_size is large enough to query the directory */
      while (1) {
         buf = ralloc_size(mem_ctx, buf_size);

         getpwuid_r(getuid(), &pwd, buf, buf_size, &result);
         if (result)
            break;

         if (errno == ERANGE) {
            ralloc_free(buf);
            buf = NULL;
            buf_size *= 2;
         } else {
            return NULL;
         }
      }

      path = concatenate_and_mkdir(mem_ctx, pwd.pw_dir, ".cache");
      if (!path)
         return NULL;

      path = concatenate_and_mkdir(mem_ctx, path, cache_dir_name);
      if (!path)
         return NULL;
   }

   if (cache_type == DISK_CACHE_SINGLE_FILE) {
      path = concatenate_and_mkdir(mem_ctx, path, driver_id);
      if (!path)
         return NULL;

      path = concatenate_and_mkdir(mem_ctx, path, gpu_name);
      if (!path)
         return NULL;
   }

   return path;
}

bool
disk_cache_enabled()
{
   /* Disk cache is not enabled for android, but android's EGL layer
    * uses EGL_ANDROID_blob_cache to manage the cache itself:
    */
   if (DETECT_OS_ANDROID)
      return false;

   /* If running as a users other than the real user disable cache */
   if (!__normal_user())
      return false;

   /* At user request, disable shader cache entirely. */
#ifdef SHADER_CACHE_DISABLE_BY_DEFAULT
   bool disable_by_default = true;
#else
   bool disable_by_default = false;
#endif
   char *envvar_name = "MESA_SHADER_CACHE_DISABLE";
   if (!getenv(envvar_name)) {
      envvar_name = "MESA_GLSL_CACHE_DISABLE";
      if (getenv(envvar_name))
         fprintf(stderr,
                 "*** MESA_GLSL_CACHE_DISABLE is deprecated; "
                 "use MESA_SHADER_CACHE_DISABLE instead ***\n");
   }

   if (debug_get_bool_option(envvar_name, disable_by_default))
      return false;

   return true;
}

void *
disk_cache_load_item_foz(struct disk_cache *cache, const cache_key key,
                         size_t *size)
{
   size_t cache_tem_size = 0;
   void *cache_item = foz_read_entry(&cache->foz_db, key, &cache_tem_size);
   if (!cache_item)
      return NULL;

   uint8_t *uncompressed_data =
       parse_and_validate_cache_item(cache, cache_item, cache_tem_size, size);
   free(cache_item);

   return uncompressed_data;
}

bool
disk_cache_write_item_to_disk_foz(struct disk_cache_put_job *dc_job)
{
   struct blob cache_blob;
   blob_init(&cache_blob);

   if (!create_cache_item_header_and_blob(dc_job, &cache_blob))
      return false;

   bool r = foz_write_entry(&dc_job->cache->foz_db, dc_job->key,
                            cache_blob.data, cache_blob.size);

   blob_finish(&cache_blob);
   return r;
}

bool
disk_cache_load_cache_index_foz(void *mem_ctx, struct disk_cache *cache)
{
   /* Load cache index into a hash map (from fossilise files) */
   return foz_prepare(&cache->foz_db, cache->path);
}

bool
disk_cache_mmap_cache_index(void *mem_ctx, struct disk_cache *cache,
                            char *path)
{
   int fd = -1;
   bool mapped = false;

   path = ralloc_asprintf(mem_ctx, "%s/index", cache->path);
   if (path == NULL)
      goto path_fail;

   fd = open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0644);
   if (fd == -1)
      goto path_fail;

   struct stat sb;
   if (fstat(fd, &sb) == -1)
      goto path_fail;

   /* Force the index file to be the expected size. */
   size_t size = sizeof(*cache->size) + CACHE_INDEX_MAX_KEYS * CACHE_KEY_SIZE;
   if (sb.st_size != size) {
#if HAVE_POSIX_FALLOCATE
      /* posix_fallocate() ensures disk space is allocated otherwise it
       * fails if there is not enough space on the disk.
       */
      if (posix_fallocate(fd, 0, size) != 0)
         goto path_fail;
#else
      /* ftruncate() allocates disk space lazily. If the disk is full
       * and it is unable to allocate disk space when accessed via
       * mmap, it will crash with a SIGBUS.
       */
      if (ftruncate(fd, size) == -1)
         goto path_fail;
#endif
   }

   /* We map this shared so that other processes see updates that we
    * make.
    *
    * Note: We do use atomic addition to ensure that multiple
    * processes don't scramble the cache size recorded in the
    * index. But we don't use any locking to prevent multiple
    * processes from updating the same entry simultaneously. The idea
    * is that if either result lands entirely in the index, then
    * that's equivalent to a well-ordered write followed by an
    * eviction and a write. On the other hand, if the simultaneous
    * writes result in a corrupt entry, that's not really any
    * different than both entries being evicted, (since within the
    * guarantees of the cryptographic hash, a corrupt entry is
    * unlikely to ever match a real cache key).
    */
   cache->index_mmap = mmap(NULL, size, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
   if (cache->index_mmap == MAP_FAILED)
      goto path_fail;
   cache->index_mmap_size = size;

   cache->size = (p_atomic_uint64_t *) cache->index_mmap;
   cache->stored_keys = cache->index_mmap + sizeof(uint64_t);
   mapped = true;

path_fail:
   if (fd != -1)
      close(fd);

   return mapped;
}

void
disk_cache_destroy_mmap(struct disk_cache *cache)
{
   munmap(cache->index_mmap, cache->index_mmap_size);
}

void *
disk_cache_db_load_item(struct disk_cache *cache, const cache_key key,
                        size_t *size)
{
   size_t cache_tem_size = 0;
   void *cache_item = mesa_cache_db_multipart_read_entry(&cache->cache_db,
                                                         key, &cache_tem_size);
   if (!cache_item)
      return NULL;

   uint8_t *uncompressed_data =
       parse_and_validate_cache_item(cache, cache_item, cache_tem_size, size);
   free(cache_item);

   return uncompressed_data;
}

bool
disk_cache_db_write_item_to_disk(struct disk_cache_put_job *dc_job)
{
   struct blob cache_blob;
   blob_init(&cache_blob);

   if (!create_cache_item_header_and_blob(dc_job, &cache_blob))
      return false;

   bool r = mesa_cache_db_multipart_entry_write(&dc_job->cache->cache_db,
                                                dc_job->key, cache_blob.data,
                                                cache_blob.size);

   blob_finish(&cache_blob);
   return r;
}

bool
disk_cache_db_load_cache_index(void *mem_ctx, struct disk_cache *cache)
{
   return mesa_cache_db_multipart_open(&cache->cache_db, cache->path);
}
#endif

#endif /* ENABLE_SHADER_CACHE */
