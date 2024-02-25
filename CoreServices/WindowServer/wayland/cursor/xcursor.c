/*
 * Copyright © 2002 Keith Packard
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

#define _GNU_SOURCE
#include "xcursor.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/*
 * Cursor files start with a header.  The header
 * contains a magic number, a version number and a
 * table of contents which has type and offset information
 * for the remaining tables in the file.
 *
 * File minor versions increment for compatible changes
 * File major versions increment for incompatible changes (never, we hope)
 *
 * Chunks of the same type are always upward compatible.  Incompatible
 * changes are made with new chunk types; the old data can remain under
 * the old type.  Upward compatible changes can add header data as the
 * header lengths are specified in the file.
 *
 *  File:
 *	FileHeader
 *	LISTofChunk
 *
 *  FileHeader:
 *	CARD32		magic	    magic number
 *	CARD32		header	    bytes in file header
 *	CARD32		version	    file version
 *	CARD32		ntoc	    number of toc entries
 *	LISTofFileToc   toc	    table of contents
 *
 *  FileToc:
 *	CARD32		type	    entry type
 *	CARD32		subtype	    entry subtype (size for images)
 *	CARD32		position    absolute file position
 */

#define XCURSOR_MAGIC 0x72756358 /* "Xcur" LSBFirst */

/*
 * This version number is stored in cursor files; changes to the
 * file format require updating this version number
 */
#define XCURSOR_FILE_MAJOR 1
#define XCURSOR_FILE_MINOR 0
#define XCURSOR_FILE_VERSION ((XCURSOR_FILE_MAJOR << 16) | (XCURSOR_FILE_MINOR))
#define XCURSOR_FILE_HEADER_LEN (4 * 4)
#define XCURSOR_FILE_TOC_LEN (3 * 4)

struct xcursor_file_toc {
	uint32_t type; /* chunk type */
	uint32_t subtype; /* subtype (size for images) */
	uint32_t position; /* absolute position in file */
};

struct xcursor_file_header {
	uint32_t magic; /* magic number */
	uint32_t header; /* byte length of header */
	uint32_t version; /* file version number */
	uint32_t ntoc; /* number of toc entries */
	struct xcursor_file_toc *tocs; /* table of contents */
};

/*
 * The rest of the file is a list of chunks, each tagged by type
 * and version.
 *
 *  Chunk:
 *	ChunkHeader
 *	<extra type-specific header fields>
 *	<type-specific data>
 *
 *  ChunkHeader:
 *	CARD32	    header	bytes in chunk header + type header
 *	CARD32	    type	chunk type
 *	CARD32	    subtype	chunk subtype
 *	CARD32	    version	chunk type version
 */

#define XCURSOR_CHUNK_HEADER_LEN (4 * 4)

struct xcursor_chunk_header {
	uint32_t header; /* bytes in chunk header */
	uint32_t type; /* chunk type */
	uint32_t subtype; /* chunk subtype (size for images) */
	uint32_t version; /* version of this type */
};

/*
 * Each cursor image occupies a separate image chunk.
 * The length of the image header follows the chunk header
 * so that future versions can extend the header without
 * breaking older applications
 *
 *  Image:
 *	ChunkHeader	header	chunk header
 *	CARD32		width	actual width
 *	CARD32		height	actual height
 *	CARD32		xhot	hot spot x
 *	CARD32		yhot	hot spot y
 *	CARD32		delay	animation delay
 *	LISTofCARD32	pixels	ARGB pixels
 */

#define XCURSOR_IMAGE_TYPE 0xfffd0002
#define XCURSOR_IMAGE_VERSION 1
#define XCURSOR_IMAGE_HEADER_LEN (XCURSOR_CHUNK_HEADER_LEN + (5*4))
#define XCURSOR_IMAGE_MAX_SIZE 0x7fff /* 32767x32767 max cursor size */

/*
 * From libXcursor/src/file.c
 */

