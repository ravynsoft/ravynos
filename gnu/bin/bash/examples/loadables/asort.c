/*
   Copyright (C) 2020 Free Software Foundation, Inc.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "bashtypes.h"
#include "shell.h"
#include "builtins.h"
#include "common.h"
#include "xmalloc.h"
#include "bashgetopt.h"

typedef struct sort_element {
    ARRAY_ELEMENT *v;   // used when sorting array in-place
    char *key;          // used when sorting assoc array
    char *value;        // points to value of array element or assoc entry
    double num;         // used for numeric sort
} sort_element;

static int reverse_flag;
static int numeric_flag;

static int
compare(const void *p1, const void *p2) {
    const sort_element e1 = *(sort_element *) p1;
    const sort_element e2 = *(sort_element *) p2;

    if (numeric_flag) {
        if (reverse_flag)
            return (e2.num > e1.num) ? 1 : (e2.num < e1.num) ? -1 : 0;
        else
            return (e1.num > e2.num) ? 1 : (e1.num < e2.num) ? -1 : 0;
    }
    else {
        if (reverse_flag)
            return strcoll(e2.value, e1.value);
        else
            return strcoll(e1.value, e2.value);
    }
}

static int
sort_index(SHELL_VAR *dest, SHELL_VAR *source) {
    HASH_TABLE *hash;
    BUCKET_CONTENTS *bucket;
    sort_element *sa;
    ARRAY *array, *dest_array;
    ARRAY_ELEMENT *ae;
    size_t i, j, n;
    char ibuf[INT_STRLEN_BOUND (intmax_t) + 1]; // used by fmtulong
    char *key;

    dest_array = array_cell(dest);

    if (assoc_p(source)) {
        hash = assoc_cell(source);
        n = hash->nentries;
        sa = xmalloc(n * sizeof(sort_element));
        i = 0;
        for ( j = 0; j < hash->nbuckets; ++j ) {
            bucket = hash->bucket_array[j];
            while ( bucket ) {
                sa[i].v = NULL;
                sa[i].key = bucket->key;
                if ( numeric_flag )
                    sa[i].num = strtod(bucket->data, NULL);
                else
                    sa[i].value = bucket->data;
                i++;
                bucket = bucket->next;
            }
        }
    }
    else {
        array = array_cell(source);
        n = array_num_elements(array);
        sa = xmalloc(n * sizeof(sort_element));
        i = 0;

        for (ae = element_forw(array->head); ae != array->head; ae = element_forw(ae)) {
            sa[i].v = ae;
            if (numeric_flag)
                sa[i].num = strtod(element_value(ae), NULL);
            else
                sa[i].value = element_value(ae);
            i++;
        }
    }

    // sanity check
    if ( i != n ) {
        builtin_error("%s: corrupt array", source->name);
        return EXECUTION_FAILURE;
    }

    qsort(sa, n, sizeof(sort_element), compare);

    array_flush(dest_array);

    for ( i = 0; i < n; ++i ) {
        if ( assoc_p(source) )
            key = sa[i].key;
        else
            key = fmtulong((long unsigned)sa[i].v->ind, 10, ibuf, sizeof(ibuf), 0);

        array_insert(dest_array, i, key);
    }

    return EXECUTION_SUCCESS;
}

static int
sort_inplace(SHELL_VAR *var) {
    size_t i, n;
    ARRAY *a;
    ARRAY_ELEMENT *ae;
    sort_element *sa = 0;

    a = array_cell(var);
    n = array_num_elements(a);

    if ( n == 0 )
        return EXECUTION_SUCCESS;

    sa = xmalloc(n * sizeof(sort_element));

    i = 0;
    for (ae = element_forw(a->head); ae != a->head; ae = element_forw(ae)) {
        sa[i].v = ae;
        if (numeric_flag)
            sa[i].num = strtod(element_value(ae), NULL);
        else
            sa[i].value = element_value(ae);
        i++;
    }

    // sanity check
    if ( i != n ) {
        builtin_error("%s: corrupt array", var->name);
        return EXECUTION_FAILURE;
    }

    qsort(sa, n, sizeof(sort_element), compare);

    // for in-place sort, simply "rewire" the array elements
    sa[0].v->prev = sa[n-1].v->next = a->head;
    a->head->next = sa[0].v;
    a->head->prev = sa[n-1].v;
    a->max_index = n - 1;
    for (i = 0; i < n; i++) {
        sa[i].v->ind = i;
        if (i > 0)
            sa[i].v->prev = sa[i-1].v;
        if (i < n - 1)
            sa[i].v->next = sa[i+1].v;
    }
    xfree(sa);
    return EXECUTION_SUCCESS;
}

int
asort_builtin(WORD_LIST *list) {
    SHELL_VAR *var, *var2;
    char *word;
    int opt, ret;
    int index_flag = 0;

    numeric_flag = 0;
    reverse_flag = 0;

    reset_internal_getopt();
    while ((opt = internal_getopt(list, "inr")) != -1) {
        switch (opt) {
            case 'i': index_flag = 1; break;
            case 'n': numeric_flag = 1; break;
            case 'r': reverse_flag = 1; break;
            CASE_HELPOPT;
            default:
                builtin_usage();
                return (EX_USAGE);
        }
    }
    list = loptend;

    if (list == 0) {
        builtin_usage();
        return EX_USAGE;
    }

    if (legal_identifier (list->word->word) == 0) {
        sh_invalidid (list->word->word);
        return EXECUTION_FAILURE;
    }

    if ( index_flag ) {
        if ( list->next == 0 || list->next->next ) {
            builtin_usage();
            return EX_USAGE;
        }
        if (legal_identifier (list->next->word->word) == 0) {
            sh_invalidid (list->next->word->word);
            return EXECUTION_FAILURE;
        }
        var = find_or_make_array_variable(list->word->word, 1);
        if (var == 0)
            return EXECUTION_FAILURE;
        var2 = find_variable(list->next->word->word);
        if ( !var2 || ( !array_p(var2) && !assoc_p(var2) ) ) {
            builtin_error("%s: Not an array", list->next->word->word);
            return EXECUTION_FAILURE;
        }
        return sort_index(var, var2);
    }

    while (list) {
        word = list->word->word;
        var = find_variable(word);
        list = list->next;

        if (var == 0 || array_p(var) == 0) {
            builtin_error("%s: Not an array", word);
            continue;
        }
        if (readonly_p(var) || noassign_p(var)) {
            if (readonly_p(var))
                err_readonly(word);
            continue;
        }

        if ( (ret = sort_inplace(var)) != EXECUTION_SUCCESS )
            return ret;
    }
    return EXECUTION_SUCCESS;

}

char *asort_doc[] = {
    "Sort arrays in-place.",
    "",
    "Options:",
    "  -n  compare according to string numerical value",
    "  -r  reverse the result of comparisons",
    "  -i  sort using indices/keys",
    "",
    "If -i is supplied, SOURCE is not sorted in-place, but the indices (or keys",
    "if associative) of SOURCE, after sorting it by its values, are placed as",
    "values in the indexed array DEST",
    "",
    "Associative arrays may not be sorted in-place.",
    "",
    "Exit status:",
    "Return value is zero unless an error happened (like invalid variable name",
    "or readonly array).",
    (char *)NULL
};

struct builtin asort_struct = {
    "asort",
    asort_builtin,
    BUILTIN_ENABLED,
    asort_doc,
    "asort [-nr] array ...  or  asort [-nr] -i dest source",
    0
};
