/*
 * Copyright © 2012 Philipp Brüschweiler
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
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * This is a small, hacky tool to extract cursors from a .pcf file.
 * The information about the file format has been gathered from
 * http://fontforge.org/pcf-format.html
 */

#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

struct glyph {
	char *name;
	int16_t left_bearing, right_bearing, ascent, descent;

	int16_t width, height;
	int16_t hotx, hoty;

	int32_t data_format;
	char *data;
};

static struct {
	int count;
	struct glyph *glyphs;
} extracted_font = {0, NULL};

#define PCF_PROPERTIES		    (1<<0)
#define PCF_ACCELERATORS	    (1<<1)
#define PCF_METRICS		    (1<<2)
#define PCF_BITMAPS		    (1<<3)
#define PCF_INK_METRICS		    (1<<4)
#define	PCF_BDF_ENCODINGS	    (1<<5)
#define PCF_SWIDTHS		    (1<<6)
#define PCF_GLYPH_NAMES		    (1<<7)
#define PCF_BDF_ACCELERATORS	    (1<<8)

#define PCF_DEFAULT_FORMAT	0x00000000
#define PCF_INKBOUNDS		0x00000200
#define PCF_ACCEL_W_INKBOUNDS	0x00000100
#define PCF_COMPRESSED_METRICS	0x00000100

#define	PCF_FORMAT_MASK		0xffffff00

struct pcf_header {
	char header[4];
	int32_t table_count;
	struct toc_entry {
		int32_t type;
		int32_t format;
		int32_t size;
		int32_t offset;
	} tables[0];
};

struct compressed_metrics {
	uint8_t left_sided_bearing;
	uint8_t right_side_bearing;
	uint8_t character_width;
	uint8_t character_ascent;
	uint8_t character_descent;
};

struct uncompressed_metrics {
	int16_t left_sided_bearing;
	int16_t right_side_bearing;
	int16_t character_width;
	int16_t character_ascent;
	int16_t character_descent;
	uint16_t character_attributes;
};

struct metrics {
	int32_t format;
	union {
		struct {
			int16_t count;
			struct compressed_metrics compressed_metrics[0];
		} compressed;
		struct {
			int32_t count;
			struct uncompressed_metrics uncompressed_metrics[0];
		} uncompressed;
	};
};

struct glyph_names {
	int32_t format;
	int32_t glyph_count;
	int32_t offsets[0];
};

struct bitmaps {
	int32_t format;
	int32_t glyph_count;
	int32_t offsets[0];
};

static void
handle_compressed_metrics(int32_t count, struct compressed_metrics *m)
{
	printf("metrics count: %d\n", count);
	extracted_font.count = count;
	extracted_font.glyphs = calloc(count, sizeof(struct glyph));

	int i;
	for (i = 0; i < count; ++i) {
		struct glyph *glyph = &extracted_font.glyphs[i];
		glyph->left_bearing =
			((int16_t) m[i].left_sided_bearing) - 0x80;
		glyph->right_bearing =
			((int16_t) m[i].right_side_bearing) - 0x80;
		glyph->width = ((int16_t) m[i].character_width) - 0x80;
		glyph->ascent = ((int16_t) m[i].character_ascent) - 0x80;
		glyph->descent = ((int16_t) m[i].character_descent) - 0x80;

		/* computed stuff */
		glyph->height = glyph->ascent + glyph->descent;

		glyph->hotx = -glyph->left_bearing;
		glyph->hoty = glyph->ascent;
	}
}

static void
handle_metrics(void *metricbuf)
{
	struct metrics *metrics = metricbuf;
	printf("metric format: %x\n", metrics->format);

	if ((metrics->format & PCF_FORMAT_MASK) == PCF_DEFAULT_FORMAT) {
		printf("todo...\n");
	} else if ((metrics->format & PCF_FORMAT_MASK) ==
		   PCF_COMPRESSED_METRICS) {
		handle_compressed_metrics(
		    metrics->compressed.count,
		    &metrics->compressed.compressed_metrics[0]);
	} else {
		printf("incompatible format\n");
		abort();
	}
}

static void
handle_glyph_names(struct glyph_names *names)
{
	printf("glyph count %d\n", names->glyph_count);

	if (names->glyph_count != extracted_font.count) {
		abort();
	}

	printf("glyph names format %x\n", names->format);

	void *names_start = ((void*) names) + sizeof(struct glyph_names)
		+ (names->glyph_count + 1) * sizeof(int32_t);

	int i;
	for (i = 0; i < names->glyph_count; ++i) {
		int32_t start = names->offsets[i];
		int32_t end = names->offsets[i+1];
		char *name = names_start + start;
		extracted_font.glyphs[i].name = calloc(1, end - start + 1);
		memcpy(extracted_font.glyphs[i].name, name, end - start);
	}
}

