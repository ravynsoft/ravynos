/*
 * Copyright 2011 Tom Stellard <tstellar@gmail.com>
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author: Tom Stellard <thomas.stellard@amd.com>
 */

#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "r500_fragprog.h"
#include "r300_fragprog_swizzle.h"
#include "radeon_compiler.h"
#include "radeon_compiler_util.h"
#include "radeon_opcodes.h"
#include "radeon_program.h"
#include "radeon_regalloc.h"
#include "radeon_swizzle.h"
#include "util/u_math.h"

#include "rc_test_helpers.h"

/* This file contains some helper functions for filling out the rc_instruction
 * data structures.  These functions take a string as input based on the format
 * output by rc_program_print().
 */

#define VERBOSE 0

#define DBG(...) do { if (VERBOSE) fprintf(stderr, __VA_ARGS__); } while(0)

#define REGEX_ERR_BUF_SIZE 50

struct match_info {
	const char * String;
	int Length;
};

static int is_whitespace(const char *str)
{
	regex_t regex;
	if (regcomp(&regex, "^[ \n]+$", REG_EXTENDED)) {
		fprintf(stderr, "Failed to compile whitespace regex\n");
		regfree(&regex);
		return 0;
	}
	bool ret = regexec(&regex, str, 0, NULL, 0) != REG_NOMATCH;
	regfree(&regex);
	return ret;
}

static int match_length(regmatch_t * matches, int index)
{
	return matches[index].rm_eo - matches[index].rm_so;
}

static int regex_helper(
	const char * regex_str,
	const char * search_str,
	regmatch_t * matches,
	int num_matches)
{
	char err_buf[REGEX_ERR_BUF_SIZE];
	regex_t regex;
	int err_code;
	unsigned int i;

	err_code = regcomp(&regex, regex_str, REG_EXTENDED);
	if (err_code) {
		regerror(err_code, &regex, err_buf, REGEX_ERR_BUF_SIZE);
		fprintf(stderr, "Failed to compile regex: %s\n", err_buf);
		regfree(&regex);
		return 0;
	}

	err_code = regexec(&regex, search_str, num_matches, matches, 0);
	DBG("Search string: '%s'\n", search_str);
	for (i = 0; i < num_matches; i++) {
		DBG("Match %u start = %zu end = %zu\n", i,
					(size_t)matches[i].rm_so,
					(size_t)matches[i].rm_eo);
	}
	if (err_code) {
		regerror(err_code, &regex, err_buf, REGEX_ERR_BUF_SIZE);
		fprintf(stderr, "Failed to match regex: %s\n", err_buf);
		regfree(&regex);
		return 0;
	}
	regfree(&regex);
	return 1;
}

#define REGEX_SRC_MATCHES 6

struct src_tokens {
	struct match_info Negate;
	struct match_info Abs;
	struct match_info File;
	struct match_info Index;
	struct match_info Swizzle;
};

/**
 * Initialize the source register at index src_index for the instruction based
 * on src_str.
 *
 * NOTE: Warning in init_rc_normal_instruction() applies to this function as
 * well.
 *
 * @param src_str A string that represents the source register.  The format for
 * this string is the same that is output by rc_program_print.
 * @return 1 On success, 0 on failure
 */
