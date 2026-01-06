/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <bom/bom.h>
#include <bom/bom_format.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#if _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

struct bom_tree_context {
    struct bom_context *context;
    char *variable_name;
    int tree_iterating;
};


static struct bom_tree_context *
_bom_tree_alloc(struct bom_context *context, const char *variable_name)
{
    assert(context != NULL);
    assert(variable_name != NULL);

    struct bom_tree_context *tree_context = malloc(sizeof(*tree_context));
    if (tree_context == NULL) {
        return NULL;
    }

    tree_context->context = context;
    tree_context->tree_iterating = 0;

    tree_context->variable_name = malloc(strlen(variable_name) + 1);
    if (tree_context->variable_name == NULL) {
        bom_tree_free(tree_context);
        return NULL;
    }
    strcpy(tree_context->variable_name, variable_name);

    return tree_context;
}

struct bom_tree_context *
bom_tree_alloc_empty(struct bom_context *context, const char *variable_name)
{
    struct bom_tree_context *tree_context = _bom_tree_alloc(context, variable_name);
    if (tree_context == NULL) {
        return NULL;
    }


    struct bom_tree_entry *entry = malloc(sizeof(*entry));
    if (entry == NULL) {
        bom_tree_free(tree_context);
        return NULL;
    }

    struct bom_tree *tree = malloc(sizeof(*tree));
    if (tree == NULL) {
        free(entry);
        bom_tree_free(tree_context);
        return NULL;
    }

    entry->is_leaf = htons(1); // todo
    entry->count = htons(0);
    entry->forward = htonl(0);
    entry->backward = htonl(0);
    uint32_t entry_index = bom_index_add(tree_context->context, entry, sizeof(*entry));
    free(entry);

    strncpy(tree->magic, "tree", 4);
    tree->version = htonl(1);
    tree->child = htonl(entry_index);
    tree->node_size = htonl(4096); // todo
    tree->path_count = htonl(0);
    tree->unknown3 = 0;
    uint32_t tree_index = bom_index_add(tree_context->context, tree, sizeof(*tree));
    free(tree);

    bom_variable_add(tree_context->context, tree_context->variable_name, tree_index);

    return tree_context;
}

struct bom_tree_context *
bom_tree_alloc_load(struct bom_context *context, const char *variable_name)
{
    struct bom_tree_context *tree_context = _bom_tree_alloc(context, variable_name);
    if (tree_context == NULL) {
        return NULL;
    }

    uint32_t tree_index = bom_variable_get(tree_context->context, tree_context->variable_name);
    if (tree_index == -1) {
        bom_tree_free(tree_context);
        return NULL;
    }

    size_t tree_len;
    struct bom_tree *tree = (struct bom_tree *)bom_index_get(tree_context->context, tree_index, &tree_len);
    if (tree == NULL || tree_len < sizeof(struct bom_tree) || strncmp(tree->magic, "tree", 4) || ntohl(tree->version) != 1) {
        bom_tree_free(tree_context);
        return NULL;
    }

    return tree_context;
}

void
bom_tree_free(struct bom_tree_context *tree_context)
{
    if (tree_context == NULL) {
        return;
    }

    free(tree_context->variable_name);
    free(tree_context);
}

struct bom_context *
bom_tree_context_get(struct bom_tree_context *tree_context)
{
    assert(tree_context != NULL);
    return tree_context->context;
}

const char *
bom_tree_variable_get(struct bom_tree_context *tree_context)
{
    assert(tree_context != NULL);
    return tree_context->variable_name;
}

void
bom_tree_iterate(struct bom_tree_context *tree_context, bom_tree_iterator iterator, void *ctx)
{
    assert(tree_context != NULL);

    tree_context->tree_iterating++;

    uint32_t tree_index = bom_variable_get(tree_context->context, tree_context->variable_name);
    struct bom_tree *tree = (struct bom_tree *)bom_index_get(tree_context->context, tree_index, NULL);

    struct bom_tree_entry *paths = (struct bom_tree_entry *)bom_index_get(tree_context->context, ntohl(tree->child), NULL);
    if (paths != NULL) {
        if (!paths->is_leaf) {
            struct bom_tree_entry_indexes *indexes = &paths->indexes[0];
            paths = (struct bom_tree_entry *)bom_index_get(tree_context->context, ntohl(indexes->value_index), NULL);
        }

        while (paths != NULL) {
            for (size_t i = 0; i < ntohs(paths->count); i++) {
                struct bom_tree_entry_indexes *indexes = &paths->indexes[i];

                size_t key_len;
                void *key = bom_index_get(tree_context->context, ntohl(indexes->key_index), &key_len);

                size_t value_len;
                void *value = bom_index_get(tree_context->context, ntohl(indexes->value_index), &value_len);

                iterator(tree_context, key, key_len, value, value_len, ctx);
            }

            if (paths->forward != htonl(0)) {
                paths = (struct bom_tree_entry *)bom_index_get(tree_context->context, ntohl(paths->forward), NULL);
            } else {
                paths = NULL;
            }
        }
    }

    tree_context->tree_iterating--;
}