static struct xcursor_image *
xcursor_image_create(int width, int height)
{
	struct xcursor_image *image;

	if (width < 0 || height < 0)
		return NULL;
	if (width > XCURSOR_IMAGE_MAX_SIZE || height > XCURSOR_IMAGE_MAX_SIZE)
		return NULL;

	image = malloc(sizeof(struct xcursor_image) +
		       width * height * sizeof(uint32_t));
	if (!image)
		return NULL;
	image->version = XCURSOR_IMAGE_VERSION;
	image->pixels = (uint32_t *) (image + 1);
	image->size = width > height ? width : height;
	image->width = width;
	image->height = height;
	image->delay = 0;
	return image;
}

static void
xcursor_image_destroy(struct xcursor_image *image)
{
	free(image);
}

static struct xcursor_images *
xcursor_images_create(int size)
{
	struct xcursor_images *images;

	images = malloc(sizeof(struct xcursor_images) +
			size * sizeof(struct xcursor_image *));
	if (!images)
		return NULL;
	images->nimage = 0;
	images->images = (struct xcursor_image **) (images + 1);
	images->name = NULL;
	return images;
}

void
xcursor_images_destroy(struct xcursor_images *images)
{
	int n;

	if (!images)
		return;

	for (n = 0; n < images->nimage; n++)
		xcursor_image_destroy(images->images[n]);
	free(images->name);
	free(images);
}

static bool
xcursor_read_uint(FILE *file, uint32_t *u)
{
	unsigned char bytes[4];

	if (!file || !u)
		return false;

	if (fread(bytes, 1, 4, file) != 4)
		return false;

	*u = ((uint32_t)(bytes[0]) << 0) |
		 ((uint32_t)(bytes[1]) << 8) |
		 ((uint32_t)(bytes[2]) << 16) |
		 ((uint32_t)(bytes[3]) << 24);
	return true;
}

static void
xcursor_file_header_destroy(struct xcursor_file_header *file_header)
{
	free(file_header);
}

static struct xcursor_file_header *
xcursor_file_header_create(uint32_t ntoc)
{
	struct xcursor_file_header *file_header;

	if (ntoc > 0x10000)
		return NULL;
	file_header = malloc(sizeof(struct xcursor_file_header) +
			    ntoc * sizeof(struct xcursor_file_toc));
	if (!file_header)
		return NULL;
	file_header->magic = XCURSOR_MAGIC;
	file_header->header = XCURSOR_FILE_HEADER_LEN;
	file_header->version = XCURSOR_FILE_VERSION;
	file_header->ntoc = ntoc;
	file_header->tocs = (struct xcursor_file_toc *) (file_header + 1);
	return file_header;
}

static struct xcursor_file_header *
xcursor_read_file_header(FILE *file)
{
	struct xcursor_file_header head, *file_header;
	uint32_t skip;
	unsigned int n;

	if (!file)
		return NULL;

	if (!xcursor_read_uint(file, &head.magic))
		return NULL;
	if (head.magic != XCURSOR_MAGIC)
		return NULL;
	if (!xcursor_read_uint(file, &head.header))
		return NULL;
	if (!xcursor_read_uint(file, &head.version))
		return NULL;
	if (!xcursor_read_uint(file, &head.ntoc))
		return NULL;
	skip = head.header - XCURSOR_FILE_HEADER_LEN;
	if (skip)
		if (fseek(file, skip, SEEK_CUR) == EOF)
			return NULL;
	file_header = xcursor_file_header_create(head.ntoc);
	if (!file_header)
		return NULL;
	file_header->magic = head.magic;
	file_header->header = head.header;
	file_header->version = head.version;
	file_header->ntoc = head.ntoc;
	for (n = 0; n < file_header->ntoc; n++) {
		if (!xcursor_read_uint(file, &file_header->tocs[n].type))
			break;
		if (!xcursor_read_uint(file, &file_header->tocs[n].subtype))
			break;
		if (!xcursor_read_uint(file, &file_header->tocs[n].position))
			break;
	}
	if (n != file_header->ntoc) {
		xcursor_file_header_destroy(file_header);
		return NULL;
	}
	return file_header;
}

static bool
xcursor_seek_to_toc(FILE *file,
		    struct xcursor_file_header *file_header,
		    int toc)
{
	if (!file || !file_header ||
	    fseek(file, file_header->tocs[toc].position, SEEK_SET) == EOF)
		return false;
	return true;
}

