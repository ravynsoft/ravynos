/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *      Marek Olšák <maraeo@gmail.com>
 */
#include "r600_pipe.h"
#include "evergreend.h"

#include "egd_tables.h"

#define AC_IS_TRACE_POINT(x)            (((x) & 0xcafe0000) == 0xcafe0000)
#define AC_GET_TRACE_POINT_ID(x)        ((x) & 0xffff)

/* Parsed IBs are difficult to read without colors. Use "less -R file" to
 * read them, or use "aha -b -f file" to convert them to html.
 */
#define COLOR_RESET	"\033[0m"
#define COLOR_RED	"\033[31m"
#define COLOR_GREEN	"\033[1;32m"
#define COLOR_YELLOW	"\033[1;33m"
#define COLOR_CYAN	"\033[1;36m"

#define INDENT_PKT 8

typedef void *(*ac_debug_addr_callback)(void *data, uint64_t addr);
static void print_spaces(FILE *f, unsigned num)
{
	fprintf(f, "%*s", num, "");
}

static void print_value(FILE *file, uint32_t value, int bits)
{
	/* Guess if it's int or float */
	if (value <= (1 << 15)) {
		if (value <= 9)
			fprintf(file, "%u\n", value);
		else
			fprintf(file, "%u (0x%0*x)\n", value, bits / 4, value);
	} else {
		float f = uif(value);

		if (fabs(f) < 100000 && f*10 == floor(f*10))
			fprintf(file, "%.1ff (0x%0*x)\n", f, bits / 4, value);
		else
			/* Don't print more leading zeros than there are bits. */
			fprintf(file, "0x%0*x\n", bits / 4, value);
	}
}

static void print_named_value(FILE *file, const char *name, uint32_t value,
			      int bits)
{
	print_spaces(file, INDENT_PKT);
	fprintf(file, COLOR_YELLOW "%s" COLOR_RESET " <- ", name);
	print_value(file, value, bits);
}

static void eg_dump_reg(FILE *file, unsigned offset, uint32_t value,
			uint32_t field_mask)
{
	unsigned r, f;

	for (r = 0; r < ARRAY_SIZE(egd_reg_table); r++) {
		const struct eg_reg *reg = &egd_reg_table[r];
		const char *reg_name = egd_strings + reg->name_offset;

		if (reg->offset == offset) {
			bool first_field = true;

			print_spaces(file, INDENT_PKT);
			fprintf(file, COLOR_YELLOW "%s" COLOR_RESET " <- ",
				reg_name);

			if (!reg->num_fields) {
				print_value(file, value, 32);
				return;
			}

			for (f = 0; f < reg->num_fields; f++) {
				const struct eg_field *field = egd_fields_table + reg->fields_offset + f;
				const int *values_offsets = egd_strings_offsets + field->values_offset;
				uint32_t val = (value & field->mask) >>
					       (ffs(field->mask) - 1);

				if (!(field->mask & field_mask))
					continue;

				/* Indent the field. */
				if (!first_field)
					print_spaces(file,
						     INDENT_PKT + strlen(reg_name) + 4);

				/* Print the field. */
				fprintf(file, "%s = ", egd_strings + field->name_offset);

				if (val < field->num_values && values_offsets[val] >= 0)
					fprintf(file, "%s\n", egd_strings + values_offsets[val]);
				else
					print_value(file, val,
						    util_bitcount(field->mask));

				first_field = false;
			}
			return;
		}
	}

	print_spaces(file, INDENT_PKT);
	fprintf(file, COLOR_YELLOW "0x%05x" COLOR_RESET " <- 0x%08x\n", offset, value);
}


static void ac_parse_set_reg_packet(FILE *f, uint32_t *ib, unsigned count,
				    unsigned reg_offset)
{
	unsigned reg = (ib[1] << 2) + reg_offset;
	unsigned i;

	for (i = 0; i < count; i++)
		eg_dump_reg(f, reg + i*4, ib[2+i], ~0);
}