static void
handle_bitmaps(struct bitmaps *bitmaps)
{
	printf("bitmaps count %d\n", bitmaps->glyph_count);

	if (bitmaps->glyph_count != extracted_font.count) {
		abort();
	}

	printf("format %x\n", bitmaps->format);

	if (bitmaps->format != 2) {
		printf("format not yet supported\n");
		abort();
	}

	void *bitmaps_start = ((void*) bitmaps) + sizeof(struct bitmaps)
		+ (bitmaps->glyph_count + 4) * sizeof(int32_t);

	int i;
	for (i = 0; i < bitmaps->glyph_count; ++i) {
		int32_t offset = bitmaps->offsets[i];
		struct glyph *glyph = &extracted_font.glyphs[i];
		glyph->data_format = bitmaps->format;

		glyph->data = bitmaps_start + offset;
	}
}

static void
handle_pcf(void *fontbuf)
{
	struct pcf_header *header = fontbuf;
	printf("tablecount %d\n", header->table_count);

	int i;
	for (i = 0; i < header->table_count; ++i) {
		struct toc_entry *entry = &header->tables[i];
		printf("type: %d\n", entry->type);
		if (entry->type == PCF_METRICS) {
			handle_metrics(fontbuf + entry->offset);
		} else if (entry->type == PCF_GLYPH_NAMES) {
			handle_glyph_names(fontbuf + entry->offset);
		} else if (entry->type == PCF_BITMAPS) {
			handle_bitmaps(fontbuf + entry->offset);
		}
	}
}

static char
get_glyph_pixel(struct glyph *glyph, int x, int y)
{
	int absx = glyph->hotx + x;
	int absy = glyph->hoty + y;

	if (absx < 0 || absx >= glyph->width ||
	    absy < 0 || absy >= glyph->height)
		return 0;

	int stride = (glyph->width + 31) / 32 * 4;
	unsigned char block = glyph->data[absy * stride + (absx/8)];
	int idx = absx % 8;
	return (block >> idx) & 1;
}

static struct {
	uint32_t *data;
	size_t capacity, size;
} data_buffer;

static void
init_data_buffer()
{
	data_buffer.data = malloc(sizeof(uint32_t) * 10);
	data_buffer.capacity = 10;
	data_buffer.size = 0;
}

static void
add_pixel(uint32_t pixel)
{
	if (data_buffer.size == data_buffer.capacity) {
		data_buffer.capacity *= 2;
		data_buffer.data =
			realloc(data_buffer.data,
				sizeof(uint32_t) * data_buffer.capacity);
	}
	data_buffer.data[data_buffer.size++] = pixel;
}

struct reconstructed_glyph {
	int32_t width, height;
	int32_t hotspot_x, hotspot_y;
	size_t offset;
	char *name;
};

static void
reconstruct_glyph(struct glyph *cursor, struct glyph *mask, char *name,
		  struct reconstructed_glyph *glyph)
{
	int minx = min(-cursor->hotx, -mask->hotx);
	int maxx = max(cursor->right_bearing, mask->right_bearing);

	int miny = min(-cursor->hoty, -mask->hoty);
	int maxy = max(cursor->height - cursor->hoty,
		       mask->height - mask->hoty);

	int width = maxx - minx;
	int height = maxy - miny;

	glyph->name = strdup(name);
	glyph->width = width;
	glyph->height = height;
	glyph->hotspot_x = -minx;
	glyph->hotspot_y = -miny;
	glyph->offset = data_buffer.size;

	int x, y;
	for (y = miny; y < maxy; ++y) {
		for (x = minx; x < maxx; ++x) {
			char alpha = get_glyph_pixel(mask, x, y);
			if (alpha) {
				char color = get_glyph_pixel(cursor, x, y);
				if (color)
					add_pixel(0xff000000);
				else
					add_pixel(0xffffffff);
			} else {
				add_pixel(0);
			}
		}
	}
}

/* 
 * Originally from
 * http://cgit.freedesktop.org/xorg/lib/libXfont/tree/src/builtins/fonts.c
 * Changed to the MIT "Expat" style license for Wayland..
 */
static const char cursor_licence[] =
	"/*\n"
	"* Copyright 1999 SuSE, Inc.\n"
	"*\n"
	"* Permission is hereby granted, free of charge, to any person obtaining\n"
	"* a copy of this software and associated documentation files (the\n"
	"* \"Software\"), to deal in the Software without restriction, including\n"
	"* without limitation the rights to use, copy, modify, merge, publish,\n"
	"* distribute, sublicense, and/or sell copies of the Software, and to\n"
	"* permit persons to whom the Software is furnished to do so, subject to\n"
	"* the following conditions:\n"
	"*\n"
	"* The above copyright notice and this permission notice (including the\n"
	"* next paragraph) shall be included in all copies or substantial\n"
	"* portions of the Software.\n"
	"*\n"
	"* THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
	"* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
	"* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n"
	"* NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS\n"
	"* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN\n"
	"* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN\n"
	"* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
	"* SOFTWARE.\n"
	"*\n"
	"* Author:  Keith Packard, SuSE, Inc.\n"
	"*/\n";