static bool
xcursor_file_read_chunk_header(FILE *file,
			       struct xcursor_file_header *file_header,
			       int toc,
			       struct xcursor_chunk_header *chunk_header)
{
	if (!file || !file_header || !chunk_header)
		return false;
	if (!xcursor_seek_to_toc(file, file_header, toc))
		return false;
	if (!xcursor_read_uint(file, &chunk_header->header))
		return false;
	if (!xcursor_read_uint(file, &chunk_header->type))
		return false;
	if (!xcursor_read_uint(file, &chunk_header->subtype))
		return false;
	if (!xcursor_read_uint(file, &chunk_header->version))
		return false;
	/* sanity check */
	if (chunk_header->type != file_header->tocs[toc].type ||
	    chunk_header->subtype != file_header->tocs[toc].subtype)
		return false;
	return true;
}

static uint32_t
dist(uint32_t a, uint32_t b)
{
	return a > b ? a - b : b - a;
}

static uint32_t
xcursor_file_best_size(struct xcursor_file_header *file_header,
		       uint32_t size, int *nsizesp)
{
	unsigned int n;
	int nsizes = 0;
	uint32_t best_size = 0;
	uint32_t this_size;

	if (!file_header || !nsizesp)
		return 0;

	for (n = 0; n < file_header->ntoc; n++) {
		if (file_header->tocs[n].type != XCURSOR_IMAGE_TYPE)
			continue;
		this_size = file_header->tocs[n].subtype;
		if (!best_size || dist(this_size, size) < dist(best_size, size)) {
			best_size = this_size;
			nsizes = 1;
		} else if (this_size == best_size) {
			nsizes++;
		}
	}
	*nsizesp = nsizes;
	return best_size;
}

static int
xcursor_find_image_toc(struct xcursor_file_header *file_header,
		       uint32_t size, int count)
{
	unsigned int toc;
	uint32_t this_size;

	if (!file_header)
		return 0;

	for (toc = 0; toc < file_header->ntoc; toc++) {
		if (file_header->tocs[toc].type != XCURSOR_IMAGE_TYPE)
			continue;
		this_size = file_header->tocs[toc].subtype;
		if (this_size != size)
			continue;
		if (!count)
			break;
		count--;
	}
	if (toc == file_header->ntoc)
		return -1;
	return toc;
}

static struct xcursor_image *
xcursor_read_image(FILE *file,
		   struct xcursor_file_header *file_header,
		   int toc)
{
	struct xcursor_chunk_header chunk_header;
	struct xcursor_image head;
	struct xcursor_image *image;
	int n;
	uint32_t *p;

	if (!file || !file_header)
		return NULL;

	if (!xcursor_file_read_chunk_header(file, file_header, toc, &chunk_header))
		return NULL;
	if (!xcursor_read_uint(file, &head.width))
		return NULL;
	if (!xcursor_read_uint(file, &head.height))
		return NULL;
	if (!xcursor_read_uint(file, &head.xhot))
		return NULL;
	if (!xcursor_read_uint(file, &head.yhot))
		return NULL;
	if (!xcursor_read_uint(file, &head.delay))
		return NULL;
	/* sanity check data */
	if (head.width > XCURSOR_IMAGE_MAX_SIZE ||
	    head.height > XCURSOR_IMAGE_MAX_SIZE)
		return NULL;
	if (head.width == 0 || head.height == 0)
		return NULL;
	if (head.xhot > head.width || head.yhot > head.height)
		return NULL;

	/* Create the image and initialize it */
	image = xcursor_image_create(head.width, head.height);
	if (image == NULL)
		return NULL;
	if (chunk_header.version < image->version)
		image->version = chunk_header.version;
	image->size = chunk_header.subtype;
	image->xhot = head.xhot;
	image->yhot = head.yhot;
	image->delay = head.delay;
	n = image->width * image->height;
	p = image->pixels;
	while (n--) {
		if (!xcursor_read_uint(file, p)) {
			xcursor_image_destroy(image);
			return NULL;
		}
		p++;
	}
	return image;
}

