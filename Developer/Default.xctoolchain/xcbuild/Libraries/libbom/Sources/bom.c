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

struct bom_context {
    struct bom_context_memory memory;
    unsigned int iteration_count;
};

struct bom_context *
_bom_alloc(struct bom_context_memory memory)
{
    struct bom_context *context = malloc(sizeof(*context));
    if (context == NULL) {
        return NULL;
    }

    context->memory = memory;
    context->iteration_count = 0;

    return context;
}

struct bom_context *
bom_alloc_empty(struct bom_context_memory memory)
{
    struct bom_context *context = _bom_alloc(memory);
    if (context == NULL) {
        return NULL;
    }


    size_t header_size = sizeof(struct bom_header);
    size_t index_size = sizeof(struct bom_index_header);
    size_t freelist_size = sizeof(struct bom_index_header) + sizeof(struct bom_index) * 2;
    size_t variables_size = sizeof(struct bom_variables);
    context->memory.resize(&context->memory, header_size + index_size + freelist_size + variables_size);

    struct bom_header *header = (struct bom_header *)context->memory.data;
    strncpy(header->magic, "BOMStore", 8);
    header->version = htonl(1);
    header->block_count = htonl(0);
    header->index_offset = htonl(header_size);
    header->index_length = htonl(index_size + freelist_size);
    header->variables_offset = htonl(ntohl(header->index_offset) + ntohl(header->index_length));
    header->trailer_len = htonl(variables_size);

    struct bom_index_header *index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));
    index_header->count = 0;

    struct bom_index_header *freelist_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset) + index_size);
    freelist_header->count = 0;

    /* BOM files always have two empty free list entries. */
    for (int i = 0; i < 2; i++) {
        struct bom_index *index = &freelist_header->index[i];
        index->address = 0;
        index->length = 0;
    }

    struct bom_variables *vars = (struct bom_variables *)((uintptr_t)header + ntohl(header->variables_offset));
    vars->count = 0;

    return context;
}

struct bom_context *
bom_alloc_load(struct bom_context_memory memory)
{
    struct bom_context *context = _bom_alloc(memory);
    if (context == NULL) {
        memory.free(&memory);
        return NULL;
    }

    if (context->memory.size < sizeof(struct bom_header)) {
        bom_free(context);
        return NULL;
    }

    struct bom_header *header = (struct bom_header *)context->memory.data;
    if (strncmp(header->magic, "BOMStore", 8) || ntohl(header->version) != 1) {
        bom_free(context);
        return NULL;
    }

    size_t index_offset = ntohl(header->index_offset);
    if (index_offset + sizeof(struct bom_index_header) > context->memory.size) {
        bom_free(context);
        return NULL;
    }

    size_t index_length = ntohl(header->index_length);
    if (index_offset + index_length > context->memory.size) {
        bom_free(context);
        return NULL;
    }

    size_t vars_offset = ntohl(header->variables_offset);
    if (vars_offset + sizeof(struct bom_variables) > context->memory.size) {
        bom_free(context);
        return NULL;
    }

    struct bom_variables *vars = (struct bom_variables *)((uintptr_t)header + ntohl(header->variables_offset));
    size_t vars_count = ntohl(vars->count);
    if (vars_offset + sizeof(struct bom_variables) + vars_count * sizeof(struct bom_variable) > context->memory.size) {
        bom_free(context);
        return NULL;
    }

    return context;
}

struct bom_context_memory const *
bom_memory(struct bom_context const *context)
{
    return &context->memory;
}

void
bom_free(struct bom_context *context)
{
    if (context == NULL) {
        return;
    }

    context->memory.free(&context->memory);
    free(context);
}


static uint32_t
_bom_address_update(uint32_t address, uint32_t point, ptrdiff_t delta)
{
    if (address >= point) {
        address += delta;
    }
    return address;
}

static void
_bom_address_update_all(struct bom_context *context, uint32_t point, ptrdiff_t delta)
{
    struct bom_header *header = (struct bom_header *)context->memory.data;

    /* Update offsets for each index. */
    struct bom_index_header *index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));
    for (size_t i = 0; i < ntohl(index_header->count); i++) {
        struct bom_index *index = &index_header->index[i];
        index->address = htonl(_bom_address_update(ntohl(index->address), point, delta));
    }

    /* Must be after indexes are updated, so index_header is still correct. */
    header->index_offset = htonl(_bom_address_update(ntohl(header->index_offset), point, delta));
    header->variables_offset = htonl(_bom_address_update(ntohl(header->variables_offset), point, delta));
}

static void
_bom_address_resize(struct bom_context *context, uint32_t point, ptrdiff_t delta)
{
    _bom_address_update_all(context, point, delta);

    context->memory.resize(&context->memory, context->memory.size + delta);
    memmove((void *)((uintptr_t)context->memory.data + point + delta), (void *)((uintptr_t)context->memory.data + point), context->memory.size - point - delta);
}


