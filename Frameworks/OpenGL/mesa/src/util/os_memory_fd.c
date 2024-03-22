/**************************************************************************
 *
 * Copyright 2021 Snap Inc.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/*
 * Memory fd wrappers.
 */

#include "detect_os.h"

#if DETECT_OS_UNIX

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "anon_file.h"
#include "mesa-sha1.h"
#include "u_math.h"
#include "os_memory.h"

/* (Re)define UUID_SIZE to avoid including vulkan.h (or p_defines.h) here. */
#define UUID_SIZE 16

struct memory_header {
   size_t size;
   size_t offset;
   uint8_t uuid[UUID_SIZE];
};

static void
get_driver_id_sha1_hash(uint8_t sha1[SHA1_DIGEST_LENGTH], const char *driver_id) {
   struct mesa_sha1 sha1_ctx;
   _mesa_sha1_init(&sha1_ctx);

   _mesa_sha1_update(&sha1_ctx, driver_id, strlen(driver_id));

   _mesa_sha1_final(&sha1_ctx, sha1);
}

/**
 * Imports memory from a file descriptor
 */
bool
os_import_memory_fd(int fd, void **ptr, uint64_t *size, char const *driver_id)
{
   void *mapped_ptr;
   struct memory_header header;

   lseek(fd, 0, SEEK_SET);
   int bytes_read = read(fd, &header, sizeof(header));
   if(bytes_read != sizeof(header))
      return false;

   // Check the uuid we put after the sizes in order to verify that the fd
   // is a memfd that we created and not some random fd.
   uint8_t sha1[SHA1_DIGEST_LENGTH];
   get_driver_id_sha1_hash(sha1, driver_id);

   assert(SHA1_DIGEST_LENGTH >= UUID_SIZE);
   if (memcmp(header.uuid, sha1, UUID_SIZE)) {
      return false;
   }

   mapped_ptr = mmap(NULL, header.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (mapped_ptr == MAP_FAILED) {
      return false;
   }
   *ptr = (void*)((uintptr_t)mapped_ptr + header.offset);
   // the offset does not count as part of the size
   *size = header.size - header.offset;
   return true;
}

/**
 * Return memory on given byte alignment
 */
void *
os_malloc_aligned_fd(size_t size, size_t alignment, int *fd, char const *fd_name, char const *driver_id)
{
   void *ptr, *buf;
   int mem_fd;
   size_t alloc_size, offset;

   *fd = -1;

   /*
    * Calculate
    *
    *   alloc_size = size + alignment + sizeof(struct memory_header) + sizeof(size_t)
    *
    * while checking for overflow.
    */
   const size_t header_size = sizeof(struct memory_header) + sizeof(size_t);
   if (add_overflow_size_t(size, alignment, &alloc_size) ||
       add_overflow_size_t(alloc_size, header_size, &alloc_size))
      return NULL;

   mem_fd = os_create_anonymous_file(alloc_size, fd_name);

   if(mem_fd < 0)
      return NULL;

#if defined(HAVE_MEMFD_CREATE) || defined(ANDROID)
   // Seal fd, so no one can grow or shrink the memory.
   if (fcntl(mem_fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_SEAL) != 0)
      goto fail;
#endif

   ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
   if (ptr == MAP_FAILED)
      goto fail;

   // Save the size and offset at the start, so we have all we need to unmap the memory
   // and we are able to find the start of the actual data-section. Also save the
   // offset directly before the data-section, so we can find the start of the mapped memory.
   // |   size   |  offset  | ... padding ... |  offset  | ... data ... |
   // ^                                                  ^              ^
   // 0                                               offset          size
   buf = (char *)(((uintptr_t)ptr + header_size + alignment - 1) & ~((uintptr_t)(alignment - 1)));
   offset = (size_t)((uintptr_t)buf - (uintptr_t)ptr);
   struct memory_header* header = (struct memory_header*)ptr;
   header->size = alloc_size;
   header->offset = offset;
   ((size_t*)buf)[-1] = offset;

   // Add the hash of the driver_id as a uuid to the header in order to identify the memory
   // when importing.
   uint8_t sha1[SHA1_DIGEST_LENGTH];
   get_driver_id_sha1_hash(sha1, driver_id);

   assert(SHA1_DIGEST_LENGTH >= UUID_SIZE);
   memcpy(header->uuid, sha1, UUID_SIZE);

   *fd = mem_fd;
   return buf;

fail:
   close(mem_fd);
   return NULL;
}

/**
 * Free memory returned by os_malloc_aligned_fd().
 */
void
os_free_fd(void *ptr)
{
   if (ptr) {
      size_t offset = ((size_t*)ptr)[-1];
      struct memory_header* header = (struct memory_header*)((uintptr_t)ptr - offset);
      // check if the offset at the beginning of the memory
      // is the same as the one we saved directly before the data.
      assert(offset == header->offset);
      munmap(header, header->size);
   }
}

#endif
