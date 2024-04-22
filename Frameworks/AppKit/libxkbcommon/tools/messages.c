/*
 * NOTE: This file has been generated automatically by “update-message-registry.py”.
 *       Do not edit manually!
 *
 */

/*
 * Copyright © 2023 Pierre Le Marre <dev@wismill.eu>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "messages-codes.h"
#include "messages.h"
#include "utils.h"

static const struct xkb_message_entry xkb_messages[] = {
    {XKB_ERROR_MALFORMED_NUMBER_LITERAL, "Malformed number literal"},
    {XKB_WARNING_CONFLICTING_KEY_TYPE_PRESERVE_ENTRIES, "Conflicting key type preserve entries"},
    {XKB_ERROR_UNSUPPORTED_MODIFIER_MASK, "Unsupported modifier mask"},
    {XKB_ERROR_EXPECTED_ARRAY_ENTRY, "Expected array entry"},
    {XKB_WARNING_ILLEGAL_KEYCODE_ALIAS, "Illegal keycode alias"},
    {XKB_WARNING_UNRECOGNIZED_KEYSYM, "Unrecognized keysym"},
    {XKB_ERROR_UNDECLARED_VIRTUAL_MODIFIER, "Undeclared virtual modifier"},
    {XKB_ERROR_WRONG_STATEMENT_TYPE, "Wrong statement type"},
    {XKB_WARNING_UNSUPPORTED_GEOMETRY_SECTION, "Unsupported geometry section"},
    {XKB_WARNING_CANNOT_INFER_KEY_TYPE, "Cannot infer key type"},
    {XKB_WARNING_INVALID_ESCAPE_SEQUENCE, "Invalid escape sequence"},
    {XKB_WARNING_ILLEGAL_KEY_TYPE_PRESERVE_RESULT, "Illegal key type preserve result"},
    {XKB_ERROR_INVALID_INCLUDE_STATEMENT, "Invalid include statement"},
    {XKB_ERROR_INVALID_MODMAP_ENTRY, "Invalid modmap entry"},
    {XKB_ERROR_UNSUPPORTED_GROUP_INDEX, "Unsupported group index"},
    {XKB_WARNING_CONFLICTING_KEY_TYPE_LEVEL_NAMES, "Conflicting key type level names"},
    {XKB_ERROR_INVALID_SET_DEFAULT_STATEMENT, "Invalid set default statement"},
    {XKB_WARNING_CONFLICTING_KEY_TYPE_MAP_ENTRY, "Conflicting key type map entry"},
    {XKB_WARNING_UNDEFINED_KEY_TYPE, "Undefined key type"},
    {XKB_WARNING_NON_BASE_GROUP_NAME, "Non base group name"},
    {XKB_ERROR_UNSUPPORTED_SHIFT_LEVEL, "Unsupported shift level"},
    {XKB_ERROR_INCLUDED_FILE_NOT_FOUND, "Included file not found"},
    {XKB_ERROR_UNKNOWN_OPERATOR, "Unknown operator"},
    {XKB_WARNING_DUPLICATE_ENTRY, "Duplicate entry"},
    {XKB_WARNING_CONFLICTING_KEY_TYPE_DEFINITIONS, "Conflicting key type definitions"},
    {XKB_ERROR_WRONG_SCOPE, "Wrong scope"},
    {XKB_WARNING_MISSING_DEFAULT_SECTION, "Missing default section"},
    {XKB_WARNING_CONFLICTING_KEY_SYMBOL, "Conflicting key symbol"},
    {XKB_ERROR_INVALID_OPERATION, "Invalid operation"},
    {XKB_WARNING_NUMERIC_KEYSYM, "Numeric keysym"},
    {XKB_WARNING_EXTRA_SYMBOLS_IGNORED, "Extra symbols ignored"},
    {XKB_WARNING_CONFLICTING_KEY_NAME, "Conflicting key name"},
    {XKB_ERROR_ALLOCATION_ERROR, "Allocation error"},
    {XKB_ERROR_WRONG_FIELD_TYPE, "Wrong field type"},
    {XKB_ERROR_INVALID_REAL_MODIFIER, "Invalid real modifier"},
    {XKB_WARNING_UNKNOWN_CHAR_ESCAPE_SEQUENCE, "Unknown char escape sequence"},
    {XKB_ERROR_INVALID_INCLUDED_FILE, "Invalid included file"},
    {XKB_WARNING_MULTIPLE_GROUPS_AT_ONCE, "Multiple groups at once"},
    {XKB_WARNING_UNSUPPORTED_SYMBOLS_FIELD, "Unsupported symbols field"},
    {XKB_ERROR_INVALID_SYNTAX, "Invalid syntax"},
    {XKB_WARNING_UNDEFINED_KEYCODE, "Undefined keycode"},
    {XKB_ERROR_INVALID_EXPRESSION_TYPE, "Invalid expression type"},
    {XKB_ERROR_INVALID_VALUE, "Invalid value"},
    {XKB_WARNING_CONFLICTING_MODMAP, "Conflicting modmap"},
    {XKB_ERROR_UNKNOWN_FIELD, "Unknown field"},
    {XKB_WARNING_CONFLICTING_KEY_ACTION, "Conflicting key action"},
    {XKB_WARNING_CONFLICTING_KEY_TYPE_MERGING_GROUPS, "Conflicting key type merging groups"},
    {XKB_ERROR_CONFLICTING_KEY_SYMBOLS_ENTRY, "Conflicting key symbols entry"},
    {XKB_WARNING_MISSING_SYMBOLS_GROUP_NAME_INDEX, "Missing symbols group name index"},
    {XKB_WARNING_CONFLICTING_KEY_FIELDS, "Conflicting key fields"},
    {XKB_ERROR_INVALID_IDENTIFIER, "Invalid identifier"},
    {XKB_WARNING_UNRESOLVED_KEYMAP_SYMBOL, "Unresolved keymap symbol"},
    {XKB_WARNING_UNDECLARED_MODIFIERS_IN_KEY_TYPE, "Undeclared modifiers in key type"}
};

int
xkb_message_get_all(const struct xkb_message_entry **messages)
{
    *messages = xkb_messages;
    return ARRAY_SIZE(xkb_messages);
}

const struct xkb_message_entry*
xkb_message_get(xkb_message_code_t code)
{
    if (code < _XKB_LOG_MESSAGE_MIN_CODE || code > _XKB_LOG_MESSAGE_MAX_CODE)
        return NULL;

    for (size_t idx = 0; idx < ARRAY_SIZE(xkb_messages); idx++) {
        if (xkb_messages[idx].code == code)
            return &xkb_messages[idx];
    }

    /* no matching message code found */
    return NULL;
}