void
bom_index_iterate(struct bom_context *context, bom_index_iterator iterator, void *ctx)
{
    assert(context != NULL);
    assert(iterator != NULL);

    context->iteration_count++;

    struct bom_header *header = (struct bom_header *)context->memory.data;
    struct bom_index_header *index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));

    for (size_t i = 0; i < ntohl(index_header->count); i++) {
        size_t data_len;
        void *data = bom_index_get(context, i, &data_len);
        if (data == NULL) {
            continue;
        }

        if (!iterator(context, i, data, data_len, ctx)) {
            break;
        }
    }

    context->iteration_count--;
}

void *
bom_index_get(struct bom_context *context, uint32_t index, size_t *data_len)
{
    assert(context != NULL);

    struct bom_header *header = (struct bom_header *)context->memory.data;
    struct bom_index_header *index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));

    if (index >= ntohl(index_header->count)) {
        return NULL;
    }

    struct bom_index *iindex = &index_header->index[index];
    size_t ioffset = ntohl(iindex->address);
    size_t ilength = ntohl(iindex->length);
    if (ioffset + ilength > context->memory.size) {
        printf("warning: %d index length (%zd, %zd) extends outside buffer (%zx).\n", index, ioffset, ilength, context->memory.size);
        return NULL;
    }

    if (data_len != NULL) {
        *data_len = ilength;
    }

    return (void *)((uintptr_t)header + ioffset);
}

void
bom_index_reserve(struct bom_context *context, size_t count)
{
    assert(context != NULL);
    assert(context->iteration_count == 0 && "cannot mutate while iterating");
    struct bom_header *header = (struct bom_header *)context->memory.data;

    /* Insert space for extra indexes at the end of the currently allocated space. */
    size_t old_index_length = ntohl(header->index_length);
    uint32_t index_point = ntohl(header->index_offset) + old_index_length;
    ptrdiff_t index_delta = sizeof(struct bom_index) * count;
    size_t new_index_length = old_index_length + index_delta;
    _bom_address_resize(context, index_point, index_delta);

    /* Re-fetch, invalidated by resize. */
    header = (struct bom_header *)context->memory.data;
    header->index_length = htonl(new_index_length);

    /* Reset the memory in the reallocated space */
    void *memory_to_reset = (void *)((uintptr_t)header + ntohl(header->index_offset) + old_index_length);
    memset(memory_to_reset, 0, index_delta);
}

uint32_t
bom_index_add(struct bom_context *context, const void *data, size_t data_len)
{
    assert(context != NULL);
    assert(data != NULL);
    assert(context->iteration_count == 0 && "cannot mutate while iterating");

    struct bom_header *header = (struct bom_header *)context->memory.data;
    struct bom_index_header *index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));

    /* Insert index at the end of the list. */
    size_t index_to_add = ntohl(index_header->count);
    uint32_t index_point = ntohl(header->index_offset) + sizeof(struct bom_index_header) + sizeof(struct bom_index) * index_to_add;
    size_t new_index_length = sizeof(struct bom_index_header) + sizeof(struct bom_index) * (index_to_add + 1);
    if (new_index_length > ntohl(header->index_length) - (sizeof(struct bom_index) * 2) ) {
        ptrdiff_t index_delta = sizeof(struct bom_index);
        _bom_address_resize(context, index_point, index_delta);

        /* Re-fetch, invalidated by resize. */
        header = (struct bom_header *)context->memory.data;
        header->index_length = htonl(ntohl(header->index_length) + sizeof(struct bom_index));

        /* Reset the memory in the last dummy index index */
        index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));
        memset((void *)&index_header->index[index_to_add+1], 0, index_delta);
    }

    /* Insert data at the very end. */
    uint32_t data_point = context->memory.size;
    _bom_address_resize(context, data_point, data_len);

    /* Re-fetch, invalidated by resize. */
    header = (struct bom_header *)context->memory.data;
    index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));

    /* Update values in newly inserted index. */
    struct bom_index *index = &index_header->index[index_to_add];
    index->address = htonl(data_point);
    index->length = htonl(data_len);

    /* Copy data into new data area. */
    memcpy((void *)((uintptr_t)header + data_point), data, data_len);

    /* Update length for newly added index. */
    index_header->count = htonl(ntohl(index_header->count) + 1);
    header->block_count = index_header->count;

    return index_to_add;
}

uint32_t
bom_free_indices_add(struct bom_context *context, size_t count)
{
    assert(count);
    assert(context->iteration_count == 0 && "cannot mutate while iterating");

    struct bom_header *header = (struct bom_header *)context->memory.data;
    struct bom_index_header *index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));

    /* Insert index at the end of the list. */
    uint32_t index_point = ntohl(header->index_offset) + sizeof(struct bom_index_header) + sizeof(struct bom_index) * ntohl(index_header->count);
    size_t new_index_length = sizeof(struct bom_index_header) + sizeof(struct bom_index) * (ntohl(index_header->count) + count);
    if (new_index_length > ntohl(header->index_length) - (sizeof(struct bom_index) * 2) ) {
        ptrdiff_t index_delta = sizeof(struct bom_index);
        _bom_address_resize(context, index_point, index_delta);

        /* Re-fetch, invalidated by resize. */
        header = (struct bom_header *)context->memory.data;
        header->index_length = htonl(ntohl(header->index_length) + (count * sizeof(struct bom_index)));
        index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));
    }

    /* Update values in newly inserted index. */
    size_t first_new_index = ntohl(index_header->count);
    for (size_t i = 0; i < count; ++i) {
        struct bom_index *index = &index_header->index[first_new_index + i];
        index->address = 0;
        index->length = 0;
    }

    /* Update length for newly added index. Only update the header count, as
       block_count tracks the number of useful indices. */
    index_header->count = htonl(ntohl(index_header->count) + count);

    return first_new_index + (count-1);
}