int init_rc_normal_src(
	struct rc_instruction * inst,
	unsigned int src_index,
	const char * src_str)
{
	const char * regex_str = "(-*)(\\|*)([[:lower:]]*)\\[*([[:digit:]]*)\\]*(\\.*[[:lower:]_]*)";
	regmatch_t matches[REGEX_SRC_MATCHES];
	struct src_tokens tokens;
	struct rc_src_register * src_reg = &inst->U.I.SrcReg[src_index];
	unsigned int i;

	/* Execute the regex */
	if (!regex_helper(regex_str, src_str, matches, REGEX_SRC_MATCHES)) {
		fprintf(stderr, "Failed to execute regex for src register.\n");
		return 0;
	}

	/* Create Tokens */
	tokens.Negate.String = src_str + matches[1].rm_so;
	tokens.Negate.Length = match_length(matches, 1);
	tokens.Abs.String = src_str + matches[2].rm_so;
	tokens.Abs.Length = match_length(matches, 2);
	tokens.File.String = src_str + matches[3].rm_so;
	tokens.File.Length = match_length(matches, 3);
	tokens.Index.String = src_str + matches[4].rm_so;
	tokens.Index.Length = match_length(matches, 4);
	tokens.Swizzle.String = src_str + matches[5].rm_so;
	tokens.Swizzle.Length = match_length(matches, 5);

	/* Negate */
	if (tokens.Negate.Length  > 0) {
		src_reg->Negate = RC_MASK_XYZW;
	}

	/* Abs */
	if (tokens.Abs.Length > 0) {
		src_reg->Abs = 1;
	}

	/* File */
	if (!strncmp(tokens.File.String, "temp", tokens.File.Length)) {
		src_reg->File = RC_FILE_TEMPORARY;
	} else if (!strncmp(tokens.File.String, "input", tokens.File.Length)) {
		src_reg->File = RC_FILE_INPUT;
	} else if (!strncmp(tokens.File.String, "const", tokens.File.Length)) {
		src_reg->File = RC_FILE_CONSTANT;
	} else if (!strncmp(tokens.File.String, "none", tokens.File.Length)) {
		src_reg->File = RC_FILE_NONE;
	}

	/* Index */
	errno = 0;
	src_reg->Index = strtol(tokens.Index.String, NULL, 10);
	if (errno > 0) {
		fprintf(stderr, "Could not convert src register index.\n");
		return 0;
	}

	/* Swizzle */
	if (tokens.Swizzle.Length == 0) {
		src_reg->Swizzle = RC_SWIZZLE_XYZW;
	} else {
		int str_index = 1;
		src_reg->Swizzle = RC_MAKE_SWIZZLE_SMEAR(RC_SWIZZLE_UNUSED);
		if (tokens.Swizzle.String[0] != '.') {
			fprintf(stderr, "First char of swizzle is not valid.\n");
			return 0;
		}
		for (i = 0; i < 4 && str_index < tokens.Swizzle.Length;
							i++, str_index++) {
			if (tokens.Swizzle.String[str_index] == '-') {
				src_reg->Negate |= (1 << i);
				str_index++;
			}
			switch(tokens.Swizzle.String[str_index]) {
			case 'x':
				SET_SWZ(src_reg->Swizzle, i, RC_SWIZZLE_X);
				break;
			case 'y':
				SET_SWZ(src_reg->Swizzle, i, RC_SWIZZLE_Y);
				break;
			case 'z':
				SET_SWZ(src_reg->Swizzle, i, RC_SWIZZLE_Z);
				break;
			case 'w':
				SET_SWZ(src_reg->Swizzle, i, RC_SWIZZLE_W);
				break;
			case '1':
				SET_SWZ(src_reg->Swizzle, i, RC_SWIZZLE_ONE);
				break;
			case '0':
				SET_SWZ(src_reg->Swizzle, i, RC_SWIZZLE_ZERO);
				break;
			case 'H':
				SET_SWZ(src_reg->Swizzle, i, RC_SWIZZLE_HALF);
				break;
			case '_':
				SET_SWZ(src_reg->Swizzle, i, RC_SWIZZLE_UNUSED);
				break;
			default:
				fprintf(stderr, "Unknown src register swizzle: %c\n",
						tokens.Swizzle.String[str_index]);
				return 0;
			}
		}
	}
	DBG("File=%u index=%u swizzle=%x negate=%u abs=%u\n",
			src_reg->File, src_reg->Index, src_reg->Swizzle,
			src_reg->Negate, src_reg->Abs);
	return 1;
}

#define REGEX_DST_MATCHES 4

struct dst_tokens {
	struct match_info File;
	struct match_info Index;
	struct match_info WriteMask;
};

/**
 * Initialize the destination for the instruction based on dst_str.
 *
 * NOTE: Warning in init_rc_normal_instruction() applies to this function as
 * well.
 *
 * @param dst_str A string that represents the destination register.  The format
 * for this string is the same that is output by rc_program_print.
 * @return 1 On success, 0 on failure
 */
