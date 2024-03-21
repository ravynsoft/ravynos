/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <avahi-common/gccmacro.h>

#include "prioq.h"

#define POINTER_TO_INT(p) ((int) (long) (p))
#define INT_TO_POINTER(i) ((void*) (long) (i))

static int compare_int(const void* a, const void* b) {
    int i = POINTER_TO_INT(a), j = POINTER_TO_INT(b);

    return i < j ? -1 : (i > j ? 1 : 0);
}

static int compare_ptr(const void* a, const void* b) {
    return a < b ? -1 : (a > b ? 1 : 0);
}

static void rec(AvahiPrioQueueNode *n) {
    if (!n)
        return;

    if (n->left)
        assert(n->left->parent == n);

    if (n->right)
        assert(n->right->parent == n);

    if (n->parent) {
        assert(n->parent->left == n || n->parent->right == n);

        if (n->parent->left == n)
            assert(n->next == n->parent->right);
    }

    if (!n->next) {
        assert(n->queue->last == n);

        if (n->parent && n->parent->left == n)
            assert(n->parent->right == NULL);
    }


    if (n->parent) {
        int a = POINTER_TO_INT(n->parent->data), b = POINTER_TO_INT(n->data);
        if (a > b) {
            printf("%i <= %i: NO\n", a, b);
            abort();
        }
    }

    rec(n->left);
    rec(n->right);
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    AvahiPrioQueue *q, *q2;
    int i;

    q = avahi_prio_queue_new(compare_int);
    q2 = avahi_prio_queue_new(compare_ptr);

    srand(time(NULL));

    for (i = 0; i < 10000; i++)
        avahi_prio_queue_put(q2, avahi_prio_queue_put(q, INT_TO_POINTER(random() & 0xFFFF)));

    while (q2->root) {
        rec(q->root);
        rec(q2->root);

        assert(q->n_nodes == q2->n_nodes);

        printf("%i\n", POINTER_TO_INT(((AvahiPrioQueueNode*)q2->root->data)->data));

        avahi_prio_queue_remove(q, q2->root->data);
        avahi_prio_queue_remove(q2, q2->root);
    }


/*     prev = 0; */
/*     while (q->root) { */
/*         int v = GPOINTER_TO_INT(q->root->data); */
/*         rec(q->root); */
/*         printf("%i\n", v); */
/*         avahi_prio_queue_remove(q, q->root); */
/*         assert(v >= prev); */
/*         prev = v; */
/*     } */

    avahi_prio_queue_free(q);
    return 0;
}