void
bom_index_append(struct bom_context *context, uint32_t idx, size_t data_len)
{
    assert(context != NULL);
    assert(context->iteration_count == 0 && "cannot mutate while iterating");

    struct bom_header *header = (struct bom_header *)context->memory.data;
    struct bom_index_header *index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));
    assert(idx < index_header->count);
    struct bom_index *index = &index_header->index[idx];

    /* Make room for the data at the end of the exisiting data. */
    uint32_t data_point = ntohl(index->address) + ntohl(index->length);
    _bom_address_resize(context, data_point, data_len);

    /* Re-fetch, invalidated by resize. */
    header = (struct bom_header *)context->memory.data;
    index_header = (struct bom_index_header *)((uintptr_t)header + ntohl(header->index_offset));
    index = &index_header->index[idx];

    /* Update length for newly added data. */
    index->length = htonl(ntohl(index->length) + data_len);
}


void
bom_variable_iterate(struct bom_context *context, bom_variable_iterator iterator, void *ctx)
{
    assert(context != NULL);
    assert(iterator != NULL);

    context->iteration_count++;

    struct bom_header *header = (struct bom_header *)context->memory.data;
    struct bom_variables *vars = (struct bom_variables *)((uintptr_t)header + ntohl(header->variables_offset));

    ptrdiff_t var_offset = 0;
    for (size_t i = 0; i < ntohl(vars->count); i++) {
        struct bom_variable *var = (struct bom_variable *)((uintptr_t)vars + sizeof(struct bom_variables) + var_offset);
        var_offset += (sizeof(struct bom_variable) + var->length);

        char *var_name = malloc(var->length + 1);
        if (var_name == NULL) {
            printf("Warning: could not allocate variable name\n");
            continue;
        }

        strncpy(var_name, var->name, var->length);
        var_name[var->length] = 0;
        if (!iterator(context, var_name, ntohl(var->index), ctx)) {
            free(var_name);
            break;
        }
        free(var_name);
    }

    context->iteration_count--;
}

struct _bom_variable_get_context {
    const char *name;
    int data_index;
};

static bool
_bom_variable_get_iterator(struct bom_context *context, const char *name, int data_index, void *ctx)
{
    struct _bom_variable_get_context *get_context = ctx;
    if (!strcmp(name, get_context->name)) {
        get_context->data_index = data_index;
        return false;
    }

    return true;
}

int
bom_variable_get(struct bom_context *context, const char *name)
{
    assert(context != NULL);
    assert(name != NULL);

    struct _bom_variable_get_context get_context = { name, -1 };
    bom_variable_iterate(context, _bom_variable_get_iterator, &get_context);
    return get_context.data_index;
}

void
bom_variable_add(struct bom_context *context, const char *name, int data_index)
{
    assert(context != NULL);
    assert(name != NULL);
    assert(context->iteration_count == 0 && "cannot mutate while iterating");

    struct bom_header *header = (struct bom_header *)context->memory.data;
    struct bom_variables *vars = (struct bom_variables *)((uintptr_t)header + ntohl(header->variables_offset));

    /* Find the end of the variables section. */
    ptrdiff_t var_offset = 0;
    for (size_t i = 0; i < ntohl(vars->count); i++) {
        struct bom_variable *var = (struct bom_variable *)((uintptr_t)vars + sizeof(struct bom_variables) + var_offset);
        var_offset += (sizeof(struct bom_variable) + var->length);
    }

    /* Insert variable at the end of the variables section. */
    uint32_t variable_point = ntohl(header->variables_offset) + sizeof(struct bom_variables) + var_offset;
    ptrdiff_t variable_delta = sizeof(struct bom_variable) + strlen(name);
    variable_delta += 4 - (variable_delta % 4);
    _bom_address_resize(context, variable_point, variable_delta);

    /* Re-fetch, invalidated by resize. */
    header = (struct bom_header *)context->memory.data;
    vars = (struct bom_variables *)((uintptr_t)header + ntohl(header->variables_offset));

    /* Update values in newly inserted variable. */
    struct bom_variable *var = (struct bom_variable *)((uintptr_t)vars + sizeof(struct bom_variables) + var_offset);
    var->index = htonl(data_index);
    var->length = (uint8_t)strlen(name);
    strncpy(var->name, name, var->length);

    /* Update length for newly added index. */
    vars->count = htonl(ntohl(vars->count) + 1);
    header->trailer_len = htonl(ntohl(header->trailer_len) + variable_delta);
}