static void
write_output_file(struct reconstructed_glyph *glyphs, int n)
{
	int i, j, counter, size;
	FILE *file = fopen("cursor-data.h", "w");
	uint32_t *data;

	fprintf(file, "%s\n", cursor_licence);

	fprintf(file, "static uint32_t cursor_data[] = {\n\t");

	counter = 0;
	for (i = 0; i < n; ++i) {
		data = data_buffer.data + glyphs[i].offset;
		size = glyphs[i].width * glyphs[i].height;

		for (j = 0; j < size; ++j) {
			fprintf(file, "0x%08x, ", data[j]);
			if (++counter % 6 == 0)
				fprintf(file, "\n\t");
		}
	}
	fprintf(file, "\n};\n\n");

	fprintf(file,
		"static struct {\n"
		"\tchar *name;\n"
		"\tint width, height;\n"
		"\tint hotspot_x, hotspot_y;\n"
		"\tsize_t offset;\n"
		"} cursor_metadata[] = {\n");

	for (i = 0; i < n; ++i)
		fprintf(file, "\t{ \"%s\", %d, %d, %d, %d, %zu },\n",
			glyphs[i].name,
			glyphs[i].width, glyphs[i].height,
			glyphs[i].hotspot_x, glyphs[i].hotspot_y,
			glyphs[i].offset);

	fprintf(file, "};");

	fclose(file);
}

struct glyph *
find_mask_glyph(char *name)
{
	const char mask[] = "_mask";
	const int masklen = strlen(mask);

	int len = strlen(name);
	int i;
	for (i = 0; i < extracted_font.count; ++i) {
		struct glyph *g = &extracted_font.glyphs[i];
		int l2 = strlen(g->name);
		if ((l2 == len + masklen) &&
		    (memcmp(g->name, name, len) == 0) &&
		    (memcmp(g->name + len, mask, masklen) == 0)) {
			return g;
		}
	}
	return NULL;
}

static void
output_all_cursors()
{
	int i, j;
	struct reconstructed_glyph *glyphs =
		malloc(sizeof(struct reconstructed_glyph) *
		       extracted_font.count/2);
	j = 0;

	for (i = 0; i < extracted_font.count; ++i) {
		struct glyph *g = &extracted_font.glyphs[i];
		if (strstr(g->name, "_mask"))
			continue;

		struct glyph *mask = find_mask_glyph(g->name);

		reconstruct_glyph(g, mask, g->name, &glyphs[j]);
		j++;
	}

	write_output_file(glyphs, extracted_font.count/2);
}

static void
find_cursor_and_mask(const char *name,
		     struct glyph **cursor,
		     struct glyph **mask)
{
	int i;
	char mask_name[100];
	sprintf(mask_name, "%s_mask", name);

	*cursor = *mask = NULL;

	for (i = 0; i < extracted_font.count && (!*mask || !*cursor); ++i) {
		struct glyph *g = &extracted_font.glyphs[i];
		if (!strcmp(name, g->name))
			*cursor = g;
		else if (!strcmp(mask_name, g->name))
			*mask = g;
	}
}

static struct {
	char *target_name, *source_name;
} interesting_cursors[] = {
	{ "bottom_left_corner", "bottom_left_corner" },
	{ "bottom_right_corner", "bottom_right_corner" },
	{ "bottom_side", "bottom_side" },
	{ "grabbing", "fleur" },
	{ "left_ptr", "left_ptr" },
	{ "left_side", "left_side" },
	{ "right_side", "right_side" },
	{ "top_left_corner", "top_left_corner" },
	{ "top_right_corner", "top_right_corner" },
	{ "top_side", "top_side" },
	{ "xterm", "xterm" },
	{ "hand1", "hand1" },
	{ "watch", "watch" }
};

static void
output_interesting_cursors()
{
	int i;
	int n = sizeof(interesting_cursors) / sizeof(interesting_cursors[0]);
	struct reconstructed_glyph *glyphs =
		malloc(n * sizeof(*glyphs));

	if (!glyphs) {
		printf("reconstructed_glyph malloc failed\n");
		abort();
	}

	for (i = 0; i < n; ++i) {
		struct glyph *cursor, *mask;
		find_cursor_and_mask(interesting_cursors[i].source_name,
				     &cursor, &mask);
		if (!cursor) {
			printf("no cursor for %s\n",
			       interesting_cursors[i].source_name);
			abort();
		}
		if (!mask) {
			printf("no mask for %s\n",
			       interesting_cursors[i].source_name);
			abort();
		}
		reconstruct_glyph(cursor, mask,
				  interesting_cursors[i].target_name,
				  &glyphs[i]);
	}

	write_output_file(glyphs, n);
}

int main()
{
	const char filename[] = "cursor.pcf";

	int fd = open(filename, O_RDONLY);
	struct stat filestat;

	fstat(fd, &filestat);

	void *fontbuf = mmap(NULL, filestat.st_size, PROT_READ,
			     MAP_PRIVATE, fd, 0);

	handle_pcf(fontbuf);

	init_data_buffer();

	//output_all_cursors();
	output_interesting_cursors();
}
