/*
 * Copyright © 2018 Intel Corporation
 * Copyright (c) 2021 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef H_LIMA_DISK_CACHE
#define H_LIMA_DISK_CACHE

struct disk_cache;
struct lima_screen;
struct lima_vs_key;
struct lima_fs_key;
struct lima_vs_compiled_shader;
struct lima_fs_compiled_shader;

void
lima_disk_cache_init(struct lima_screen *screen);

struct lima_vs_compiled_shader *
lima_vs_disk_cache_retrieve(struct disk_cache *cache,
                            struct lima_vs_key *key);

struct lima_fs_compiled_shader *
lima_fs_disk_cache_retrieve(struct disk_cache *cache,
                            struct lima_fs_key *key);

void
lima_vs_disk_cache_store(struct disk_cache *cache,
                         const struct lima_vs_key *key,
                         const struct lima_vs_compiled_shader *shader);

void
lima_fs_disk_cache_store(struct disk_cache *cache,
                         const struct lima_fs_key *key,
                         const struct lima_fs_compiled_shader *shader);

#endif
