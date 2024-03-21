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

#include <assert.h>
#include <stdlib.h>

#include <avahi-common/malloc.h>

#include "prioq.h"

AvahiPrioQueue* avahi_prio_queue_new(AvahiPQCompareFunc compare) {
    AvahiPrioQueue *q;
    assert(compare);

    if (!(q = avahi_new(AvahiPrioQueue, 1)))
        return NULL; /* OOM */

    q->root = q->last = NULL;
    q->n_nodes = 0;
    q->compare = compare;

    return q;
}

void avahi_prio_queue_free(AvahiPrioQueue *q) {
    assert(q);

    while (q->last)
        avahi_prio_queue_remove(q, q->last);

    assert(!q->n_nodes);
    avahi_free(q);
}

static AvahiPrioQueueNode* get_node_at_xy(AvahiPrioQueue *q, unsigned x, unsigned y) {
    unsigned r;
    AvahiPrioQueueNode *n;
    assert(q);

    n = q->root;
    assert(n);

    for (r = 0; r < y; r++) {
        assert(n);

        if ((x >> (y-r-1)) & 1)
            n = n->right;
        else
            n = n->left;
    }

    assert(n->x == x);
    assert(n->y == y);

    return n;
}

static void exchange_nodes(AvahiPrioQueue *q, AvahiPrioQueueNode *a, AvahiPrioQueueNode *b) {
    AvahiPrioQueueNode *l, *r, *p, *ap, *an, *bp, *bn;
    unsigned t;
    assert(q);
    assert(a);
    assert(b);
    assert(a != b);

    /* Swap positions */
    t = a->x; a->x = b->x; b->x = t;
    t = a->y; a->y = b->y; b->y = t;

    if (a->parent == b) {
        /* B is parent of A */

        p = b->parent;
        b->parent = a;

        if ((a->parent = p)) {
            if (a->parent->left == b)
                a->parent->left = a;
            else
                a->parent->right = a;
        } else
            q->root = a;

        if (b->left == a) {
            if ((b->left = a->left))
                b->left->parent = b;
            a->left = b;

            r = a->right;
            if ((a->right = b->right))
                a->right->parent = a;
            if ((b->right = r))
                b->right->parent = b;

        } else {
            if ((b->right = a->right))
                b->right->parent = b;
            a->right = b;

            l = a->left;
            if ((a->left = b->left))
                a->left->parent = a;
            if ((b->left = l))
                b->left->parent = b;
        }
    } else if (b->parent == a) {
        /* A ist parent of B */

        p = a->parent;
        a->parent = b;

        if ((b->parent = p)) {
            if (b->parent->left == a)
                b->parent->left = b;
            else
                b->parent->right = b;
        } else
            q->root = b;

        if (a->left == b) {
            if ((a->left = b->left))
                a->left->parent = a;
            b->left = a;

            r = a->right;
            if ((a->right = b->right))
                a->right->parent = a;
            if ((b->right = r))
                b->right->parent = b;
        } else {
            if ((a->right = b->right))
                a->right->parent = a;
            b->right = a;

            l = a->left;
            if ((a->left = b->left))
                a->left->parent = a;
            if ((b->left = l))
                b->left->parent = b;
        }
    } else {
        AvahiPrioQueueNode *apl = NULL, *bpl = NULL;

        /* Swap parents */
        ap = a->parent;
        bp = b->parent;

        if (ap)
            apl = ap->left;
        if (bp)
            bpl = bp->left;

        if ((a->parent = bp)) {
            if (bpl == b)
                bp->left = a;
            else
                bp->right = a;
        } else
            q->root = a;

        if ((b->parent = ap)) {
            if (apl == a)
                ap->left = b;
            else
                ap->right = b;
        } else
            q->root = b;

        /* Swap children */
        l = a->left;
        r = a->right;

        if ((a->left = b->left))
            a->left->parent = a;

        if ((b->left = l))
            b->left->parent = b;

        if ((a->right = b->right))
            a->right->parent = a;

        if ((b->right = r))
            b->right->parent = b;
    }

    /* Swap siblings */
    ap = a->prev; an = a->next;
    bp = b->prev; bn = b->next;

    if (a->next == b) {
        /* A is predecessor of B */
        a->prev = b;
        b->next = a;

        if ((a->next = bn))
            a->next->prev = a;
        else
            q->last = a;

        if ((b->prev = ap))
            b->prev->next = b;

    } else if (b->next == a) {
        /* B is predecessor of A */
        a->next = b;
        b->prev = a;

        if ((a->prev = bp))
            a->prev->next = a;

        if ((b->next = an))
            b->next->prev = b;
        else
            q->last = b;

    } else {
        /* A is no neighbour of B */

        if ((a->prev = bp))
            a->prev->next = a;

        if ((a->next = bn))
            a->next->prev = a;
        else
            q->last = a;

        if ((b->prev = ap))
            b->prev->next = b;

        if ((b->next = an))
            b->next->prev = b;
        else
            q->last = b;
    }
}

