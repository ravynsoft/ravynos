/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <bom/bom.h>
#include <bom/bom_format.h>

#include <stdio.h>
#include <stdbool.h>

#if _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

static void
_bom_tree_dump(struct bom_tree_context *tree, void *key, size_t key_len, void *value, size_t value_len, void *ctx)
{
    printf("\t\tEntry with key of size %zu and value of size %zu\n", key_len, value_len);
}

static bool
_bom_variable_dump(struct bom_context *context, const char *name, int data_index, void *ctx)
{
    printf("\t%s: index %x\n", name, data_index);

    struct bom_tree_context *tree_context = bom_tree_alloc_load(context, name);
    if (tree_context != NULL) {
        printf("\tFound BOM Tree:\n");
        bom_tree_iterate(tree_context, _bom_tree_dump, NULL);
    }
    return true;
}

static bool
_bom_index_dump(struct bom_context *context, uint32_t index, void *data, size_t data_len, void *ctx)
{
    printf("\t%d: data at %p (%zx bytes)\n", index, data, data_len);
    return true;
}

int
main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "error: missing argument\n");
        return 1;
    }

    struct bom_context *context;
    context = bom_alloc_load(bom_context_memory_file(argv[1], true, 0));
    if (context == NULL) {
        fprintf(stderr, "error: failed to load BOM\n");
        return 1;
    }

    struct bom_header *header = (struct bom_header *)bom_memory(context)->data;
    struct bom_index_header *index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));
    struct bom_variables *vars = (struct bom_variables *)((uintptr_t)header + ntohl(header->variables_offset));

    printf("Start of memory: %p\n", header);
    printf("Start of index header: %p - %u bytes\n", index_header, (uint32_t)ntohl(header->index_length));
    printf("Start of variable header: %p - %u bytes\n", vars, (uint32_t)ntohl(header->trailer_len));
    printf("\n");

    printf("variables:\n");
    bom_variable_iterate(context, &_bom_variable_dump, NULL);
    printf("\n");

    printf("Number of useful index blocks: %u\n", (uint32_t)ntohl(header->block_count));
    printf("\n");

    printf("index:\n");
    bom_index_iterate(context, &_bom_index_dump, NULL);

    return 0;
}