static uint32_t *ac_parse_packet3(FILE *f, uint32_t *ib, int *num_dw,
				  int trace_id, enum amd_gfx_level gfx_level,
				  ac_debug_addr_callback addr_callback,
				  void *addr_callback_data)
{
	unsigned count = PKT_COUNT_G(ib[0]);
	unsigned op = PKT3_IT_OPCODE_G(ib[0]);
	const char *predicate = PKT3_PREDICATE(ib[0]) ? "(predicate)" : "";
	const char *compute_mode = (ib[0] & 0x2) ? "(C)" : "";
	unsigned i;

	/* Print the name first. */
	for (i = 0; i < ARRAY_SIZE(packet3_table); i++)
		if (packet3_table[i].op == op)
			break;

	if (i < ARRAY_SIZE(packet3_table)) {
		const char *name = egd_strings + packet3_table[i].name_offset;

		if (op == PKT3_SET_CONTEXT_REG ||
		    op == PKT3_SET_CONFIG_REG ||
		    op == PKT3_SET_UCONFIG_REG ||
		    op == PKT3_SET_SH_REG)
			fprintf(f, COLOR_CYAN "%s%s%s" COLOR_CYAN ":\n",
				name, compute_mode, predicate);
		else
			fprintf(f, COLOR_GREEN "%s%s%s" COLOR_RESET ":\n",
				name, compute_mode, predicate);
	} else
		fprintf(f, COLOR_RED "PKT3_UNKNOWN 0x%x%s%s" COLOR_RESET ":\n",
			op, compute_mode, predicate);

	/* Print the contents. */
	switch (op) {
	case PKT3_SET_CONTEXT_REG:
		ac_parse_set_reg_packet(f, ib, count, EVERGREEN_CONTEXT_REG_OFFSET);
		break;
	case PKT3_SET_CONFIG_REG:
		ac_parse_set_reg_packet(f, ib, count, EVERGREEN_CONFIG_REG_OFFSET);
		break;
	case PKT3_SURFACE_SYNC:
		eg_dump_reg(f, R_0085F0_CP_COHER_CNTL, ib[1], ~0);
		eg_dump_reg(f, R_0085F4_CP_COHER_SIZE, ib[2], ~0);
		eg_dump_reg(f, R_0085F8_CP_COHER_BASE, ib[3], ~0);
		print_named_value(f, "POLL_INTERVAL", ib[4], 16);
		break;
	case PKT3_EVENT_WRITE:
		/* TODO dump VGT_EVENT_INITIATOR */
#if 0
		eg_dump_reg(f, R_028A90_VGT_EVENT_INITIATOR, ib[1],
			    S_028A90_EVENT_TYPE(~0));
#endif
		print_named_value(f, "EVENT_TYPE", ib[1] & 0xff, 8);
		print_named_value(f, "EVENT_INDEX", (ib[1] >> 8) & 0xf, 4);
		print_named_value(f, "INV_L2", (ib[1] >> 20) & 0x1, 1);
		if (count > 0) {
			print_named_value(f, "ADDRESS_LO", ib[2], 32);
			print_named_value(f, "ADDRESS_HI", ib[3], 16);
		}
		break;
	case PKT3_DRAW_INDEX_AUTO:
		eg_dump_reg(f, R_008970_VGT_NUM_INDICES, ib[1], ~0);
		eg_dump_reg(f, R_0287F0_VGT_DRAW_INITIATOR, ib[2], ~0);
		break;
	case PKT3_DRAW_INDEX_2:
		eg_dump_reg(f, R_028A78_VGT_DMA_MAX_SIZE, ib[1], ~0);
		eg_dump_reg(f, R_0287E8_VGT_DMA_BASE, ib[2], ~0);
		eg_dump_reg(f, R_0287E4_VGT_DMA_BASE_HI, ib[3], ~0);
		eg_dump_reg(f, R_008970_VGT_NUM_INDICES, ib[4], ~0);
		eg_dump_reg(f, R_0287F0_VGT_DRAW_INITIATOR, ib[5], ~0);
		break;
	case PKT3_INDEX_TYPE:
		eg_dump_reg(f, R_028A7C_VGT_DMA_INDEX_TYPE, ib[1], ~0);
		break;
	case PKT3_NUM_INSTANCES:
		eg_dump_reg(f, R_028A88_VGT_NUM_INSTANCES, ib[1], ~0);
		break;
	case PKT3_INDIRECT_BUFFER:
		break;
	case PKT3_PFP_SYNC_ME:
		break;
	case PKT3_NOP:
		if (ib[0] == 0xffff1000) {
			count = -1; /* One dword NOP. */
			break;
		} else if (count == 0 && AC_IS_TRACE_POINT(ib[1])) {
			unsigned packet_id = AC_GET_TRACE_POINT_ID(ib[1]);

			print_spaces(f, INDENT_PKT);
			fprintf(f, COLOR_RED "Trace point ID: %u\n", packet_id);

			if (trace_id == -1)
				break; /* tracing was disabled */

			print_spaces(f, INDENT_PKT);
			if (packet_id < trace_id)
				fprintf(f, COLOR_RED
					"This trace point was reached by the CP."
					COLOR_RESET "\n");
			else if (packet_id == trace_id)
				fprintf(f, COLOR_RED
					"!!!!! This is the last trace point that "
					"was reached by the CP !!!!!"
					COLOR_RESET "\n");
			else if (packet_id+1 == trace_id)
				fprintf(f, COLOR_RED
					"!!!!! This is the first trace point that "
					"was NOT been reached by the CP !!!!!"
					COLOR_RESET "\n");
			else
				fprintf(f, COLOR_RED
					"!!!!! This trace point was NOT reached "
					"by the CP !!!!!"
					COLOR_RESET "\n");
			break;
		}
		FALLTHROUGH; /* print all dwords */
	default:
		for (i = 0; i < count+1; i++) {
			print_spaces(f, INDENT_PKT);
			fprintf(f, "0x%08x\n", ib[1+i]);
		}
	}

	ib += count + 2;
	*num_dw -= count + 2;
	return ib;
}