int init_rc_normal_dst(
	struct rc_instruction * inst,
	const char * dst_str)
{
	const char * regex_str = "([[:lower:]]*)\\[*([[:digit:]]*)\\]*(\\.*[[:lower:]]*)";
	regmatch_t matches[REGEX_DST_MATCHES];
	struct dst_tokens tokens;
	unsigned int i;

	/* Execute the regex */
	if (!regex_helper(regex_str, dst_str, matches, REGEX_DST_MATCHES)) {
		fprintf(stderr, "Failed to execute regex for dst register.\n");
		return 0;
	}

	/* Create Tokens */
	tokens.File.String = dst_str + matches[1].rm_so;
	tokens.File.Length = match_length(matches, 1);
	tokens.Index.String = dst_str + matches[2].rm_so;
	tokens.Index.Length = match_length(matches, 2);
	tokens.WriteMask.String = dst_str + matches[3].rm_so;
	tokens.WriteMask.Length = match_length(matches, 3);

	/* File Type */
	if (!strncmp(tokens.File.String, "temp", tokens.File.Length)) {
		inst->U.I.DstReg.File = RC_FILE_TEMPORARY;
	} else if (!strncmp(tokens.File.String, "output", tokens.File.Length)) {
		inst->U.I.DstReg.File = RC_FILE_OUTPUT;
	} else if (!strncmp(tokens.File.String, "none", tokens.File.Length)) {
		inst->U.I.DstReg.File = RC_FILE_NONE;
		return 1;
	} else {
		fprintf(stderr, "Unknown dst register file type.\n");
		return 0;
	}

	/* File Index */
	errno = 0;
	inst->U.I.DstReg.Index = strtol(tokens.Index.String, NULL, 10);

	if (errno > 0) {
		fprintf(stderr, "Could not convert dst register index\n");
		return 0;
	}

	/* WriteMask */
	if (tokens.WriteMask.Length == 0) {
		inst->U.I.DstReg.WriteMask = RC_MASK_XYZW;
	} else {
		inst->U.I.DstReg.WriteMask = 0;
		/* The first character should be '.' */
		if (tokens.WriteMask.String[0] != '.') {
			fprintf(stderr, "1st char of writemask is not valid.\n");
			return 0;
		}
		for (i = 1; i < tokens.WriteMask.Length; i++) {
			switch(tokens.WriteMask.String[i]) {
			case 'x':
				inst->U.I.DstReg.WriteMask |= RC_MASK_X;
				break;
			case 'y':
				inst->U.I.DstReg.WriteMask |= RC_MASK_Y;
				break;
			case 'z':
				inst->U.I.DstReg.WriteMask |= RC_MASK_Z;
				break;
			case 'w':
				inst->U.I.DstReg.WriteMask |= RC_MASK_W;
				break;
			default:
				fprintf(stderr, "Unknown swizzle in writemask: %c\n",
							tokens.WriteMask.String[i]);
				return 0;
			}
		}
	}
	DBG("Dst Reg File=%u Index=%d Writemask=%d\n",
			inst->U.I.DstReg.File,
			inst->U.I.DstReg.Index,
			inst->U.I.DstReg.WriteMask);
	return 1;
}

#define REGEX_INST_MATCHES 7
#define REGEX_CONST_MATCHES 5

struct inst_tokens {
	struct match_info Opcode;
	struct match_info Sat;
	struct match_info Dst;
	struct match_info Srcs[3];
};

/**
 * Initialize a normal instruction based on inst_str.
 *
 * WARNING: This function might not be able to handle every kind of format that
 * rc_program_print() can output.  If you are having problems with a
 * particular string, you may need to add support for it to this functions.
 *
 * @param inst_str A string that represents the source register.  The format for
 * this string is the same that is output by rc_program_print.
 * @return 1 On success, 0 on failure
 */