static struct xcursor_images *
xcursor_xc_file_load_images(FILE *file, int size)
{
	struct xcursor_file_header *file_header;
	uint32_t best_size;
	int nsize;
	struct xcursor_images *images;
	int n;
	int toc;

	if (!file || size < 0)
		return NULL;
	file_header = xcursor_read_file_header(file);
	if (!file_header)
		return NULL;
	best_size = xcursor_file_best_size(file_header, (uint32_t) size, &nsize);
	if (!best_size) {
		xcursor_file_header_destroy(file_header);
		return NULL;
	}
	images = xcursor_images_create(nsize);
	if (!images) {
		xcursor_file_header_destroy(file_header);
		return NULL;
	}
	for (n = 0; n < nsize; n++) {
		toc = xcursor_find_image_toc(file_header, best_size, n);
		if (toc < 0)
			break;
		images->images[images->nimage] = xcursor_read_image(file, file_header,
								    toc);
		if (!images->images[images->nimage])
			break;
		images->nimage++;
	}
	xcursor_file_header_destroy(file_header);
	if (images->nimage != nsize) {
		xcursor_images_destroy(images);
		images = NULL;
	}
	return images;
}

/*
 * From libXcursor/src/library.c
 */

#ifndef ICONDIR
#define ICONDIR "/usr/X11R6/lib/X11/icons"
#endif

#ifndef XCURSORPATH
#define XCURSORPATH "~/.icons:/usr/share/icons:/usr/share/pixmaps:~/.cursors:/usr/share/cursors/xorg-x11:"ICONDIR
#endif

#define XDG_DATA_HOME_FALLBACK "~/.local/share"
#define CURSORDIR "/icons"

/** Get search path for cursor themes
 *
 * This function builds the list of directories to look for cursor
 * themes in.  The format is PATH-like: directories are separated by
 * colons.
 *
 * The memory block returned by this function is allocated on the heap
 * and must be freed by the caller.
 */
static char *
xcursor_library_path(void)
{
	const char *env_var, *suffix;
	char *path;
	size_t path_size;

	env_var = getenv("XCURSOR_PATH");
	if (env_var)
		return strdup(env_var);

	env_var = getenv("XDG_DATA_HOME");
	if (!env_var || env_var[0] != '/')
		env_var = XDG_DATA_HOME_FALLBACK;

	suffix = CURSORDIR ":" XCURSORPATH;
	path_size = strlen(env_var) + strlen(suffix) + 1;
	path = malloc(path_size);
	if (!path)
		return NULL;
	snprintf(path, path_size, "%s%s", env_var, suffix);
	return path;
}

static char *
xcursor_build_theme_dir(const char *dir, const char *theme)
{
	const char *colon;
	const char *tcolon;
	char *full;
	const char *home, *homesep;
	int dirlen;
	int homelen;
	int themelen;
	size_t full_size;

	if (!dir || !theme)
		return NULL;

	colon = strchr(dir, ':');
	if (!colon)
		colon = dir + strlen(dir);

	dirlen = colon - dir;

	tcolon = strchr(theme, ':');
	if (!tcolon)
		tcolon = theme + strlen(theme);

	themelen = tcolon - theme;

	home = "";
	homelen = 0;
	homesep = "";
	if (*dir == '~') {
		home = getenv("HOME");
		if (!home)
			return NULL;
		homelen = strlen(home);
		homesep = "/";
		dir++;
		dirlen--;
	}

	/*
	 * add space for any needed directory separators, one per component,
	 * and one for the trailing null
	 */
	full_size = 1 + homelen + 1 + dirlen + 1 + themelen + 1;
	full = malloc(full_size);
	if (!full)
		return NULL;
	snprintf(full, full_size, "%s%s%.*s/%.*s", home, homesep,
		 dirlen, dir, themelen, theme);
	return full;
}

static char *
xcursor_build_fullname(const char *dir, const char *subdir, const char *file)
{
	char *full;
	size_t full_size;

	if (!dir || !subdir || !file)
		return NULL;

	full_size = strlen(dir) + 1 + strlen(subdir) + 1 + strlen(file) + 1;
	full = malloc(full_size);
	if (!full)
		return NULL;
	snprintf(full, full_size, "%s/%s/%s", dir, subdir, file);
	return full;
}

static const char *
xcursor_next_path(const char *path)
{
	char *colon = strchr(path, ':');

	if (!colon)
		return NULL;
	return colon + 1;
}

