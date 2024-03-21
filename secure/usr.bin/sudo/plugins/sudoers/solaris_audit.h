/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2014, Oracle and/or its affiliates.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef SUDOERS_SOLARIS_AUDIT_H
#define	SUDOERS_SOLARIS_AUDIT_H

struct sudoers_context;
int	solaris_audit_success(const struct sudoers_context *ctx, char *const argv[]);
int	solaris_audit_failure(const struct sudoers_context *ctx, char *const argv[], const char *errmsg);

#endif /* SUDOERS_SOLARIS_AUDIT_H */