int parse_rc_normal_instruction(
	struct rc_instruction * inst,
	const char * inst_str)
{
	const char * regex_str = "[[:digit:]: ]*([[:upper:][:digit:]]+)(_SAT)*[ ]*([^,;]*)[, ]*([^,;]*)[, ]*([^,;]*)[, ]*([^;]*)";
	int i;
	regmatch_t matches[REGEX_INST_MATCHES];
	struct inst_tokens tokens;

	/* Execute the regex */
	if (!regex_helper(regex_str, inst_str, matches, REGEX_INST_MATCHES)) {
		return 0;
	}
	memset(&tokens, 0, sizeof(tokens));

	/* Create Tokens */
	tokens.Opcode.String = inst_str + matches[1].rm_so;
	tokens.Opcode.Length = match_length(matches, 1);
	if (matches[2].rm_so > -1) {
		tokens.Sat.String = inst_str + matches[2].rm_so;
		tokens.Sat.Length = match_length(matches, 2);
	}


	/* Fill out the rest of the instruction. */
	inst->Type = RC_INSTRUCTION_NORMAL;

	for (i = 0; i < MAX_RC_OPCODE; i++) {
		const struct rc_opcode_info * info = rc_get_opcode_info(i);
		unsigned int first_src = 3;
		unsigned int j;
		if (strncmp(tokens.Opcode.String, info->Name, tokens.Opcode.Length)) {
			continue;
		}
		inst->U.I.Opcode = info->Opcode;
		if (info->HasDstReg) {
			char * dst_str;
			tokens.Dst.String = inst_str + matches[3].rm_so;
			tokens.Dst.Length = match_length(matches, 3);
			first_src++;

			dst_str = malloc(sizeof(char) * (tokens.Dst.Length + 1));
			strncpy(dst_str, tokens.Dst.String, tokens.Dst.Length);
			dst_str[tokens.Dst.Length] = '\0';
			init_rc_normal_dst(inst, dst_str);
			free(dst_str);
		}
		for (j = 0; j < info->NumSrcRegs; j++) {
			char * src_str;
			tokens.Srcs[j].String =
				inst_str + matches[first_src + j].rm_so;
			tokens.Srcs[j].Length =
				match_length(matches, first_src + j);

			src_str = malloc(sizeof(char) *
						(tokens.Srcs[j].Length + 1));
			strncpy(src_str, tokens.Srcs[j].String,
						tokens.Srcs[j].Length);
			src_str[tokens.Srcs[j].Length] = '\0';
			init_rc_normal_src(inst, j, src_str);
			free(src_str);
		}
		if (info->HasTexture) {
			/* XXX: Will this always be XYZW ? */
			inst->U.I.TexSwizzle = RC_SWIZZLE_XYZW;
		}
		break;
	}
	return 1;
}

#define INDEX_TOKEN_LEN 4
#define FLOAT_TOKEN_LEN 50
int parse_constant(unsigned *index, float *data, const char *const_str)
{
	int matched = sscanf(const_str, "const[%d] {%f, %f, %f, %f}", index,
				&data[0], &data[1], &data[2], &data[3]);
	return matched == 5;
}

int init_rc_normal_instruction(
	struct rc_instruction * inst,
	const char * inst_str)
{
	/* Initialize inst */
	memset(inst, 0, sizeof(struct rc_instruction));

	return parse_rc_normal_instruction(inst, inst_str);
}

void add_instruction(struct radeon_compiler *c, const char * inst_string)
{
	struct rc_instruction * new_inst =
		rc_insert_new_instruction(c, c->Program.Instructions.Prev);

	parse_rc_normal_instruction(new_inst, inst_string);

}

int add_constant(struct radeon_compiler *c, const char *const_str)
{
	float data[4];
	unsigned index;
	struct rc_constant_list *constants;
	struct rc_constant constant;

	if (!parse_constant(&index, data, const_str)) {
		return 0;
	}

	constants = &c->Program.Constants;
	if (constants->_Reserved < index) {
		struct rc_constant * newlist;

		constants->_Reserved = index + 100;

		newlist = malloc(sizeof(struct rc_constant) * constants->_Reserved);
		if (constants->Constants) {
			memcpy(newlist, constants->Constants,
				sizeof(struct rc_constant) *
					constants->_Reserved);
			free(constants->Constants);
		}

		constants->Constants = newlist;
	}

	memset(&constant, 0, sizeof(constant));
	constant.Type = RC_CONSTANT_IMMEDIATE;
	constant.Size = 4;
	memcpy(constant.u.Immediate, data, sizeof(float) * 4);
	constants->Constants[index] = constant;
	constants->Count = MAX2(constants->Count, index + 1);

	return 1;
}

