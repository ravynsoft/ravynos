/*
 * context.c - context save and restore
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 2015 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Peter Stephenson and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Peter Stephenson and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */
/*
 * This short file provides a home for the stack of saved contexts.
 * The actions for saving and restoring are encapsulated within
 * individual modules.
 */

#include "zsh.mdh"
#include "context.pro"

struct context_stack {
    struct context_stack *next;

    struct hist_stack hist_stack;
    struct lex_stack lex_stack;
    struct parse_stack parse_stack;
};

static struct context_stack *cstack;

/* save some or all of current context */

/**/
mod_export void
zcontext_save_partial(int parts)
{
    struct context_stack *cs;

    queue_signals();

    cs = (struct context_stack *)malloc(sizeof(struct context_stack));

    if (parts & ZCONTEXT_HIST) {
	hist_context_save(&cs->hist_stack, !cstack);
    }
    if (parts & ZCONTEXT_LEX) {
	lex_context_save(&cs->lex_stack, !cstack);
    }
    if (parts & ZCONTEXT_PARSE) {
	parse_context_save(&cs->parse_stack, !cstack);
    }

    cs->next = cstack;
    cstack = cs;

    unqueue_signals();
}

/* save context in full */

/**/
mod_export void
zcontext_save(void)
{
    zcontext_save_partial(ZCONTEXT_HIST|ZCONTEXT_LEX|ZCONTEXT_PARSE);
}

/* restore context or part thereof */

/**/
mod_export void
zcontext_restore_partial(int parts)
{
    struct context_stack *cs = cstack;

    DPUTS(!cstack, "BUG: zcontext_restore() without zcontext_save()");

    queue_signals();
    cstack = cstack->next;

    if (parts & ZCONTEXT_HIST) {
	hist_context_restore(&cs->hist_stack, !cstack);
    }
    if (parts & ZCONTEXT_LEX) {
	lex_context_restore(&cs->lex_stack, !cstack);
    }
    if (parts & ZCONTEXT_PARSE) {
	parse_context_restore(&cs->parse_stack, !cstack);
    }

    free(cs);

    unqueue_signals();
}

/* restore full context */

/**/
mod_export void
zcontext_restore(void)
{
    zcontext_restore_partial(ZCONTEXT_HIST|ZCONTEXT_LEX|ZCONTEXT_PARSE);
}
