/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#include "util/u_pointer.h"
#include "util/u_hash_table.h"

#if DETECT_OS_UNIX
#include <sys/stat.h>
#endif


static uint32_t
pointer_hash(const void *key)
{
   return _mesa_hash_pointer(key);
}


static bool
pointer_equal(const void *a, const void *b)
{
   return a == b;
}


struct hash_table *
util_hash_table_create_ptr_keys(void)
{
   return _mesa_hash_table_create(NULL, pointer_hash, pointer_equal);
}


static uint32_t hash_fd(const void *key)
{
#if DETECT_OS_UNIX
   int fd = pointer_to_intptr(key);
   struct stat stat;

   fstat(fd, &stat);

   return stat.st_dev ^ stat.st_ino ^ stat.st_rdev;
#else
   return 0;
#endif
}


static bool equal_fd(const void *key1, const void *key2)
{
#if DETECT_OS_UNIX
   int fd1 = pointer_to_intptr(key1);
   int fd2 = pointer_to_intptr(key2);
   struct stat stat1, stat2;

   fstat(fd1, &stat1);
   fstat(fd2, &stat2);

   return stat1.st_dev == stat2.st_dev &&
          stat1.st_ino == stat2.st_ino &&
          stat1.st_rdev == stat2.st_rdev;
#else
   return 0;
#endif
}


struct hash_table *
util_hash_table_create_fd_keys(void)
{
   return _mesa_hash_table_create(NULL, hash_fd, equal_fd);
}


void *
util_hash_table_get(struct hash_table *ht,
                    void *key)
{
   struct hash_entry *entry = _mesa_hash_table_search(ht, key);

   return entry ? entry->data : NULL;
}


int
util_hash_table_foreach(struct hash_table *ht,
                        int (*callback)
                        (void *key, void *value, void *data),
                        void *data)
{
   hash_table_foreach(ht, entry) {
      int error = callback((void*)entry->key, entry->data, data);
      if (error != 0)
         return error;
   }
   return 0;
}
