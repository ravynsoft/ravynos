// SPDX-License-Identifier: GPL-2.0-only
/*
 * Find the configuration and theme directories
 *
 * Copyright Johan Malm 2020
 */

#include <glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "common/dir.h"

struct dir {
	const char *prefix;
	const char *path;
};

static struct dir config_dirs[] = {
	{ "XDG_CONFIG_HOME", "labwc" },
	{ "HOME", ".config/labwc" },
	{ "XDG_CONFIG_DIRS", "labwc" },
	{ NULL, "/usr/etc/xdg/labwc" },
	{ NULL, NULL }
};

static struct dir theme_dirs[] = {
	{ "XDG_DATA_HOME", "themes" },
	{ "HOME", ".local/share/themes" },
	{ "HOME", ".themes" },
	{ "XDG_DATA_DIRS", "themes" },
	{ NULL, "/usr/share/themes" },
	{ NULL, "/usr/share/themes" },
	{ NULL, "opt/share/themes" },
	{ NULL, NULL }
};

static bool
isdir(const char *path)
{
	struct stat st;
	return (!stat(path, &st) && S_ISDIR(st.st_mode));
}

struct ctx {
	void (*build_path_fn)(struct ctx *ctx, char *prefix, const char *path);
	char *buf;
	size_t len;
	struct dir *dirs;
	const char *theme_name;
};

static void
build_config_path(struct ctx *ctx, char *prefix, const char *path)
{
	if (!prefix) {
		snprintf(ctx->buf, ctx->len, "%s", path);
	} else {
		snprintf(ctx->buf, ctx->len, "%s/%s", prefix, path);
	}
}

static void
build_theme_path(struct ctx *ctx, char *prefix, const char *path)
{
	if (!prefix) {
		snprintf(ctx->buf, ctx->len, "%s/%s/openbox-3", path,
			 ctx->theme_name);
	} else {
		snprintf(ctx->buf, ctx->len, "%s/%s/%s/openbox-3", prefix, path,
			 ctx->theme_name);
	}
}

char *
find_dir(struct ctx *ctx)
{
	char *debug = getenv("LABWC_DEBUG_DIR_CONFIG_AND_THEME");

	for (int i = 0; ctx->dirs[i].path; i++) {
		struct dir d = ctx->dirs[i];
		if (!d.prefix) {
			/* handle /usr/etc/xdg... */
			ctx->build_path_fn(ctx, NULL, d.path);
			if (debug) {
				fprintf(stderr, "%s\n", ctx->buf);
			}
			if (isdir(ctx->buf)) {
				return ctx->buf;
			}
		} else {
			/* handle $HOME/.config/... and $XDG_* */
			char *prefix = getenv(d.prefix);
			if (!prefix) {
				continue;
			}
			gchar **prefixes;
			prefixes = g_strsplit(prefix, ":", -1);
			for (gchar **p = prefixes; *p; p++) {
				ctx->build_path_fn(ctx, *p, d.path);
				if (debug) {
					fprintf(stderr, "%s\n", ctx->buf);
				}
				if (isdir(ctx->buf)) {
					g_strfreev(prefixes);
					return ctx->buf;
				}
			}
			g_strfreev(prefixes);
		}
	}
	/* no directory was found */
	ctx->buf[0] = '\0';
	return ctx->buf;
}

char *
config_dir(void)
{
	static char buf[4096] = { 0 };
	if (buf[0] != '\0') {
		return buf;
	}
	struct ctx ctx = { .build_path_fn = build_config_path,
			   .buf = buf,
			   .len = sizeof(buf),
			   .dirs = config_dirs };
	return find_dir(&ctx);
}

char *
theme_dir(const char *theme_name)
{
	static char buf[4096] = { 0 };
	struct ctx ctx = { .build_path_fn = build_theme_path,
			   .buf = buf,
			   .len = sizeof(buf),
			   .dirs = theme_dirs,
			   .theme_name = theme_name };
	return find_dir(&ctx);
}