void
bom_tree_reserve(struct bom_tree_context *tree_context, size_t count)
{
    assert(tree_context != NULL);
    assert(tree_context->tree_iterating == 0);

    uint32_t tree_index = bom_variable_get(tree_context->context, tree_context->variable_name);
    struct bom_tree *tree = (struct bom_tree *)bom_index_get(tree_context->context, tree_index, NULL);

    uint32_t paths_index = ntohl(tree->child);
    bom_index_append(tree_context->context, paths_index, sizeof(struct bom_tree_entry_indexes) * count);
}

void
bom_tree_add(struct bom_tree_context *tree_context, const void *key, size_t key_len, const void *value, size_t value_len)
{
    assert(tree_context != NULL);
    assert(key != NULL);
    assert(value != NULL);
    assert(tree_context->tree_iterating == 0);

    uint32_t key_index = bom_index_add(tree_context->context, key, key_len);
    uint32_t value_index = bom_index_add(tree_context->context, value, value_len);

    uint32_t tree_index = bom_variable_get(tree_context->context, tree_context->variable_name);
    struct bom_tree *tree = (struct bom_tree *)bom_index_get(tree_context->context, tree_index, NULL);

    uint32_t paths_index = ntohl(tree->child);

    size_t paths_length;
    struct bom_tree_entry *paths = (struct bom_tree_entry *)bom_index_get(tree_context->context, paths_index, &paths_length);

    if ((ntohs(paths->count) + 1) * sizeof(struct bom_tree_entry_indexes) > paths_length) {
        /* Make room for the new index, extending the size as necessary. */
        bom_index_append(tree_context->context, paths_index, sizeof(struct bom_tree_entry_indexes));

        /* Re-fetch after append invalidation. */
        tree = (struct bom_tree *)bom_index_get(tree_context->context, tree_index, NULL);
        paths = (struct bom_tree_entry *)bom_index_get(tree_context->context, paths_index, &paths_length);
    }

    size_t last_index = ntohs(paths->count);
    size_t entry_index = 0;
    size_t start_range = 0;
    size_t end_range = last_index;

    /* BOM trees store their keys sorted. Figure out the index for the new entry
       using binary search. start_range and end_range are inclusive possibilities. */
    while (start_range < end_range) {
        size_t delta = end_range - start_range;
        entry_index = delta / 2 + start_range;
        struct bom_tree_entry_indexes *other_index = &paths->indexes[entry_index];

        size_t other_len;
        void *other_key = bom_index_get(tree_context->context, ntohl(other_index->key_index), &other_len);

        /* Check the ordering for the candidate key and the existing key value. If the values are
           seemingly identical, order shorter keys first. */
        int result = other_key == NULL ? -1 : memcmp(key, other_key, other_len < key_len ? other_len : key_len);
        if (result == 0 && key_len != other_len) {
            result = key_len < other_len ? -1 : 1;
        }

        if (result < 0) {
            /* If comparing c in [a,b,c,d,e], then choose [a,b,c] as the
               potential part of the tree the new key may live in. The candidate index
               is always less than the end range candidate, so the range always shrinks. */
            end_range = entry_index;
        } else if (result > 0) {
            /* If comparing c in [a,b,c,d,e], then choose [d,e] as the
               potential part of the tree the new key may live in. The new key's index
               must be greater than c's index. */
            start_range = entry_index + 1;
        } else {
            break;
        }
    }

    /* Set the indexes for the inserted entry after shifting all other data */
    struct bom_tree_entry_indexes *indexes = &paths->indexes[entry_index];
    if (entry_index < last_index) {
        memmove(&paths->indexes[entry_index+1], indexes, (last_index - entry_index) * sizeof(struct bom_tree_entry_indexes));
    }
    indexes->key_index = htonl(key_index);
    indexes->value_index = htonl(value_index);

    /* Update counts for inserted entry. */
    tree->path_count = htonl(ntohl(tree->path_count) + 1);
    paths->count = htons(ntohs(paths->count) + 1);
}