/* Move a node to the correct position */
void avahi_prio_queue_shuffle(AvahiPrioQueue *q, AvahiPrioQueueNode *n) {
    assert(q);
    assert(n);
    assert(n->queue == q);

    /* Move up until the position is OK */
    while (n->parent && q->compare(n->parent->data, n->data) > 0)
        exchange_nodes(q, n, n->parent);

    /* Move down until the position is OK */
    for (;;) {
        AvahiPrioQueueNode *min;

        if (!(min = n->left)) {
            /* No children */
            assert(!n->right);
            break;
        }

        if (n->right && q->compare(n->right->data, min->data) < 0)
            min = n->right;

        /* min now contains the smaller one of our two children */

        if (q->compare(n->data, min->data) <= 0)
            /* Order OK */
            break;

        exchange_nodes(q, n, min);
    }
}

AvahiPrioQueueNode* avahi_prio_queue_put(AvahiPrioQueue *q, void* data) {
    AvahiPrioQueueNode *n;
    assert(q);

    if (!(n = avahi_new(AvahiPrioQueueNode, 1)))
        return NULL; /* OOM */

    n->queue = q;
    n->data = data;

    if (q->last) {
        assert(q->root);
        assert(q->n_nodes);

        n->y = q->last->y;
        n->x = q->last->x+1;

        if (n->x >= ((unsigned) 1 << n->y)) {
            n->x = 0;
            n->y++;
        }

        q->last->next = n;
        n->prev = q->last;

        assert(n->y > 0);
        n->parent = get_node_at_xy(q, n->x/2, n->y-1);

        if (n->x & 1)
            n->parent->right = n;
        else
            n->parent->left = n;
    } else {
        assert(!q->root);
        assert(!q->n_nodes);

        n->y = n->x = 0;
        q->root = n;
        n->prev = n->parent = NULL;
    }

    n->next = n->left = n->right = NULL;
    q->last = n;
    q->n_nodes++;

    avahi_prio_queue_shuffle(q, n);

    return n;
}

void avahi_prio_queue_remove(AvahiPrioQueue *q, AvahiPrioQueueNode *n) {
    assert(q);
    assert(n);
    assert(q == n->queue);

    if (n != q->last) {
        AvahiPrioQueueNode *replacement = q->last;
        exchange_nodes(q, replacement, n);
        avahi_prio_queue_remove(q, n);
        avahi_prio_queue_shuffle(q, replacement);
        return;
    }

    assert(n == q->last);
    assert(!n->next);
    assert(!n->left);
    assert(!n->right);

    q->last = n->prev;

    if (n->prev) {
        n->prev->next = NULL;
        assert(n->parent);
    } else
        assert(!n->parent);

    if (n->parent) {
        assert(n->prev);
        if (n->parent->left == n) {
            assert(n->parent->right == NULL);
            n->parent->left = NULL;
        } else {
            assert(n->parent->right == n);
            assert(n->parent->left != NULL);
            n->parent->right = NULL;
        }
    } else {
        assert(q->root == n);
        assert(!n->prev);
        assert(q->n_nodes == 1);
        q->root = NULL;
    }

    avahi_free(n);

    assert(q->n_nodes > 0);
    q->n_nodes--;
}

