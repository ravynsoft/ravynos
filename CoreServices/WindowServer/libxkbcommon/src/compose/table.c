/*
 * Copyright © 2013,2021 Ran Benita <ran234@gmail.com>
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include "utils.h"
#include "table.h"
#include "parser.h"
#include "paths.h"
#include "xkbcommon/xkbcommon.h"

static struct xkb_compose_table *
xkb_compose_table_new(struct xkb_context *ctx,
                      const char *locale,
                      enum xkb_compose_format format,
                      enum xkb_compose_compile_flags flags)
{
    char *resolved_locale;
    struct xkb_compose_table *table;
    struct compose_node dummy;

    resolved_locale = resolve_locale(ctx, locale);
    if (!resolved_locale)
        return NULL;

    table = calloc(1, sizeof(*table));
    if (!table) {
        free(resolved_locale);
        return NULL;
    }

    table->refcnt = 1;
    table->ctx = xkb_context_ref(ctx);

    table->locale = resolved_locale;
    table->format = format;
    table->flags = flags;

    darray_init(table->nodes);
    darray_init(table->utf8);

    dummy.keysym = XKB_KEY_NoSymbol;
    dummy.leaf.is_leaf = true;
    dummy.leaf.utf8 = 0;
    dummy.leaf.keysym = XKB_KEY_NoSymbol;
    darray_append(table->nodes, dummy);

    darray_append(table->utf8, '\0');

    return table;
}

XKB_EXPORT struct xkb_compose_table *
xkb_compose_table_ref(struct xkb_compose_table *table)
{
    table->refcnt++;
    return table;
}

XKB_EXPORT void
xkb_compose_table_unref(struct xkb_compose_table *table)
{
    if (!table || --table->refcnt > 0)
        return;
    free(table->locale);
    darray_free(table->nodes);
    darray_free(table->utf8);
    xkb_context_unref(table->ctx);
    free(table);
}

XKB_EXPORT struct xkb_compose_table *
xkb_compose_table_new_from_file(struct xkb_context *ctx,
                                FILE *file,
                                const char *locale,
                                enum xkb_compose_format format,
                                enum xkb_compose_compile_flags flags)
{
    struct xkb_compose_table *table;
    bool ok;

    if (flags & ~(XKB_COMPOSE_COMPILE_NO_FLAGS)) {
        log_err_func(ctx, "unrecognized flags: %#x\n", flags);
        return NULL;
    }

    if (format != XKB_COMPOSE_FORMAT_TEXT_V1) {
        log_err_func(ctx, "unsupported compose format: %d\n", format);
        return NULL;
    }

    table = xkb_compose_table_new(ctx, locale, format, flags);
    if (!table)
        return NULL;

    ok = parse_file(table, file, "(unknown file)");
    if (!ok) {
        xkb_compose_table_unref(table);
        return NULL;
    }

    return table;
}

XKB_EXPORT struct xkb_compose_table *
xkb_compose_table_new_from_buffer(struct xkb_context *ctx,
                                  const char *buffer, size_t length,
                                  const char *locale,
                                  enum xkb_compose_format format,
                                  enum xkb_compose_compile_flags flags)
{
    struct xkb_compose_table *table;
    bool ok;

    if (flags & ~(XKB_COMPOSE_COMPILE_NO_FLAGS)) {
        log_err_func(ctx, "unrecognized flags: %#x\n", flags);
        return NULL;
    }

    if (format != XKB_COMPOSE_FORMAT_TEXT_V1) {
        log_err_func(ctx, "unsupported compose format: %d\n", format);
        return NULL;
    }

    table = xkb_compose_table_new(ctx, locale, format, flags);
    if (!table)
        return NULL;

    ok = parse_string(table, buffer, length, "(input string)");
    if (!ok) {
        xkb_compose_table_unref(table);
        return NULL;
    }

    return table;
}

XKB_EXPORT struct xkb_compose_table *
xkb_compose_table_new_from_locale(struct xkb_context *ctx,
                                  const char *locale,
                                  enum xkb_compose_compile_flags flags)
{
    struct xkb_compose_table *table;
    char *path;
    FILE *file;
    bool ok;

    if (flags & ~(XKB_COMPOSE_COMPILE_NO_FLAGS)) {
        log_err_func(ctx, "unrecognized flags: %#x\n", flags);
        return NULL;
    }

    table = xkb_compose_table_new(ctx, locale, XKB_COMPOSE_FORMAT_TEXT_V1,
                                  flags);
    if (!table)
        return NULL;

    path = get_xcomposefile_path(ctx);
    if (path) {
        file = fopen(path, "rb");
        if (file)
            goto found_path;
    }
    free(path);

    path = get_xdg_xcompose_file_path(ctx);
    if (path) {
        file = fopen(path, "rb");
        if (file)
            goto found_path;
    }
    free(path);

    path = get_home_xcompose_file_path(ctx);
    if (path) {
        file = fopen(path, "rb");
        if (file)
            goto found_path;
    }
    free(path);

    path = get_locale_compose_file_path(ctx, table->locale);
    if (path) {
        file = fopen(path, "rb");
        if (file)
            goto found_path;
    }
    free(path);

    log_err(ctx, XKB_LOG_MESSAGE_NO_ID,
            "couldn't find a Compose file for locale \"%s\" (mapped to \"%s\")\n",
            locale, table->locale);
    xkb_compose_table_unref(table);
    return NULL;

found_path:
    ok = parse_file(table, file, path);
    fclose(file);
    if (!ok) {
        free(path);
        xkb_compose_table_unref(table);
        return NULL;
    }

    log_dbg(ctx, XKB_LOG_MESSAGE_NO_ID,
            "created compose table from locale %s with path %s\n",
            table->locale, path);

    free(path);
    return table;
}

XKB_EXPORT const xkb_keysym_t *
xkb_compose_table_entry_sequence(struct xkb_compose_table_entry *entry,
                                 size_t *sequence_length)
{
    *sequence_length = entry->sequence_length;
    return entry->sequence;
}

XKB_EXPORT xkb_keysym_t
xkb_compose_table_entry_keysym(struct xkb_compose_table_entry *entry)
{
    return entry->keysym;
}

XKB_EXPORT const char *
xkb_compose_table_entry_utf8(struct xkb_compose_table_entry *entry)
{
    return entry->utf8;
}

enum node_direction {
    NODE_LEFT = 0,
    NODE_DOWN,
    NODE_RIGHT,
    NODE_UP
};

struct xkb_compose_table_iterator_cursor {
    uint32_t node_offset:30; /* WARNING: ensure it fits MAX_COMPOSE_NODES */
    uint8_t direction:2;     /* enum node_direction: current direction
                              * traversing the tree */
};

