/*
 * Copyright © 2020 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _ISASPEC_H_
#define _ISASPEC_H_

#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>
#include "util/bitset.h"

#ifdef __cplusplus
extern "C" {
#endif

struct isa_decode_value {
	/** for {NAME} */
	const char *str;
	/** for all other fields */
	uint64_t num;
};

struct isa_decode_hook {
	const char *fieldname;
	void (*cb)(void *data, struct isa_decode_value *val);
};

struct isa_entrypoint {
	const char *name;
	uint32_t offset;
};

struct isa_print_state {
	FILE *out;

	/**
	 * Column number of current line
	 */
	unsigned line_column;
};

void isa_print(struct isa_print_state *state, const char *fmt, ...) PRINTFLIKE(2, 3);

struct isa_decode_options {
	uint32_t gpu_id;

	/** show errors detected in decoding, like unexpected dontcare bits */
	bool show_errors;

	/**
	 * If non-zero, maximum # of instructions that are unmatched before
	 * bailing, ie. to trigger stopping if we start trying to decode
	 * random garbage.
	 */
	unsigned max_errors;

	/** Generate branch target labels */
	bool branch_labels;

	/**
	 * Flag which can be set, for ex, but decode hook to trigger end of
	 * decoding
	 */
	bool stop;

	/**
	 * Data passed back to decode hooks
	 */
	void *cbdata;

	/**
	 * Callback for field decode
	 */
	void (*field_cb)(void *data, const char *field_name, struct isa_decode_value *val);

	/**
	 * Callback for fields that need custom code to print their value.
	 */
	void (*field_print_cb)(struct isa_print_state *print, const char *field_name, uint64_t val);

	/**
	 * Callback prior to instruction decode
	 */
	void (*pre_instr_cb)(void *data, unsigned n, void *instr);

	/**
	 * Callback after instruction decode
	 */
	void (*post_instr_cb)(void *data, unsigned n, void *instr);

	/**
	 * callback for undefined instructions
	 */
	void (*no_match_cb)(FILE *out, const BITSET_WORD *bitset, size_t size);

	/**
	 * List of known entrypoints to treat like call targets
	 */
	unsigned entrypoint_count;
	const struct isa_entrypoint *entrypoints;
};

void isa_disasm(void *bin, int sz, FILE *out, const struct isa_decode_options *options);

bool isa_decode(void *out, void *bin, const struct isa_decode_options *options);

#ifdef __cplusplus
}
#endif

#endif /* _ISASPEC_H_ */