static bool
xcursor_white(char c)
{
	return c == ' ' || c == '\t' || c == '\n';
}

static bool
xcursor_sep(char c)
{
	return c == ';' || c == ',';
}

static char *
xcursor_theme_inherits(const char *full)
{
	char *line = NULL;
	size_t line_size = 0;
	char *result = NULL;
	FILE *f;

	if (!full)
		return NULL;

	f = fopen(full, "r");
	if (!f)
		return NULL;

	while (getline(&line, &line_size, f) >= 0) {
		const char *l;
		char *r;

		if (strncmp(line, "Inherits", 8))
			continue;

		l = line + 8;
		while (*l == ' ')
			l++;
		if (*l != '=')
			continue;
		l++;
		while (*l == ' ')
			l++;
		result = malloc(strlen(l) + 1);
		if (!result)
			break;

		r = result;
		while (*l) {
			while (xcursor_sep(*l) || xcursor_white(*l))
				l++;
			if (!*l)
				break;
			if (r != result)
				*r++ = ':';
			while (*l && !xcursor_white(*l) && !xcursor_sep(*l))
				*r++ = *l++;
		}
		*r++ = '\0';

		break;
	}

	fclose(f);
	free(line);

	return result;
}

static void
load_all_cursors_from_dir(const char *path, int size,
			  void (*load_callback)(struct xcursor_images *, void *),
			  void *user_data)
{
	FILE *f;
	DIR *dir = opendir(path);
	struct dirent *ent;
	char *full;
	struct xcursor_images *images;

	if (!dir)
		return;

	for (ent = readdir(dir); ent; ent = readdir(dir)) {
#ifdef _DIRENT_HAVE_D_TYPE
		if (ent->d_type != DT_UNKNOWN &&
		    ent->d_type != DT_REG &&
		    ent->d_type != DT_LNK)
			continue;
#endif

		full = xcursor_build_fullname(path, "", ent->d_name);
		if (!full)
			continue;

		f = fopen(full, "r");
		if (!f) {
			free(full);
			continue;
		}

		images = xcursor_xc_file_load_images(f, size);

		if (images) {
			images->name = strdup(ent->d_name);
			load_callback(images, user_data);
		}

		fclose(f);
		free(full);
	}

	closedir(dir);
}

/** Load all the cursor of a theme
 *
 * This function loads all the cursor images of a given theme and its
 * inherited themes. Each cursor is loaded into an struct xcursor_images object
 * which is passed to the caller's load callback. If a cursor appears
 * more than once across all the inherited themes, the load callback
 * will be called multiple times, with possibly different struct xcursor_images
 * object which have the same name. The user is expected to destroy the
 * struct xcursor_images objects passed to the callback with
 * xcursor_images_destroy().
 *
 * \param theme The name of theme that should be loaded
 * \param size The desired size of the cursor images
 * \param load_callback A callback function that will be called
 * for each cursor loaded. The first parameter is the struct xcursor_images
 * object representing the loaded cursor and the second is a pointer
 * to data provided by the user.
 * \param user_data The data that should be passed to the load callback
 */
void
xcursor_load_theme(const char *theme, int size,
		   void (*load_callback)(struct xcursor_images *, void *),
		   void *user_data)
{
	char *full, *dir;
	char *inherits = NULL;
	const char *path, *i;
	char *xcursor_path;

	if (!theme)
		theme = "default";

	xcursor_path = xcursor_library_path();
	for (path = xcursor_path;
	     path;
	     path = xcursor_next_path(path)) {
		dir = xcursor_build_theme_dir(path, theme);
		if (!dir)
			continue;

		full = xcursor_build_fullname(dir, "cursors", "");
		load_all_cursors_from_dir(full, size, load_callback,
					  user_data);
		free(full);

		if (!inherits) {
			full = xcursor_build_fullname(dir, "", "index.theme");
			inherits = xcursor_theme_inherits(full);
			free(full);
		}

		free(dir);
	}

	for (i = inherits; i; i = xcursor_next_path(i))
		xcursor_load_theme(i, size, load_callback, user_data);

	free(inherits);
	free(xcursor_path);
}