void init_compiler(
	struct radeon_compiler *c,
	enum rc_program_type program_type,
	unsigned is_r500,
	unsigned is_r400)
{
	struct rc_regalloc_state *rs = malloc(sizeof(struct rc_regalloc_state));
	rc_init_regalloc_state(rs, program_type);
	rc_init(c, rs);

	c->type = program_type;
	c->is_r500 = is_r500;
	c->max_temp_regs = is_r500 ? 128 : (is_r400 ? 64 : 32);
	c->max_constants = is_r500 ? 256 : 32;
	c->max_alu_insts = (is_r500 || is_r400) ? 512 : 64;
	c->max_tex_insts = (is_r500 || is_r400) ? 512 : 32;
	if (program_type == RC_FRAGMENT_PROGRAM) {
		c->has_half_swizzles = 1;
		c->has_presub = 1;
		c->has_omod = 1;
		c->SwizzleCaps =
			is_r500 ? &r500_swizzle_caps : &r300_swizzle_caps;
	} else {
		c->SwizzleCaps = &r300_vertprog_swizzle_caps;
	}
}

void destroy_compiler(struct radeon_compiler *c)
{
	rc_destroy_regalloc_state((struct rc_regalloc_state *)c->regalloc_state);
	free((void *)c->regalloc_state);
	rc_destroy(c);
}

#define MAX_LINE_LENGTH 100

unsigned load_program(
	struct radeon_compiler *c,
	struct rc_test_file *test,
	const char *filename)
{
	char line[MAX_LINE_LENGTH];
	char path[PATH_MAX];
	FILE *file;
	unsigned *count;
	char **string_store;
	unsigned i = 0;
	int n;

	memset(line, 0, sizeof(line));
	n = snprintf(path, PATH_MAX, TEST_PATH "/%s", filename);
	if (n < 0 || n >= PATH_MAX) {
		return 0;
	}

	file = fopen(path, "r");
	if (!file) {
		return 0;
	}
	memset(test, 0, sizeof(struct rc_test_file));

	count = &test->num_input_lines;

	while (fgets(line, MAX_LINE_LENGTH, file)){
		char last_char = line[MAX_LINE_LENGTH - 1];
		if (last_char && last_char != '\n') {
			fprintf(stderr, "Error line cannot be longer than 100 "
				"characters:\n%s\n", line);
			fclose(file);
			return 0;
		}

		// Comment
		if (line[0] == '#' || is_whitespace(line)) {
			continue;
		}

		if (line[0] == '=') {
			count = &test->num_expected_lines;
			continue;
		}

		(*count)++;
	}

	test->input = malloc(sizeof(char *) * test->num_input_lines);
	test->expected = malloc(sizeof(char *) * test->num_expected_lines);

	rewind(file);
	string_store = test->input;

	while(fgets(line, MAX_LINE_LENGTH, file)) {
		// Comment
		char * dst;
		if (line[0] == '#' || is_whitespace(line)) {
			continue;
		}

		if (line[0] == '=') {
			i = 0;
			string_store = test->expected;
			continue;
		}

		dst = string_store[i++] = malloc((strlen(line) + 1) *
							sizeof (char));
		strcpy(dst, line);
	}

	for (i = 0; i < test->num_input_lines; i++) {
		if (test->input[i][0] == 'c') {
			add_constant(c, test->input[i]);
			free(test->input[i]);
			continue;
		}
		// XXX: Parse immediates from the file.
		add_instruction(c, test->input[i]);
		free(test->input[i]);
	}

	fclose(file);
	free(test->input);
	free(test->expected);
	return 1;
}
