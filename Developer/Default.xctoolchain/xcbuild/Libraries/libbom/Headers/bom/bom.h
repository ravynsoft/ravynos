/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef _BOM_H
#define _BOM_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Memory Management */

struct bom_context_memory {
    void *data;
    size_t size;

    void (*resize)(struct bom_context_memory *memory, size_t size);
    void (*free)(struct bom_context_memory *memory);
    void *ctx;
};

struct bom_context_memory
bom_context_memory(void const *data, size_t size);

struct bom_context_memory
bom_context_memory_file(const char *fn, bool writeable, size_t minimum_size);


struct bom_context;

struct bom_context *
bom_alloc_empty(struct bom_context_memory memory);

struct bom_context *
bom_alloc_load(struct bom_context_memory memory);

struct bom_context_memory const *
bom_memory(struct bom_context const *context);

void
bom_free(struct bom_context *context);


/* Index */

typedef bool (*bom_index_iterator)(struct bom_context *context, uint32_t index, void *data, size_t data_len, void *ctx);

void
bom_index_iterate(struct bom_context *context, bom_index_iterator iterator, void *ctx);

void *
bom_index_get(struct bom_context *context, uint32_t index, size_t *data_len);

void
bom_index_reserve(struct bom_context *context, size_t count);

uint32_t
bom_index_add(struct bom_context *context, const void *data, size_t data_len);

uint32_t
bom_free_indices_add(struct bom_context *context, size_t count);

void
bom_index_append(struct bom_context *context, uint32_t index, size_t data_len);


/* Variable */

typedef bool (*bom_variable_iterator)(struct bom_context *context, const char *name, int data_index, void *ctx);

void
bom_variable_iterate(struct bom_context *context, bom_variable_iterator iterator, void *ctx);

int
bom_variable_get(struct bom_context *context, const char *name);

void
bom_variable_add(struct bom_context *context, const char *name, int data_index);


/* Tree */

struct bom_tree_context;

struct bom_tree_context *
bom_tree_alloc_empty(struct bom_context *context, const char *variable_name);

struct bom_tree_context *
bom_tree_alloc_load(struct bom_context *context, const char *variable_name);

void
bom_tree_free(struct bom_tree_context *tree);

struct bom_context *
bom_tree_context_get(struct bom_tree_context *tree);

const char *
bom_tree_variable_get(struct bom_tree_context *tree);

typedef void (*bom_tree_iterator)(struct bom_tree_context *tree, void *key, size_t key_len, void *value, size_t value_len, void *ctx);

void
bom_tree_iterate(struct bom_tree_context *tree, bom_tree_iterator iterator, void *ctx);

void
bom_tree_reserve(struct bom_tree_context *tree, size_t count);

void
bom_tree_add(struct bom_tree_context *tree, const void *key, size_t key_len, const void *value, size_t value_len);


#ifdef __cplusplus
}
#endif

#endif /* _BOM_H */