struct xkb_compose_table_iterator {
    struct xkb_compose_table *table;
    /* Current entry */
    struct xkb_compose_table_entry entry;
    /* Stack of pending nodes to process */
    darray(struct xkb_compose_table_iterator_cursor) cursors;
};

XKB_EXPORT struct xkb_compose_table_iterator *
xkb_compose_table_iterator_new(struct xkb_compose_table *table)
{
    struct xkb_compose_table_iterator *iter;
    struct xkb_compose_table_iterator_cursor cursor;
    xkb_keysym_t *sequence;

    iter = calloc(1, sizeof(*iter));
    if (!iter) {
        return NULL;
    }
    iter->table = xkb_compose_table_ref(table);
    sequence = calloc(MAX_LHS_LEN, sizeof(xkb_keysym_t));
    if (!sequence) {
        free(iter);
        return NULL;
    }
    iter->entry.sequence = sequence;
    iter->entry.sequence_length = 0;

    darray_init(iter->cursors);
    cursor.direction = NODE_LEFT;
    /* Offset 0 is a dummy null entry, skip it. */
    cursor.node_offset = 1;
    darray_append(iter->cursors, cursor);

    return iter;
}

XKB_EXPORT void
xkb_compose_table_iterator_free(struct xkb_compose_table_iterator *iter)
{
    xkb_compose_table_unref(iter->table);
    darray_free(iter->cursors);
    free(iter->entry.sequence);
    free(iter);
}

XKB_EXPORT struct xkb_compose_table_entry *
xkb_compose_table_iterator_next(struct xkb_compose_table_iterator *iter)
{
    /*
     * This function takes the following recursive traversal function,
     * and makes it non-recursive and resumable. The iter->cursors stack
     * is analogous to the call stack, and cursor->direction to the
     * instruction pointer of a stack frame.
     *
     *    traverse(xkb_keysym_t *sequence, size_t sequence_length, uint16_t p) {
     *        if (!p) return
     *        // cursor->direction == NODE_LEFT
     *        node = &darray_item(table->nodes, p)
     *        traverse(sequence, sequence_length, node->lokid)
     *        // cursor->direction == NODE_DOWN
     *        sequence[sequence_length++] = node->keysym
     *        if (node->is_leaf)
     *            emit(sequence, sequence_length, node->leaf.keysym, table->utf[node->leaf.utf8])
     *        else
     *            traverse(sequence, sequence_length, node->internal.eqkid)
     *        sequence_length--
     *        // cursor->direction == NODE_RIGHT
     *        traverse(sequence, sequence_length, node->hikid)
     *        // cursor->direction == NODE_UP
     *    }
     */

    struct xkb_compose_table_iterator_cursor *cursor;
    const struct compose_node *node;

    while (!darray_empty(iter->cursors)) {
        cursor = &darray_item(iter->cursors, darray_size(iter->cursors) - 1);
        node = &darray_item(iter->table->nodes, cursor->node_offset);

        switch (cursor->direction) {
        case NODE_LEFT:
            cursor->direction = NODE_DOWN;
            if (node->lokid) {
                struct xkb_compose_table_iterator_cursor new_cursor = {node->lokid, NODE_LEFT};
                darray_append(iter->cursors, new_cursor);
            }
            break;

        case NODE_DOWN:
            cursor->direction = NODE_RIGHT;
            assert (iter->entry.sequence_length <= MAX_LHS_LEN);
            iter->entry.sequence[iter->entry.sequence_length] = node->keysym;
            iter->entry.sequence_length++;
            if (node->is_leaf) {
                iter->entry.keysym = node->leaf.keysym;
                iter->entry.utf8 = &darray_item(iter->table->utf8, node->leaf.utf8);
                return &iter->entry;
            } else {
                struct xkb_compose_table_iterator_cursor new_cursor = {node->internal.eqkid, NODE_LEFT};
                darray_append(iter->cursors, new_cursor);
            }
            break;

        case NODE_RIGHT:
            cursor->direction = NODE_UP;
            iter->entry.sequence_length--;
            if (node->hikid) {
                struct xkb_compose_table_iterator_cursor new_cursor = {node->hikid, NODE_LEFT};
                darray_append(iter->cursors, new_cursor);
            }
            break;

        case NODE_UP:
            darray_remove_last(iter->cursors);
            break;
        }
    }

    return NULL;
}