/**
 * Parse and print an IB into a file.
 *
 * \param f		file
 * \param ib		IB
 * \param num_dw	size of the IB
 * \param gfx_level	gfx level
 * \param trace_id	the last trace ID that is known to have been reached
 *			and executed by the CP, typically read from a buffer
 * \param addr_callback Get a mapped pointer of the IB at a given address. Can
 *                      be NULL.
 * \param addr_callback_data user data for addr_callback
 */
static void eg_parse_ib(FILE *f, uint32_t *ib, int num_dw, int trace_id,
			const char *name, enum amd_gfx_level gfx_level,
			ac_debug_addr_callback addr_callback, void *addr_callback_data)
{
	fprintf(f, "------------------ %s begin ------------------\n", name);

	while (num_dw > 0) {
		unsigned type = PKT_TYPE_G(ib[0]);

		switch (type) {
		case 3:
			ib = ac_parse_packet3(f, ib, &num_dw, trace_id,
					      gfx_level, addr_callback,
					      addr_callback_data);
			break;
		case 2:
			/* type-2 nop */
			if (ib[0] == 0x80000000) {
				fprintf(f, COLOR_GREEN "NOP (type 2)" COLOR_RESET "\n");
				ib++;
				num_dw--;
				break;
			}
			FALLTHROUGH;
		default:
			fprintf(f, "Unknown packet type %i\n", type);
			return;
		}
	}

	fprintf(f, "------------------- %s end -------------------\n", name);
	if (num_dw < 0) {
		printf("Packet ends after the end of IB.\n");
		exit(0);
	}
	fprintf(f, "\n");
}

static void eg_dump_last_ib(struct r600_context *rctx, FILE *f)
{
	int last_trace_id = -1;

	if (!rctx->last_gfx.ib)
		return;

	if (rctx->last_trace_buf) {
		/* We are expecting that the ddebug pipe has already
		 * waited for the context, so this buffer should be idle.
		 * If the GPU is hung, there is no point in waiting for it.
		 */
		uint32_t *map = rctx->b.ws->buffer_map(rctx->b.ws, rctx->last_trace_buf->buf,
						       NULL,
						       PIPE_MAP_UNSYNCHRONIZED |
						       PIPE_MAP_READ);
		if (map)
			last_trace_id = *map;
	}

	eg_parse_ib(f, rctx->last_gfx.ib, rctx->last_gfx.num_dw,
		    last_trace_id, "IB", rctx->b.gfx_level,
		     NULL, NULL);
}


void eg_dump_debug_state(struct pipe_context *ctx, FILE *f,
			 unsigned flags)
{
	struct r600_context *rctx = (struct r600_context*)ctx;

	eg_dump_last_ib(rctx, f);

	fprintf(f, "Done.\n");

	/* dump only once */
	radeon_clear_saved_cs(&rctx->last_gfx);
	r600_resource_reference(&rctx->last_trace_buf, NULL);
}
