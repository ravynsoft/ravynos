/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/acl.h>
//#include <membership.h>
#include <xpc/xpc.h>
#include <TargetConditionals.h>
#include <asl.h>
#include <asl_core.h>
#include <asl_msg.h>
#include "asl_common.h"

#define _PATH_ASL_CONF "/etc/asl.conf"
#define _PATH_ASL_CONF_DIR "/etc/asl"

#define PATH_VAR_LOG "/var/log/"
#define PATH_VAR_LOG_LEN 9

#define PATH_LIBRARY_LOGS "/Library/Logs/"
#define PATH_LIBRARY_LOGS_LEN 14

#if !TARGET_IPHONE_SIMULATOR
#define _PATH_ASL_CONF_LOCAL_DIR "/usr/local/etc/asl"
#endif

static const char *asl_out_action_name[] =
{
	"none         ",
	"set          ",
	"output       ",
	"ignore       ",
	"skip         ",
	"claim        ",
	"notify       ",
	"broadcast    ",
	"access       ",
	"set          ",
	"unset        ",
	"store        ",
	"asl_file     ",
	"asl_dir      ",
	"file         ",
	"forward      ",
	"control      ",
	"set (file)   ",
	"set (plist)  ",
	"set (profile)"
};

#define forever for(;;)
#define KEYMATCH(S,K) ((strncasecmp(S, K, strlen(K)) == 0))

#define STAMP_STYLE_INVALID -1
#define STAMP_STYLE_NULL 0
#define STAMP_STYLE_SEC 1
#define STAMP_STYLE_SEQ 2
#define STAMP_STYLE_UTC_OR_LCL 3

#if 0
asl_msg_t *
xpc_object_to_asl_msg(xpc_object_t xobj)
{
	__block asl_msg_t *out;

	if (xobj == NULL) return NULL;
	if (xpc_get_type(xobj) != XPC_TYPE_DICTIONARY) return NULL;

	out = asl_msg_new(ASL_TYPE_MSG);
	xpc_dictionary_apply(xobj, ^bool(const char *key, xpc_object_t xval) {
		char tmp[64];

		if (xpc_get_type(xval) == XPC_TYPE_NULL)
		{
			asl_msg_set_key_val_op(out, key, NULL, 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_BOOL)
		{
			if (xpc_bool_get_value(xval)) asl_msg_set_key_val_op(out, key, "1", 0);
			else asl_msg_set_key_val_op(out, key, "0", 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_INT64)
		{
			snprintf(tmp, sizeof(tmp), "%lld", xpc_int64_get_value(xval));
			asl_msg_set_key_val_op(out, key, tmp, 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_UINT64)
		{
			snprintf(tmp, sizeof(tmp), "%llu", xpc_uint64_get_value(xval));
			asl_msg_set_key_val_op(out, key, tmp, 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_DOUBLE)
		{
			snprintf(tmp, sizeof(tmp), "%f", xpc_double_get_value(xval));
			asl_msg_set_key_val_op(out, key, tmp, 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_DATE)
		{
			snprintf(tmp, sizeof(tmp), "%lld", xpc_date_get_value(xval));
			asl_msg_set_key_val_op(out, key, tmp, 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_DATA)
		{
			size_t len = xpc_data_get_length(xval);
			char *encoded = asl_core_encode_buffer(xpc_data_get_bytes_ptr(xval), len);
			asl_msg_set_key_val_op(out, key, encoded, 0);
			free(encoded);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_STRING)
		{
			asl_msg_set_key_val_op(out, key, xpc_string_get_string_ptr(xval), 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_UUID)
		{
			uuid_string_t us;
			uuid_unparse(xpc_uuid_get_bytes(xval), us);
			asl_msg_set_key_val_op(out, key, us, 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_FD)
		{
			/* XPC_TYPE_FD is not supported */
			asl_msg_set_key_val_op(out, key, "{XPC_TYPE_FD}", 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_SHMEM)
		{
			/* XPC_TYPE_SHMEM is not supported */
			asl_msg_set_key_val_op(out, key, "{XPC_TYPE_SHMEM}", 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_ARRAY)
		{
			/* XPC_TYPE_ARRAY is not supported */
			asl_msg_set_key_val_op(out, key, "{XPC_TYPE_ARRAY}", 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_DICTIONARY)
		{
			/* XPC_TYPE_DICTIONARY is not supported */
			asl_msg_set_key_val_op(out, key, "{XPC_TYPE_DICTIONARY}", 0);
		}
		else if (xpc_get_type(xval) == XPC_TYPE_ERROR)
		{
			/* XPC_TYPE_ERROR is not supported */
			asl_msg_set_key_val_op(out, key, "{XPC_TYPE_ERROR}", 0);
		}
		else
		{
			/* UNKNOWN TYPE */
			asl_msg_set_key_val_op(out, key, "{XPC_TYPE_???}", 0);
		}

		return true;
	});

	return out;
}

asl_msg_t *
configuration_profile_to_asl_msg(const char *ident)
{
	xpc_object_t xobj = configuration_profile_copy_property_list(ident);
	asl_msg_t *out = xpc_object_to_asl_msg(xobj);
	if (xobj != NULL) xpc_release(xobj);
	return out;
}
#endif

/* strdup + skip leading and trailing whitespace */
static char *
_strdup_clean(const char *s)
{
	char *out;
	const char *first, *last;
	size_t len;

	if (s == NULL) return NULL;

	first = s;
	while ((*first == ' ') || (*first == '\t')) first++;
	len = strlen(first);
	if (len == 0) return NULL;

	last = first + len - 1;
	while ((len > 0) && ((*last == ' ') || (*last == '\t')))
	{
		last--;
		len--;
	}

	if (len == 0) return NULL;

	out = malloc(len + 1);
	if (out == NULL) return NULL;

	memcpy(out, first, len);
	out[len] = '\0';
	return out;
}

static char **
_insert_string(char *s, char **l, uint32_t x)
{
	unsigned int i, len;

	if (s == NULL) return l;
	if (l == NULL) 
	{
		l = (char **)malloc(2 * sizeof(char *));
		if (l == NULL) return NULL;

		l[0] = strdup(s);
		if (l[0] == NULL)
		{
			free(l);
			return NULL;
		}

		l[1] = NULL;
		return l;
	}

	for (i = 0; l[i] != NULL; i++);

	 /* len includes the NULL at the end of the list */
	len = i + 1;

	l = (char **)reallocf(l, (len + 1) * sizeof(char *));
	if (l == NULL) return NULL;

	if ((x >= (len - 1)) || (x == IndexNull))
	{
		l[len - 1] = strdup(s);
		if (l[len - 1] == NULL)
		{
			free(l);
			return NULL;
		}

		l[len] = NULL;
		return l;
	}

	for (i = len; i > x; i--) l[i] = l[i - 1];
	l[x] = strdup(s);
	if (l[x] == NULL) return NULL;

	return l;
}

char **
explode(const char *s, const char *delim)
{
	char **l = NULL;
	const char *p;
	char *t, quote;
	int i, n;

	if (s == NULL) return NULL;

	quote = '\0';

	p = s;
	while (p[0] != '\0')
	{
		/* scan forward */
		for (i = 0; p[i] != '\0'; i++)
		{
			if (quote == '\0')
			{
				/* not inside a quoted string: check for delimiters and quotes */
				if (strchr(delim, p[i]) != NULL) break;
				else if (p[i] == '\'') quote = p[i];
				else if (p[i] == '"') quote = p[i];
			}
			else
			{
				/* inside a quoted string - look for matching quote */
				if (p[i] == quote) quote = '\0';
			}
		}

		n = i;
		t = malloc(n + 1);
		if (t == NULL) return NULL;

		for (i = 0; i < n; i++) t[i] = p[i];
		t[n] = '\0';
		l = _insert_string(t, l, IndexNull);
		free(t);
		t = NULL;
		if (p[i] == '\0') return l;
		if (p[i + 1] == '\0') l = _insert_string("", l, IndexNull);
		p = p + i + 1;
	}

	return l;
}

void
free_string_list(char **l)
{
	int i;

	if (l == NULL) return;
	for (i = 0; l[i] != NULL; i++) free(l[i]);
	free(l);
}

char *
get_line_from_file(FILE *f)
{
	char *s, *out;
	size_t len;

	out = fgetln(f, &len);
	if (out == NULL) return NULL;
	if (len == 0) return NULL;

	s = malloc(len + 1);
	if (s == NULL) return NULL;

	memcpy(s, out, len);

	if (s[len - 1] != '\n') len++;
	s[len - 1] = '\0';
	return s;
}

char *
next_word_from_string(char **s)
{
	char *a, *p, *e, *out, s0;
	int quote1, quote2, len;

	if (s == NULL) return NULL;
	if (*s == NULL) return NULL;

	s0 = **s;

	quote1 = 0;
	quote2 = 0;

	p = *s;

	/* allow whole word to be contained in quotes */
	if (*p == '\'')
	{
		quote1 = 1;
		p++;
	}

	if (*p == '"')
	{
		quote2 = 1;
		p++;
	}

	a = p;
	e = p;

	while (*p != '\0')
	{
		if (*p == '\\')
		{
			p++;
			e = p;

			if (*p == '\0')
			{
				p--;
				break;
			}

			p++;
			e = p;
			continue;
		}

		if (*p == '\'')
		{
			if (quote1 == 0) quote1 = 1;
			else quote1 = 0;
		}

		if (*p == '"')
		{
			if (quote2 == 0) quote2 = 1;
			else quote2 = 0;
		}

		if (((*p == ' ') || (*p == '\t')) && (quote1 == 0) && (quote2 == 0))
		{
			e = p + 1;
			break;
		}

		p++;
		e = p;
	}

	*s = e;

	len = p - a;

	/* check for quoted string */
	if (((s0 == '\'') || (s0 == '"')) && (s0 == a[len-1])) len--;

	if (len == 0) return NULL;

	out = malloc(len + 1);
	if (out == NULL) return NULL;

	memcpy(out, a, len);
	out[len] = '\0';
	return out;
}

static asl_out_dst_data_t *
_asl_out_dest_for_path(asl_out_module_t *m, const char *path)
{
	if (m == NULL) return NULL;
	if (path == NULL) return NULL;

	while (m != NULL)
	{
		asl_out_rule_t *r = m->ruleset;
		while (r != NULL)
		{
			if ((r->action == ACTION_OUT_DEST) && (r->dst != NULL) && (r->dst->path != NULL) && (!strcmp(r->dst->path, path))) return r->dst;
			r = r->next;
		}

		m = m->next;
	}

	return NULL;
}

/*
 * Create a directory path.
 *
 * mlist provides owner, group, and access mode, which are required for "non-standard"
 * directories.  Directories for standard paths (/var/log or /Library/Logs) default
 * to root/admin/0755 if there is no mlist rule for them.
 */
static int
_asl_common_make_dir_path(asl_out_module_t *mlist, uint32_t flags, const char *path)
{
	int i;
	char **path_parts;
	asl_string_t *processed_path;
	mode_t mode;

	if (path == NULL) return 0;

	path_parts = explode(path, "/");
	if (path_parts == NULL) return 0;

	processed_path = asl_string_new(ASL_ENCODE_NONE);

	i = 0;
	if (path[0] == '/') i = 1;

	for (; path_parts[i] != NULL; i++)
	{
		struct stat sb;
		int status;
		mode_t mask;
		asl_out_dst_data_t *dst;
		char *tmp;

		asl_string_append_char_no_encoding(processed_path, '/');
		asl_string_append_no_encoding(processed_path, path_parts[i]);
		tmp = asl_string_bytes(processed_path);

		memset(&sb, 0, sizeof(struct stat));
		status = lstat(tmp, &sb);
		if ((status == 0) && S_ISLNK(sb.st_mode))
		{
			char real[MAXPATHLEN];
			if (realpath(tmp, real) == NULL)
			{
				asl_string_release(processed_path);
				free_string_list(path_parts);
				return -1;
			}

			memset(&sb, 0, sizeof(struct stat));
			status = stat(real, &sb);
		}

		if (status == 0)
		{
			if (!S_ISDIR(sb.st_mode))
			{
				/* path component is not a directory! */
				asl_string_release(processed_path);
				free_string_list(path_parts);
				return -1;
			}

			/* exists and is a directory or a link to a directory */
			continue;
		}
		else if (errno != ENOENT)
		{
			/* unexpected status from stat() */
			asl_string_release(processed_path);
			free_string_list(path_parts);
			return -1;
		}

		dst = _asl_out_dest_for_path(mlist, tmp);
		if ((dst == NULL) && (flags & MODULE_FLAG_NONSTD_DIR))
		{
			/* no rule to create a non-standard path component! */
			asl_string_release(processed_path);
			free_string_list(path_parts);
			return -1;
		}

		mode = 0755;
		if (dst != NULL)
		{
			mode = dst->mode;
			if (mode == 010000) mode = 0755;
		}

		mask = umask(0);
		status = mkdir(tmp, mode);
		umask(mask);

#if !TARGET_IPHONE_SIMULATOR
		uid_t u = 0;
		gid_t g = 80;

		if (dst != NULL)
		{
			if (dst->nuid > 0) u = dst->uid[0];
			if (dst->ngid > 0) g = dst->gid[0];
		}

		chown(tmp, u, g);
#endif
	}

	asl_string_release(processed_path);
	free_string_list(path_parts);

	return 0;
}

int
asl_out_mkpath(asl_out_module_t *mlist, asl_out_rule_t *r)
{
	char tmp[MAXPATHLEN], *p;
	struct stat sb;
	int status;

	if (r == NULL) return -1;
	if (r->dst == NULL) return -1;
	if (r->dst->path == NULL) return -1;

	snprintf(tmp, sizeof(tmp), "%s", r->dst->path);

	if (r->action != ACTION_ASL_DIR)
	{
		p = strrchr(tmp, '/');
		if (p == NULL) return -1;
		*p = '\0';
	}

	memset(&sb, 0, sizeof(struct stat));
	status = stat(tmp, &sb);
	if (status == 0)
	{
		if (S_ISDIR(sb.st_mode)) return 0;
		return -1;
	}

	if (errno == ENOENT)
	{
		uint32_t dirflag = r->dst->flags & MODULE_FLAG_NONSTD_DIR;
		status = _asl_common_make_dir_path(mlist, dirflag, tmp);
		return status;
	}

	return -1;
}

void
asl_make_timestamp(time_t stamp, uint32_t flags, char *buf, size_t len)
{
	struct tm t;
	uint32_t h, m, s;

	if (buf == NULL) return;

	if (flags & MODULE_FLAG_STYLE_UTC)
	{
		memset(&t, 0, sizeof(t));
		gmtime_r(&stamp, &t);
		snprintf(buf, len, "%d-%02d-%02dT%02d:%02d:%02dZ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	}
	else if (flags & MODULE_FLAG_STYLE_UTC_B)
	{
		memset(&t, 0, sizeof(t));
		gmtime_r(&stamp, &t);
		snprintf(buf, len, "%d%02d%02dT%02d%02d%02dZ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	}
	else if (flags & MODULE_FLAG_STYLE_LCL)
	{
		bool neg = false;
		memset(&t, 0, sizeof(t));
		localtime_r(&stamp, &t);

		if ((neg = (t.tm_gmtoff < 0))) t.tm_gmtoff *= -1;

		s = t.tm_gmtoff;
		h = s / 3600;
		s %= 3600;
		m = s / 60;
		s %= 60;

		if (s > 0) snprintf(buf, len, "%d-%02d-%02dT%02d:%02d:%02d%c%u:%02u:%02u", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, neg ? '-' : '+', h, m, s);
		else if (m > 0) snprintf(buf, len, "%d-%02d-%02dT%02d:%02d:%02d%c%u:%02u", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, neg ? '-' : '+', h, m);
		else snprintf(buf, len, "%d-%02d-%02dT%02d:%02d:%02d%c%u", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, neg ? '-' : '+', h);
	}
	else if (flags & MODULE_FLAG_STYLE_LCL_B)
	{
		bool neg = false;
		memset(&t, 0, sizeof(t));
		localtime_r(&stamp, &t);

		if ((neg = (t.tm_gmtoff < 0))) t.tm_gmtoff *= -1;

		s = t.tm_gmtoff;
		h = s / 3600;
		s %= 3600;
		m = s / 60;
		s %= 60;

		if (s > 0) snprintf(buf, len, "%d%02d%02dT%02d%02d%02d%c%02u%02u%02u", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, neg ? '-' : '+', h, m, s);
		else if (m > 0) snprintf(buf, len, "%d%02d%02dT%02d%02d%02d%c%02u%02u", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, neg ? '-' : '+', h, m);
		else snprintf(buf, len, "%d%02d%02dT%02d%02d%02d%c%02u", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, neg ? '-' : '+', h);
	}
	else
	{
		snprintf(buf, len, "%c%lu", STYLE_SEC_PREFIX_CHAR, stamp);
	}
}

void
asl_make_dst_filename(asl_out_dst_data_t *dst, char *buf, size_t len)
{
	if (dst == NULL) return;
	if (buf == NULL) return;

	if (dst->flags & (MODULE_FLAG_BASESTAMP | MODULE_FLAG_TYPE_ASL_DIR))
	{
		char tstamp[32];
		const char *name = dst->path;

		if (dst->flags & MODULE_FLAG_TYPE_ASL_DIR) name = dst->fname;

		if (dst->stamp == 0) dst->stamp = time(NULL);
		asl_make_timestamp(dst->stamp, dst->flags, tstamp, sizeof(tstamp));
		snprintf(buf, len, "%s.%s", name, tstamp);
	}
	else
	{
		snprintf(buf, len, "%s", dst->path);
	}
}

int
asl_check_option(asl_msg_t *msg, const char *opt)
{
	const char *p;
	uint32_t len;

	if (msg == NULL) return 0;
	if (opt == NULL) return 0;

	len = strlen(opt);
	if (len == 0) return 0;

	p = asl_msg_get_val_for_key(msg, ASL_KEY_OPTION);
	if (p == NULL) return 0;

	while (*p != '\0')
	{
		while ((*p == ' ') || (*p == '\t') || (*p == ',')) p++;
		if (*p == '\0') return 0;

		if (strncasecmp(p, opt, len) == 0)
		{
			p += len;
			if ((*p == ' ') || (*p == '\t') || (*p == ',') || (*p == '\0')) return 1;
		}

		while ((*p != ' ') && (*p != '\t') && (*p != ',') && (*p != '\0')) p++;
	}

	return 0;
}

void
asl_out_dst_data_release(asl_out_dst_data_t *dst)
{
	if (dst == NULL) return;

	if (dst->refcount > 0) dst->refcount--;
	if (dst->refcount > 0) return;

	free(dst->path);
	free(dst->fname);
	free(dst->rotate_dir);
	free(dst->fmt);
#if !TARGET_IPHONE_SIMULATOR
	free(dst->uid);
	free(dst->gid);
#endif
	free(dst);
}

asl_out_dst_data_t *
asl_out_dst_data_retain(asl_out_dst_data_t *dst)
{
	if (dst == NULL) return NULL;
	dst->refcount++;
	return dst;
}

/* set owner, group, mode, and acls for a file */
int
asl_out_dst_set_access(int fd, asl_out_dst_data_t *dst)
{
#if !TARGET_IPHONE_SIMULATOR
	uid_t fuid = 0;
	gid_t fgid = 80;
#if !TARGET_OS_EMBEDDED
	int status;
	acl_t acl;
	uuid_t uuid;
	acl_entry_t entry;
	acl_permset_t perms;
	uint32_t i;
#endif
#endif

	if (dst == NULL) return -1;
	if (fd < 0) return -1;

#if TARGET_IPHONE_SIMULATOR
	return fd;
#else

	if (dst->nuid > 0) fuid = dst->uid[0];
	if (dst->ngid > 0) fgid = dst->gid[0];

	fchown(fd, fuid, fgid);

#if TARGET_OS_EMBEDDED
	return fd;
#else
	acl = acl_init(1);

	for (i = 0; i < dst->ngid; i++)
	{
		if (dst->gid[i] == (gid_t)-2) continue;

		/*
		 * Don't bother setting group access if this is
		 * file's group and the file is group-readable.
		 */
		if ((dst->gid[i] == fgid) && (dst->mode & 00040)) continue;

#if 0
		status = mbr_gid_to_uuid(dst->gid[i], uuid);
		if (status != 0)
		{
			dst->gid[i] = -2;
			continue;
		}
#endif

		status = acl_create_entry_np(&acl, &entry, ACL_FIRST_ENTRY);
		if (status != 0) goto asl_file_create_return;

#if 0
		status = acl_set_tag_type(entry, ACL_EXTENDED_ALLOW);
		if (status != 0) goto asl_file_create_return;
#endif

		status = acl_set_qualifier(entry, &uuid);
		if (status != 0) goto asl_file_create_return;

		status = acl_get_permset(entry, &perms);
		if (status != 0) goto asl_file_create_return;

		status = acl_add_perm(perms, ACL_READ_DATA);
		if (status != 0) goto asl_file_create_return;
	}

	for (i = 0; i < dst->nuid; i++)
	{
		if (dst->uid[i] == (gid_t)-2) continue;

		/*
		 * Don't bother setting user access if this is
		 * file's owner and the file is owner-readable.
		 */
		if ((dst->uid[i] == fuid) && (dst->mode & 00400)) continue;

#if 0
		status = mbr_uid_to_uuid(dst->uid[i], uuid);
		if (status != 0)
		{
			dst->uid[i] = -2;
			continue;
		}
#endif

		status = acl_create_entry_np(&acl, &entry, ACL_FIRST_ENTRY);
		if (status != 0) goto asl_file_create_return;

#if 0
		status = acl_set_tag_type(entry, ACL_EXTENDED_ALLOW);
		if (status != 0) goto asl_file_create_return;
#endif

		status = acl_set_qualifier(entry, &uuid);
		if (status != 0) goto asl_file_create_return;

		status = acl_get_permset(entry, &perms);
		if (status != 0) goto asl_file_create_return;

		status = acl_add_perm(perms, ACL_READ_DATA);
		if (status != 0) goto asl_file_create_return;
	}

	status = acl_set_fd(fd, acl);
	if (status != 0)
	{
		close(fd);
		fd = -1;
	}

asl_file_create_return:

	acl_free(acl);
	return fd;
#endif /* !TARGET_OS_EMBEDDED */
#endif /* !TARGET_IPHONE_SIMULATOR */
}

/* create a file with acls */
int
asl_out_dst_file_create_open(asl_out_dst_data_t *dst, char **pathp)
{
	int fd, status;
	struct stat sb;
	char outpath[MAXPATHLEN];

	if (dst == NULL) return -1;
	if (dst->path == NULL) return -1;

	asl_make_dst_filename(dst, outpath, sizeof(outpath));
	if (dst->fname != NULL) free(dst->fname);

	dst->fname = strdup(outpath);
	if (dst->fname == NULL) return -1;

	if (pathp != NULL) *pathp = strdup(outpath);

	memset(&sb, 0, sizeof(struct stat));
	status = stat(outpath, &sb);
	if (status == 0)
	{
		/* must be a regular file */
		if (!S_ISREG(sb.st_mode)) return -1;

		/* file exists */
		fd = open(outpath, O_RDWR | O_APPEND | O_EXCL, 0);

		if (dst->stamp == 0) dst->stamp = sb.st_birthtimespec.tv_sec;
		if (dst->stamp == 0) dst->stamp = sb.st_mtimespec.tv_sec;
		dst->size = sb.st_size;

		return fd;
	}
	else if (errno != ENOENT)
	{
		/* stat error other than non-existant file */
		return -1;
	}

	fd = open(outpath, O_RDWR | O_CREAT | O_EXCL, (dst->mode & 00666));
	if (fd < 0) return -1;

	dst->stamp = time(NULL);

	fd = asl_out_dst_set_access(fd, dst);
	if (fd < 0) unlink(outpath);

	return fd;
}

void
asl_out_module_free(asl_out_module_t *m)
{
	asl_out_rule_t *r, *n;
	asl_out_module_t *x;

	while (m != NULL)
	{
		x = m->next;

		/* free name */
		free(m->name);

		/* free ruleset */
		r = m->ruleset;
		while (r != NULL)
		{
			n = r->next;
			if (r->dst != NULL) asl_out_dst_data_release(r->dst);

			if (r->query != NULL) asl_msg_release(r->query);
			free(r->options);
			free(r);
			r = n;
		}

		free(m);
		m = x;
	}
}

asl_out_module_t *
asl_out_module_new(const char *name)
{
	asl_out_module_t *out = (asl_out_module_t *)calloc(1, sizeof(asl_out_module_t));

	if (out == NULL) return NULL;
	if (name == NULL) return NULL;

	out->name = strdup(name);
	if (out->name == NULL)
	{
		free(out);
		return NULL;
	}

	out->flags = MODULE_FLAG_ENABLED;

	return out;
}

/* Skip over query */
static char *
_asl_out_module_find_action(char *s)
{
	char *p;

	p = s;
	if (p == NULL) return NULL;

	/* Skip command character (?, Q, *, or =) */
	p++;

	forever
	{
		/* Find next [ */
		while ((*p == ' ') || (*p == '\t')) p++;

		if (*p == '\0') return NULL;
		if (*p != '[') return p;

		/* skip to closing ] */
		while (*p != ']')
		{
			p++;
			if (*p == '\\')
			{
				p++;
				if (*p == ']') p++;
			}
		}

		if (*p == ']') p++;
	}

	/* skip whitespace */
	while ((*p == ' ') || (*p == '\t')) p++;

	return NULL;
}

/*
 * Parse parameter setting line
 *
 * = param options
 *		evaluated once when module is initialized
 *
 * = [query] param options
 *		evaluated for each message, param set if message matches query
 *
 * = param [File path]
 *		evaluated once when module is initialized
 *		evaluated when change notification received for path
 *
 * = param [Plist path] ...
 *		evaluated once when module is initialized
 *		evaluated when change notification received for path
 *
 * = param [Profile name] ...
 *		evaluated once when module is initialized
 *		evaluated when change notification received for profile
 */
static asl_out_rule_t *
_asl_out_module_parse_set_param(asl_out_module_t *m, char *s)
{
	char *act, *p, *q;
	asl_out_rule_t *out, *rule;

	if (m == NULL) return NULL;

	out = (asl_out_rule_t *)calloc(1, sizeof(asl_out_rule_t));
	if (out == NULL) return NULL;

	q = s + 1;
	while ((*q == ' ') || (*q == '\'')) q++;
	out->action = ACTION_SET_PARAM;

	if (*q == '[')
	{
		/* = [query] param options */
		act = _asl_out_module_find_action(s);
		if (act == NULL)
		{
			free(out);
			return NULL;
		}

		out->options = _strdup_clean(act);

		p = act - 1;
		if (*p == ']') p = act;
		*p = '\0';

		*s = 'Q';
		out->query = asl_msg_from_string(s);
		if (out->query == NULL)
		{
			free(out->options);
			free(out);
			return NULL;
		}
	}
	else
	{
		/* = param ... */
		p = strchr(s, '[');
		if (p == NULL)
		{
			/* = param options */
			out->options = _strdup_clean(q);
		}
		else
		{
			/* = param [query] */
			if ((!strncmp(p, "[File ", 6)) || (!strncmp(p, "[File\t", 6))) out->action = ACTION_SET_FILE;
			else if ((!strncmp(p, "[Plist ", 7)) || (!strncmp(p, "[Plist\t", 7))) out->action = ACTION_SET_PLIST;
			else if ((!strncmp(p, "[Profile ", 9)) || (!strncmp(p, "[Profile\t", 9))) out->action = ACTION_SET_PROF;

			p--;
			*p = '\0';
			out->options = _strdup_clean(q);

			*p = ' ';
			p--;
			*p = 'Q';
			out->query = asl_msg_from_string(p);
			if (out->query == NULL)
			{
				free(out->options);
				free(out);
				return NULL;
			}
		}
	}

	if (m->ruleset == NULL) m->ruleset = out;
	else
	{
		for (rule = m->ruleset; rule->next != NULL; rule = rule->next);
		rule->next = out;
	}

	return out;
}

#if !TARGET_IPHONE_SIMULATOR
static void
_dst_add_uid(asl_out_dst_data_t *dst, char *s)
{
	unsigned int i;
	uid_t uid;

	if (dst == NULL) return;
	if (s == NULL) return;

	uid = atoi(s);

	for (i = 0 ; i < dst->nuid; i++)
	{
		if (dst->uid[i] == uid) return;
	}

	dst->uid = reallocf(dst->uid, (dst->nuid + 1) * sizeof(uid_t));
	if (dst->uid == NULL)
	{
		dst->nuid = 0;
		return;
	}

	dst->uid[dst->nuid++] = uid;
}

static void
_dst_add_gid(asl_out_dst_data_t *dst, char *s)
{
	int i;
	gid_t gid;

	if (dst == NULL) return;
	if (s == NULL) return;

	gid = atoi(s);

	for (i = 0 ; i < dst->ngid; i++)
	{
		if (dst->gid[i] == gid) return;
	}

	dst->gid = reallocf(dst->gid, (dst->ngid + 1) * sizeof(gid_t));
	if (dst->gid == NULL)
	{
		dst->ngid = 0;
		return;
	}

	dst->gid[dst->ngid++] = gid;
}
#endif /* !TARGET_IPHONE_SIMULATOR */

static char *
_dst_format_string(char *s)
{
	char *fmt;
	size_t i, len, n;

	if (s == NULL) return NULL;

	len = strlen(s);

	/* format string can be enclosed by quotes */
	if ((len >= 2) && ((s[0] == '\'') || (s[0] == '"')) && (s[len-1] == s[0]))
	{
		s++;
		len -= 2;
	}

	n = 0;
	for (i = 0; i < len; i++) if (s[i] == '\\') n++;

	fmt = malloc(1 + len - n);
	if (fmt == NULL) return NULL;

	for (i = 0, n = 0; i < len; i++) if (s[i] != '\\') fmt[n++] = s[i];
	fmt[n] = '\0';
	return fmt;
}

size_t
asl_str_to_size(char *s)
{
	size_t len, n, max;
	char x;

	if (s == NULL) return 0;

	len = strlen(s);
	if (len == 0) return 0;

	n = 1;
	x = s[len - 1];
	if (x > 90) x -= 32;
	if (x == 'K') n = 1ll << 10;
	else if (x == 'M') n = 1ll << 20;
	else if (x == 'G') n = 1ll << 30;

	max = atoll(s) * n;
	return max;
}

static bool
_dst_path_match(const char *newpath, const char *existingpath)
{
	if (newpath == NULL) return (existingpath == NULL);
	if (existingpath == NULL) return false;
	if (newpath[0] == '/') return (strcmp(newpath, existingpath) == 0);

	const char *trailing = strrchr(existingpath, '/');
	if (trailing == NULL) return (strcmp(newpath, existingpath) == 0);
	trailing++;
	return (strcmp(newpath, trailing) == 0);
}

static asl_out_dst_data_t *
_asl_out_module_parse_dst(asl_out_module_t *m, char *s, mode_t def_mode)
{
	asl_out_rule_t *out, *rule;
	asl_out_dst_data_t *dst;
	char *p, *opts, *path;
	char **path_parts;
	int has_dotdot, recursion_limit;
	uint32_t i, flags = 0;

	if (m == NULL) return NULL;
	if (s == NULL) return NULL;

	/* skip whitespace */
	while ((*s == ' ') || (*s == '\t')) s++;

	opts = s;
	path = next_word_from_string(&opts);
	if (path == NULL) return NULL;

	/*
	 * Check path for ".." component (not permitted).
	 * Also substitute environment variables.
	 */
	has_dotdot = 0;
	path_parts = explode(path, "/");
	asl_string_t *processed_path = asl_string_new(ASL_ENCODE_NONE);
	recursion_limit = 5;

	while ((recursion_limit > 0) && (path_parts != NULL) && (processed_path != NULL))
	{
		uint32_t i;
		int did_sub = 0;

		for (i = 0; path_parts[i] != NULL; i++)
		{
			if (!strncmp(path_parts[i], "$ENV(", 5))
			{
				char *p = strchr(path_parts[i], ')');
				if (p != NULL) *p = '\0';
				char *env_val = getenv(path_parts[i] + 5);
				if (env_val != NULL)
				{
					did_sub = 1;

					if (env_val[0] != '/') asl_string_append_char_no_encoding(processed_path, '/');
					asl_string_append_no_encoding(processed_path, env_val);
				}
			}
			else
			{
				if (i == 0)
				{
					if (path_parts[0][0] != '\0') asl_string_append_no_encoding(processed_path, path_parts[i]);
				}
				else
				{
					asl_string_append_char_no_encoding(processed_path, '/');
					asl_string_append_no_encoding(processed_path, path_parts[i]);
				}
			}

			if ((has_dotdot == 0) && (!strcmp(path_parts[i], ".."))) has_dotdot = 1;
		}

		free_string_list(path_parts);
		path_parts = NULL;

		if ((did_sub == 1) && (has_dotdot == 0))
		{
			/* substitution might have added a ".." so check the new path */
			free(path);
			path = asl_string_release_return_bytes(processed_path);
			processed_path = asl_string_new(ASL_ENCODE_NONE);
			path_parts = explode(path, "/");
			recursion_limit--;
		}
	}

	free(path);
	free_string_list(path_parts);
	path_parts = NULL;

	if ((has_dotdot != 0) || (recursion_limit == 0))
	{
		asl_string_release(processed_path);
		return NULL;
	}

	path = asl_string_release_return_bytes(processed_path);

	/* check if there's already a dst for this path */
	for (rule = m->ruleset; rule != NULL; rule = rule->next)
	{
		if (rule->action != ACTION_OUT_DEST) continue;

		dst = rule->dst;
		if (dst == NULL) continue;

		if (_dst_path_match(path, dst->path))
		{
			free(path);
			return dst;
		}
	}

	flags |= MODULE_FLAG_NONSTD_DIR;

	if (path[0] != '/')
	{
		char *t = path;
		const char *log_root = "/var/log";

#if TARGET_IPHONE_SIMULATOR
		log_root = getenv("SIMULATOR_LOG_ROOT");
		if (log_root == NULL) log_root = "/tmp/log";
#endif

		if (!strcmp(m->name, ASL_MODULE_NAME))
		{
			asprintf(&path, "%s/%s", log_root, t);
		}
		else
		{
			asprintf(&path, "%s/module/%s/%s", log_root, m->name, t);
		}

		free(t);
		flags &= ~MODULE_FLAG_NONSTD_DIR;
	}
	else
	{
		/*
		 * Standard log directories get marked so that syslogd
		 * will create them without explicit rules.
		 */
		if (!strncmp(path, PATH_VAR_LOG, PATH_VAR_LOG_LEN)) flags &= ~MODULE_FLAG_NONSTD_DIR;
		else if (!strncmp(path, PATH_LIBRARY_LOGS, PATH_LIBRARY_LOGS_LEN)) flags &= ~MODULE_FLAG_NONSTD_DIR;
	}

	out = (asl_out_rule_t *)calloc(1, sizeof(asl_out_rule_t));
	dst = (asl_out_dst_data_t *)calloc(1, sizeof(asl_out_dst_data_t));
	if ((out == NULL) || (dst == NULL))
	{
		free(path);
		free(out);
		free(dst);
		return NULL;
	}

	dst->refcount = 1;
	dst->path = path;
	dst->mode = def_mode;
	dst->ttl[LEVEL_ALL] = DEFAULT_TTL;
	dst->flags = flags | MODULE_FLAG_COALESCE;

	while (NULL != (p = next_word_from_string(&opts)))
	{
		if (KEYMATCH(p, "mode=")) dst->mode = strtol(p+5, NULL, 0);
#if !TARGET_IPHONE_SIMULATOR
		else if (KEYMATCH(p, "uid=")) _dst_add_uid(dst, p+4);
		else if (KEYMATCH(p, "gid=")) _dst_add_gid(dst, p+4);
#endif
		else if (KEYMATCH(p, "fmt=")) dst->fmt = _dst_format_string(p+4);
		else if (KEYMATCH(p, "format=")) dst->fmt = _dst_format_string(p+7);
		else if (KEYMATCH(p, "dest=")) dst->rotate_dir = _strdup_clean(p+5);
		else if (KEYMATCH(p, "dst=")) dst->rotate_dir = _strdup_clean(p+4);
		else if (KEYMATCH(p, "coalesce="))
		{
			if (KEYMATCH(p+9, "0")) dst->flags &= ~MODULE_FLAG_COALESCE;
			else if (KEYMATCH(p+9, "off")) dst->flags &= ~MODULE_FLAG_COALESCE;
			else if (KEYMATCH(p+9, "false")) dst->flags &= ~MODULE_FLAG_COALESCE;
		}
		else if (KEYMATCH(p, "compress")) dst->flags |= MODULE_FLAG_COMPRESS;
		else if (KEYMATCH(p, "extern")) dst->flags |= MODULE_FLAG_EXTERNAL;
		else if (KEYMATCH(p, "truncate")) dst->flags |= MODULE_FLAG_TRUNCATE;
		else if (KEYMATCH(p, "dir")) dst->flags |= MODULE_FLAG_TYPE_ASL_DIR;
		else if (KEYMATCH(p, "soft")) dst->flags |= MODULE_FLAG_SOFT_WRITE;
		else if (KEYMATCH(p, "file_max=")) dst->file_max = asl_str_to_size(p+9);
		else if (KEYMATCH(p, "all_max=")) dst->all_max = asl_str_to_size(p+8);
		else if (KEYMATCH(p, "style=") || KEYMATCH(p, "rotate="))
		{
			const char *x = p + 6;

			if (KEYMATCH(p, "rotate=")) x++;

			dst->flags |= MODULE_FLAG_ROTATE;

			if (KEYMATCH(x, "sec") || KEYMATCH(x, "seconds"))
			{
				dst->flags |= MODULE_FLAG_STYLE_SEC;
			}
			else if (KEYMATCH(x, "utc") || KEYMATCH(x, "date") || KEYMATCH(x, "zulu"))
			{
				const char *dash = strchr(x, '-');
				if ((dash != NULL) && (*(dash + 1) == 'b')) dst->flags |= MODULE_FLAG_STYLE_UTC_B;
				else dst->flags |= MODULE_FLAG_STYLE_UTC;
			}
			else if (KEYMATCH(x, "local") || KEYMATCH(x, "lcl"))
			{
				const char *dash = strchr(x, '-');
				if ((dash != NULL) && (*(dash + 1) == 'b')) dst->flags |= MODULE_FLAG_STYLE_LCL_B;
				else dst->flags |= MODULE_FLAG_STYLE_LCL;
			}
			else if (KEYMATCH(x, "#") || KEYMATCH(x, "seq") || KEYMATCH(x, "sequence"))
			{
				dst->flags |= MODULE_FLAG_STYLE_SEQ;
			}
			else
			{
				dst->flags |= MODULE_FLAG_STYLE_SEC;
			}
		}
		else if (KEYMATCH(p, "rotate")) dst->flags |= MODULE_FLAG_ROTATE;
		else if (KEYMATCH(p, "crashlog"))
		{
			/* crashlog implies rotation */
			dst->flags |= MODULE_FLAG_ROTATE;
			dst->flags |= MODULE_FLAG_CRASHLOG;
			dst->flags |= MODULE_FLAG_BASESTAMP;
			dst->flags &= ~MODULE_FLAG_COALESCE;
		}
		else if (KEYMATCH(p, "basestamp"))
		{
			dst->flags |= MODULE_FLAG_BASESTAMP;
		}
		else if (KEYMATCH(p, "ttl"))
		{
			char *q = p + 3;
			if (*q == '=')
			{
				dst->ttl[LEVEL_ALL] = strtol(p+4, NULL, 0);
			}
			else if ((*q >= '0') && (*q <= '7') && (*(q+1) == '='))
			{
				uint32_t x = *q - '0';
				dst->ttl[x] = strtol(p+5, NULL, 0);
			}
		}

		free(p);
		p = NULL;
	}

#if TARGET_OS_EMBEDDED
	/* check for crashreporter files */
	if ((KEYMATCH(dst->path, _PATH_CRASHREPORTER)) || (KEYMATCH(dst->path, _PATH_CRASHREPORTER_MOBILE)))
	{
		dst->flags |= MODULE_FLAG_ROTATE;
		dst->flags |= MODULE_FLAG_CRASHLOG;
		dst->flags |= MODULE_FLAG_BASESTAMP;
		dst->flags &= ~MODULE_FLAG_COALESCE;
	}
#endif

	/* ttl[LEVEL_ALL] must be max of all level-specific ttls */
	for (i = 0; i <= 7; i++) if (dst->ttl[i] > dst->ttl[LEVEL_ALL]) dst->ttl[LEVEL_ALL] = dst->ttl[i];

	/* default text file format is "std" */
	if (dst->fmt == NULL) dst->fmt = strdup("std");

	/* duplicate compression is only possible for std and bsd formats */
	if (strcmp(dst->fmt, "std") && strcmp(dst->fmt, "bsd")) dst->flags &= ~MODULE_FLAG_COALESCE;

	/* note if format is one of std, bsd, or msg */
	if (!strcmp(dst->fmt, "std") || !strcmp(dst->fmt, "bsd") || !strcmp(dst->fmt, "msg")) dst->flags |= MODULE_FLAG_STD_BSD_MSG;

	/* MODULE_FLAG_STYLE_SEQ can not be used with MODULE_FLAG_BASESTAMP */
	if ((dst->flags & MODULE_FLAG_BASESTAMP) && (dst->flags & MODULE_FLAG_STYLE_SEQ))
	{
		dst->flags &= ~MODULE_FLAG_STYLE_SEQ;
		dst->flags |= MODULE_FLAG_STYLE_SEC;
	}

	/* set time format for raw output */
	if (!strcmp(dst->fmt, "raw")) dst->tfmt = "sec";

	/* check for ASL_PLACE_DATABASE_DEFAULT */
	if (!strcmp(dst->path, ASL_PLACE_DATABASE_DEFAULT))
	{
		dst->flags = MODULE_FLAG_TYPE_ASL_DIR;
	}

	out->action = ACTION_OUT_DEST;
	out->dst = dst;

	/* dst rules go first */
	out->next = m->ruleset;
	m->ruleset = out;

	return dst;
}

static asl_out_rule_t *
_asl_out_module_parse_query_action(asl_out_module_t *m, char *s)
{
	char *act, *p;
	asl_out_rule_t *out, *rule;

	if (m == NULL) return NULL;

	out = (asl_out_rule_t *)calloc(1, sizeof(asl_out_rule_t));
	if (out == NULL) return NULL;

	act = _asl_out_module_find_action(s);
	if (act == NULL) return NULL;

	/* find whitespace delimiter */
	p = strchr(act, ' ');
	if (p == NULL) p = strchr(act, '\t');
	if (p != NULL) *p = '\0';

	if (!strcasecmp(act, "ignore"))               out->action = ACTION_IGNORE;
	else if (!strcasecmp(act, "skip"))            out->action = ACTION_SKIP;
	else if (!strcasecmp(act, "claim"))           out->action = ACTION_CLAIM;
	else if (!strcasecmp(act, "notify"))          out->action = ACTION_NOTIFY;
	else if (!strcasecmp(act, "file"))            out->action = ACTION_FILE;
	else if (!strcasecmp(act, "asl_file"))        out->action = ACTION_ASL_FILE;
	else if (!strcasecmp(act, "directory"))       out->action = ACTION_ASL_DIR;
	else if (!strcasecmp(act, "dir"))             out->action = ACTION_ASL_DIR;
	else if (!strcasecmp(act, "asl_directory"))   out->action = ACTION_ASL_DIR;
	else if (!strcasecmp(act, "asl_dir"))         out->action = ACTION_ASL_DIR;
	else if (!strcasecmp(act, "store_dir"))       out->action = ACTION_ASL_DIR;
	else if (!strcasecmp(act, "store_directory")) out->action = ACTION_ASL_DIR;
	else if (!strcasecmp(act, "control"))		  out->action = ACTION_CONTROL;
	else if (!strcasecmp(act, "save"))            out->action = ACTION_ASL_STORE;
	else if (!strcasecmp(act, "store"))           out->action = ACTION_ASL_STORE;
	else if (!strcasecmp(act, "access"))          out->action = ACTION_ACCESS;
	else if (!strcasecmp(act, "set"))             out->action = ACTION_SET_KEY;
	else if (!strcasecmp(act, "unset"))           out->action = ACTION_UNSET_KEY;
	else if	(!strcmp(m->name, ASL_MODULE_NAME))
	{
		/* actions only allowed in com.apple.asl */
		if (!strcasecmp(act, "broadcast"))   out->action = ACTION_BROADCAST;
		else if (!strcasecmp(act, "forward"))     out->action = ACTION_FORWARD;
	}

	if (out->action == ACTION_NONE)
	{
		free(out);
		return NULL;
	}

	/* options follow delimited (now zero) */
	if (p != NULL)
	{
		/* skip whitespace */
		while ((*p == ' ') || (*p == '\t')) p++;

		out->options = _strdup_clean(p+1);

		if (out->options == NULL)
		{
			free(out);
			return NULL;
		}
	}

	p = act - 1;

	*p = '\0';

	if (*s== '*')
	{
		out->query = asl_msg_new(ASL_TYPE_QUERY);
	}
	else
	{
		*s = 'Q';
		out->query = asl_msg_from_string(s);
	}

	if (out->query == NULL)
	{
		free(out->options);
		free(out);
		return NULL;
	}

	/* store /some/path means save to an asl file */
	if (out->action == ACTION_ASL_STORE)
	{
		if (out->options == NULL) out->dst = asl_out_dst_data_retain(_asl_out_module_parse_dst(m, ASL_PLACE_DATABASE_DEFAULT, 0755));
		else if (!strncmp(out->options, ASL_PLACE_DATABASE_DEFAULT, strlen(ASL_PLACE_DATABASE_DEFAULT))) out->dst = asl_out_dst_data_retain(_asl_out_module_parse_dst(m, out->options, 0755));
		else if (out->options != NULL) out->action = ACTION_ASL_FILE;
	}

	if ((out->action == ACTION_FILE) || (out->action == ACTION_ASL_FILE) || (out->action == ACTION_ASL_DIR))
	{
		mode_t def_mode = 0644;
		if (out->action == ACTION_ASL_DIR) def_mode = 0755;

		out->dst = asl_out_dst_data_retain(_asl_out_module_parse_dst(m, out->options, def_mode));
		if (out->dst == NULL)
		{
			out->action = ACTION_NONE;
			return out;
		}

		/*
		 * dst might have been set up by a previous ACTION_OUT_DEST ('>') rule with no mode.
		 * If so, mode would be 010000.  Set it now, since we know whether it is a file or dir.
		 */
		if (out->dst->mode == 010000) out->dst->mode = def_mode;

		if ((out->action == ACTION_FILE) && (out->dst != NULL) && (out->dst->fmt != NULL) && (!strcasecmp(out->dst->fmt, "asl")))
		{
			out->action = ACTION_ASL_FILE;
		}

		if ((out->action == ACTION_ASL_FILE) && (out->dst != NULL))
		{
			out->dst->flags |= MODULE_FLAG_TYPE_ASL;
		}

		if (out->action == ACTION_ASL_DIR)
		{
			/* coalesce is meaningless for ASL directories */
			out->dst->flags &= ~MODULE_FLAG_COALESCE;

			/* no compression at this point */
			out->dst->flags &= ~MODULE_FLAG_COMPRESS;

			out->dst->flags |= MODULE_FLAG_TYPE_ASL_DIR;

			/* set style bits for basestamp asl_dirs */
			if (((out->dst->flags & MODULE_FLAG_STYLE_BITS) == 0) && (out->dst->flags & MODULE_FLAG_BASESTAMP)) out->dst->flags |= MODULE_FLAG_STYLE_LCL_B;
		}

		/* only ACTION_FILE and ACTION_ASL_FILE may rotate */
		if ((out->action != ACTION_FILE) && (out->action != ACTION_ASL_FILE))
		{
			out->dst->flags &= ~MODULE_FLAG_ROTATE;
		}

#if !TARGET_IPHONE_SIMULATOR
		if (out->dst->nuid == 0) _dst_add_uid(out->dst, "0");
		if (out->dst->ngid == 0) _dst_add_gid(out->dst, "80");
#endif
	}

	if (m->ruleset == NULL) m->ruleset = out;
	else
	{
		for (rule = m->ruleset; rule->next != NULL; rule = rule->next);
		rule->next = out;
	}

	return out;
}

asl_out_rule_t *
asl_out_module_parse_line(asl_out_module_t *m, char *s)
{
	while ((*s == ' ') || (*s == '\t')) s++;

	if ((*s == 'Q') || (*s == '?') || (*s == '*'))
	{
		return _asl_out_module_parse_query_action(m, s);
	}
	else if (*s == '=')
	{
		return _asl_out_module_parse_set_param(m, s);
	}
	else if (*s == '>') 
	{
		_asl_out_module_parse_dst(m, s + 1, 010000);
	}

	return NULL;
}

asl_out_module_t *
asl_out_module_init_from_file(const char *name, FILE *f)
{
	asl_out_module_t *out;
	char *line;

	if (f == NULL) return NULL;

	out = asl_out_module_new(name);
	if (out == NULL) return NULL;

	/* read and parse config file */
	while (NULL != (line = get_line_from_file(f)))
	{
		asl_out_module_parse_line(out, line);
		free(line);
	}

	return out;
}

static asl_out_module_t *
_asl_out_module_find(asl_out_module_t *list, const char *name)
{
	asl_out_module_t *x;

	if (list == NULL) return NULL;
	if (name == NULL) return NULL;

	for (x = list; x != NULL; x = x->next)
	{
		if ((x->name != NULL) && (!strcmp(x->name, name))) return x;
	}

	return NULL;
}

static void
_asl_out_module_read_and_merge_dir(asl_out_module_t **list, const char *path, uint32_t flags)
{
	DIR *d;
	struct dirent *ent;
	FILE *f;
	asl_out_module_t *last, *x;

	if (list == NULL) return;
	if (path == NULL) return;

	last = *list;
	if (last != NULL)
	{
		while (last->next != NULL) last = last->next;
	}

	d = opendir(path);
	if (d != NULL)
	{
		while (NULL != (ent = readdir(d)))
		{
			if ((ent->d_name != NULL) && (ent->d_name[0] != '.'))
			{
				/* merge: skip this file if we already have a module with this name */
				if (_asl_out_module_find(*list, ent->d_name) != NULL) continue;

				char tmp[MAXPATHLEN];
				snprintf(tmp, sizeof(tmp), "%s/%s", path, ent->d_name);
				f = fopen(tmp, "r");
				if (f != NULL)
				{
					x = asl_out_module_init_from_file(ent->d_name, f);
					fclose(f);

					if (x != NULL)
					{
						x->flags |= flags;

						if (!strcmp(ent->d_name, ASL_MODULE_NAME))
						{
							/* com.apple.asl goes at the head of the list */
							x->next = *list;
							*list = x;
							if (last == NULL) last = *list;
						}
						else if (*list == NULL)
						{
							*list = x;
							last = *list;
						}
						else
						{
							last->next = x;
							last = x;
						}
					}
				}
			}
		}

		closedir(d);
	}
}

asl_out_module_t *
asl_out_module_init(void)
{
	asl_out_module_t *out = NULL;

#if TARGET_IPHONE_SIMULATOR
	char *sim_root_path, *sim_resources_path;
	char *asl_conf, *asl_conf_dir, *asl_conf_local_dir;

	sim_root_path = getenv("IPHONE_SIMULATOR_ROOT");
	assert(sim_root_path);

	sim_resources_path = getenv("IPHONE_SHARED_RESOURCES_DIRECTORY");
	assert(sim_resources_path);

	asprintf(&asl_conf, "%s%s", sim_root_path, _PATH_ASL_CONF);
	asprintf(&asl_conf_dir, "%s%s", sim_root_path, _PATH_ASL_CONF_DIR);
	asprintf(&asl_conf_local_dir, "%s%s", sim_resources_path, _PATH_ASL_CONF_DIR);

	_asl_out_module_read_and_merge_dir(&out, asl_conf_local_dir, MODULE_FLAG_LOCAL);
	free(asl_conf_local_dir);

	_asl_out_module_read_and_merge_dir(&out, asl_conf_dir, 0);
	free(asl_conf_dir);
#else
	_asl_out_module_read_and_merge_dir(&out, _PATH_ASL_CONF_LOCAL_DIR, MODULE_FLAG_LOCAL);
	_asl_out_module_read_and_merge_dir(&out, _PATH_ASL_CONF_DIR, 0);
#endif

	if (_asl_out_module_find(out, ASL_MODULE_NAME) == NULL)
	{
		/* system just has old-style /etc/asl.conf */
#if TARGET_IPHONE_SIMULATOR
		FILE *f = fopen(asl_conf, "r");
		free(asl_conf);
#else
		FILE *f = fopen(_PATH_ASL_CONF, "r");
#endif
		if (f != NULL)
		{
			asl_out_module_t *x = asl_out_module_init_from_file(ASL_MODULE_NAME, f);
			fclose(f);
			if (x != NULL)
			{
				x->next = out;
				out = x;
			}
		}
	}

	return out;
}

/*
 * Print rule
 */
char *
asl_out_module_rule_to_string(asl_out_rule_t *r)
{
	uint32_t len;
	char *str, *out;

	if (r == NULL)
	{
		asprintf(&out, "NULL rule");
		return out;
	}

	str = asl_msg_to_string(r->query, &len);

	asprintf(&out, "  %s%s%s%s%s",
			 asl_out_action_name[r->action],
			 (r->query == NULL) ? "" : " ", 
			 (r->query == NULL) ? "" : str,
			 (r->options == NULL) ? "" : " ", 
			 (r->options == NULL) ? "" : r->options);

	free(str);
	return out;
}

/*
 * Print module
 */
void
asl_out_module_print(FILE *f, asl_out_module_t *m)
{
	asl_out_rule_t *r, *n;
	asl_out_dst_data_t *o;
	uint32_t i, ttlnset;

	n = NULL;
	for (r = m->ruleset; r != NULL; r = n)
	{
		uint32_t len;
		char *str = asl_msg_to_string(r->query, &len);

		fprintf(f, "  %s", asl_out_action_name[r->action]);
		if (r->query != NULL) fprintf(f, " %s", str);
		if (r->options != NULL) fprintf(f, " %s", r->options);
		if (r->action == ACTION_OUT_DEST)
		{
			o = r->dst;
			if (o == NULL)
			{
				fprintf(f, "  data: NULL");
			}
			else
			{
				fprintf(f, "%s\n", o->path);
				fprintf(f, "    rules: %u\n", o->refcount - 1);
				fprintf(f, "    dest: %s\n", (o->rotate_dir == NULL) ? "(none)" : o->rotate_dir);
				fprintf(f, "    format: %s\n", (o->fmt == NULL) ? "std" : o->fmt);
				fprintf(f, "    time_format: %s\n", (o->tfmt == NULL) ? "lcl" : o->tfmt);
				fprintf(f, "    flags: 0x%08x", o->flags);
				if (o->flags != 0)
				{
					char c = '(';
					fprintf(f, " ");
					if (o->flags & MODULE_FLAG_ENABLED)
					{
						fprintf(f, "%cenabled", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_SOFT_WRITE)
					{
						fprintf(f, "%csoft", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_ROTATE)
					{
						fprintf(f, "%crotate", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_COALESCE)
					{
						fprintf(f, "%ccoalesce", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_COMPRESS)
					{
						fprintf(f, "%ccompress", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_STYLE_SEC)
					{
						fprintf(f, "%cseconds", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_STYLE_SEQ)
					{
						fprintf(f, "%csequence", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_STYLE_UTC)
					{
						fprintf(f, "%cutc", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_STYLE_UTC_B)
					{
						fprintf(f, "%cutc-basic", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_STYLE_LCL)
					{
						fprintf(f, "%clocal", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_STYLE_LCL_B)
					{
						fprintf(f, "%clocal-basic", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_BASESTAMP)
					{
						fprintf(f, "%cbasestamp", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_NONSTD_DIR)
					{
						fprintf(f, "%cnon-std_dir", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_EXTERNAL)
					{
						fprintf(f, "%cexternal", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_CRASHLOG)
					{
						fprintf(f, "%ccrashlog", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_TYPE_ASL)
					{
						fprintf(f, "%casl_file", c);
						c = ' ';
					}
					if (o->flags & MODULE_FLAG_TYPE_ASL_DIR)
					{
						fprintf(f, "%casl_directory", c);
						c = ' ';
					}
					fprintf(f, ")");
				}
				fprintf(f, "\n");

				fprintf(f, "    ttl: %u", o->ttl[LEVEL_ALL]);
				ttlnset = 0;
				for (i = 0; (i <= 7) & (ttlnset == 0); i++) if (o->ttl[i] != 0) ttlnset = 1;
				if (ttlnset != 0) for (i = 0; i <= 7; i++) printf(" [%d %d]", i, (o->ttl[i] == 0) ? o->ttl[LEVEL_ALL] : o->ttl[i]);
				fprintf(f, "\n");

				fprintf(f, "    mode: 0%o\n", o->mode);
				fprintf(f, "    file_max: %lu\n", o->file_max);
				fprintf(f, "    all_max: %lu\n", o->all_max);
#if !TARGET_IPHONE_SIMULATOR
				fprintf(f, "    uid:");
				for (i = 0; i < o->nuid; i++) fprintf(f, " %d", o->uid[i]);
				fprintf(f, "\n");
				fprintf(f, "    gid:");
				for (i = 0; i < o->ngid; i++) fprintf(f, " %d", o->gid[i]);
#endif
			}
		}

		fprintf(f, "\n");
		n = r->next;

		free(str);
	}
}

void
asl_out_file_list_free(asl_out_file_list_t *l)
{
	asl_out_file_list_t *n;

	if (l == NULL) return;

	while (l != NULL)
	{
		free(l->name);
		n = l->next;
		free(l);
		l = n;
	}
}

/*
 * Checks input name for the form base[.stamp][.gz]
 * name == base is allowed if src is true.
 * base.gz is not allowed.
 * Output parameter stamp must be freed by caller.
 */
bool
_check_file_name(const char *name, const char *base, bool src, char **stamp)
{
	size_t baselen, nparts;
	const char *p, *q, *part[2];
	bool isgz = false;

	if (name == NULL) return false;
	if (base == NULL) return false;

	baselen = strlen(base);
	if (baselen == 0) return false;

	if (stamp != NULL) *stamp = NULL;

	if (strncmp(name, base, baselen)) return false;

	p = name + baselen;

	/* name == base not allowed (it's the "active" file) */
	if (*p == '\0') return false;

	/* name must be base.something */
	if (*p != '.') return false;

	/* maximum of 2 parts (stamp and gz) */
	nparts = 0;
	for (q = p; *q != '\0'; q++)
	{
		if (*q == '.')
		{
			if (nparts == 2) return false;
			part[nparts++] = q + 1;
		}
	}

	if (nparts == 0) return false;

	isgz = strcmp(part[nparts - 1], "gz") == 0;

	/* no compressed files in src */
	if (src && isgz) return false;

	/* expecting base.stamp or base.stamp.gz */

	if (nparts == 1)
	{
		/* compressed files must have a stamp (base.gz is not allowed) */
		if (isgz) return false;

		/* got base.stamp */
		if (stamp != NULL) *stamp = strdup(part[0]);
		return true;
	}

	/* expecting base.stamp.gz */
	if (!isgz) return false;

	/* got base.stamp.gz */
	if (stamp != NULL)
	{
		*stamp = strdup(part[0]);
		char *x = strchr(*stamp, '.');
		if (x != NULL) *x = '\0';
	}

	return true;
}

/*
 * Find files in a directory (dir) that all have a common prefix (base).
 * Bits in flags further control the search.
 *
 * MODULE_FLAG_STYLE_SEQ means a numeric sequence number is expected, although not required.
 * E.g. foo.log foo.log.0
 *
 * MODULE_FLAG_STYLE_SEC also means a numeric sequence number is required following an 'T' character.
 * The numeric value is the file's timestamp in seconds.  E.g foo.log.T1335200452
 *
 * MODULE_FLAG_STYLE_UTC requires a date/time component as the file's timestamp.
 * E.g. foo.2012-04-06T15:30:00Z
 *
 * MODULE_FLAG_STYLE_UTC_B requires a date/time component as the file's timestamp.
 * E.g. foo.20120406T153000Z
 *
 * MODULE_FLAG_STYLE_LCL requires a date/time component as the file's timestamp.
 * E.g. foo.2012-04-06T15:30:00-7
 *
 * MODULE_FLAG_STYLE_LCL_B requires a date/time component as the file's timestamp.
 * E.g. foo.20120406T153000-07
 */
int
_parse_stamp_style(char *stamp, uint32_t flags, uint32_t *sp, time_t *tp)
{
	int i, n;
	bool digits;
	struct tm t;
	char zone;
	uint32_t h, m, s;
	long utc_offset = 0;
	time_t ftime = 0;

	/* check for NULL (no stamp) */
	if (stamp == NULL) return STAMP_STYLE_NULL;

	/* check for MODULE_FLAG_STYLE_SEC (foo.T12345678) */
	if (stamp[0] == 'T')
	{
		n = atoi(stamp + 1);
		if ((n == 0) && strcmp(stamp + 1, "0")) return STAMP_STYLE_INVALID;
		if (tp != NULL) *tp = (time_t)n;

		return STAMP_STYLE_SEC;
	}

	/* check for MODULE_FLAG_STYLE_SEQ (foo.0 or foo.2.gz) */
	digits = true;
	for (i = 0; digits && (stamp[i] != '\0'); i++) digits = (stamp[i] >= '0') && (stamp[i] <= '9');

	if (!digits && (!strcmp(stamp + i, ".gz"))) digits = true;

	if (digits)
	{
		n = atoi(stamp);
		if (sp != NULL) *sp = (uint32_t)n;
		return STAMP_STYLE_SEQ;
	}

	/* check for MODULE_FLAG_STYLE_UTC, UTC_B, LCL, or LCL_B */
	memset(&t, 0, sizeof(t));
	h = m = s = 0;

	n = 0;
	if ((flags & MODULE_FLAG_STYLE_UTC) || (flags & MODULE_FLAG_STYLE_LCL))
	{
		n = sscanf(stamp, "%d-%d-%dT%d:%d:%d%c%u:%u:%u", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &zone, &h, &m, &s);
	}
	else if ((flags & MODULE_FLAG_STYLE_UTC_B) || (flags & MODULE_FLAG_STYLE_LCL_B))
	{
		n = sscanf(stamp, "%4d%2d%2dT%2d%2d%2d%c%2u%2u%2u", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &zone, &h, &m, &s);
	}
	else
	{
		return STAMP_STYLE_INVALID;
	}

	if (n < 6) return STAMP_STYLE_INVALID;

	if (n == 6)
	{
		zone = 'J';
	}
	else if ((zone == '-') || (zone == '+'))
	{
		if (n >= 8) utc_offset += (3600 * h);
		if (n >= 9) utc_offset += (60 * m);
		if (n == 10) utc_offset += s;
		if (zone == '-') utc_offset *= -1;
	}
	else if ((zone >= 'A') && (zone <= 'Z'))
	{
		if (zone < 'J') utc_offset = 3600 * ((zone - 'A') + 1);
		else if ((zone >= 'K') && (zone <= 'M')) utc_offset = 3600 * (zone - 'A');
		else if (zone <= 'Y') utc_offset = -3600 * ((zone - 'N') + 1);
	}
	else if ((zone >= 'a') && (zone <= 'z'))
	{
		if (zone < 'j') utc_offset = 3600 * ((zone - 'a') + 1);
		else if ((zone >= 'k') && (zone <= 'm')) utc_offset = 3600 * (zone - 'a');
		else if (zone <= 'y') utc_offset = -3600 * ((zone - 'n') + 1);
	}
	else
	{
		return STAMP_STYLE_INVALID;
	}

	t.tm_year -= 1900;
	t.tm_mon -= 1;
	t.tm_sec += utc_offset;
	t.tm_isdst = -1;

	if ((zone == 'J') || (zone == 'j')) ftime = mktime(&t);
	else ftime = timegm(&t);

	if (tp != NULL) *tp = ftime;

	return STAMP_STYLE_UTC_OR_LCL;
}

asl_out_file_list_t *
asl_list_log_files(const char *dir, const char *base, bool src, uint32_t flags)
{
	DIR *d;
	struct dirent *ent;
	char path[MAXPATHLEN];
	uint32_t seq;
	time_t ftime;
	struct stat sb;
	int pstyle, fstyle;
	asl_out_file_list_t *out, *x, *y;

	if (dir == NULL) return NULL;
	if (base == NULL) return NULL;

	out = NULL;

	d = opendir(dir);
	if (d == NULL) return NULL;

	while (NULL != (ent = readdir(d)))
	{
		char *stamp = NULL;
		bool check;

		if (ent->d_name == NULL) continue;

		check = _check_file_name(ent->d_name, base, src, &stamp);
		if (!check) continue;

		seq = IndexNull;
		ftime = 0;

		pstyle = _parse_stamp_style(stamp, flags, &seq, &ftime);
		free(stamp);

		if (pstyle == STAMP_STYLE_INVALID) continue;

		fstyle = STAMP_STYLE_NULL;
		if (flags & MODULE_FLAG_STYLE_SEC) fstyle = STAMP_STYLE_SEC;
		else if (flags & MODULE_FLAG_STYLE_SEQ) fstyle = STAMP_STYLE_SEQ;
		else if ((flags & MODULE_FLAG_STYLE_UTC) || (flags & MODULE_FLAG_STYLE_LCL)) fstyle = STAMP_STYLE_UTC_OR_LCL;
		else if ((flags & MODULE_FLAG_STYLE_UTC_B) || (flags & MODULE_FLAG_STYLE_LCL_B)) fstyle = STAMP_STYLE_UTC_OR_LCL;

		/*
		 * accept the file if:
		 * style is STAMP_STYLE_NULL (no timestamp)
		 * src is true and style is STAMP_STYLE_SEC
		 * actual style matches the style implied by the input flags
		 */

		check = false;
		if (pstyle == STAMP_STYLE_NULL) check = true;
		if ((pstyle == STAMP_STYLE_SEC) && src) check = true;
		if (pstyle == fstyle) check = true;

		if (!check) continue;

		x = (asl_out_file_list_t *)calloc(1, sizeof(asl_out_file_list_t));
		if (x == NULL)
		{
			asl_out_file_list_free(out);
			return NULL;
		}

		x->name = strdup(ent->d_name);
		x->ftime = ftime;
		x->seq = seq;

		memset(&sb, 0, sizeof(sb));
		snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
		if (stat(path, &sb) == 0)
		{
			x->size = sb.st_size;
			if (pstyle == STAMP_STYLE_SEQ)
			{
				x->ftime = sb.st_birthtimespec.tv_sec;
				if (x->ftime == 0) x->ftime = sb.st_mtimespec.tv_sec;
			}
		}

		if (pstyle == STAMP_STYLE_SEQ)
		{
			if (out == NULL)
			{
				out = x;
			}
			else if ((x->seq == IndexNull) || ((x->seq < out->seq) && (out->seq != IndexNull)))
			{
				x->next = out;
				out->prev = x;
				out = x;
			}
			else
			{
				for (y = out; y != NULL; y = y->next)
				{
					if (y->next == NULL)
					{
						y->next = x;
						x->prev = y;
						break;
					}
					else if ((x->seq < y->next->seq) && (y->next->seq != IndexNull))
					{
						x->next = y->next;
						y->next = x;
						x->prev = y;
						x->next->prev = x;
						break;
					}
				}
			}
		}
		else
		{
			if (out == NULL)
			{
				out = x;
			}
			else if (x->ftime < out->ftime)
			{
				x->next = out;
				out->prev = x;
				out = x;
			}
			else
			{
				for (y = out; y != NULL; y = y->next)
				{
					if (y->next == NULL)
					{
						y->next = x;
						x->prev = y;
						break;
					}
					else if (x->ftime < y->next->ftime)
					{
						x->next = y->next;
						y->next = x;
						x->prev = y;
						x->next->prev = x;
						break;
					}
				}
			}
		}
	}

	closedir(d);
	return out;
}

/*
 * List the source files for an output asl_out_dst_data_t
 */
asl_out_file_list_t *
asl_list_src_files(asl_out_dst_data_t *dst)
{
	char *base;
	uint32_t flags = MODULE_FLAG_STYLE_SEC;
	asl_out_file_list_t *out;

	if (dst == NULL) return NULL;
	if (dst->path == NULL) return NULL;

	/*
	 * MODULE_FLAG_EXTERNAL means some process other than syslogd writes the file.
	 * We check for its existence, and that it is non-zero in size.
	 */
	if (dst->flags & MODULE_FLAG_EXTERNAL)
	{
		struct stat sb;

		memset(&sb, 0, sizeof(struct stat));

		if (stat(dst->path, &sb) == 0)
		{
			if (S_ISREG(sb.st_mode) && (sb.st_size != 0))
			{
				out = (asl_out_file_list_t *)calloc(1, sizeof(asl_out_file_list_t));
				if (out != NULL)
				{
					char *p = strrchr(dst->path, '/');
					if (p == NULL) p = dst->path;
					else p++;
					out->name = strdup(p);
					out->ftime = sb.st_birthtimespec.tv_sec;
					if (out->ftime == 0) out->ftime = sb.st_mtimespec.tv_sec;
					return out;
				}
			}
		}

		return NULL;
	}

	/*
	 * Checkpoint / source format may be one of:
	 * MODULE_FLAG_STYLE_SEC   (foo.T12345678.log),
	 * MODULE_FLAG_STYLE_UTC   (foo.20120-06-24T12:34:56Z.log)
	 * MODULE_FLAG_STYLE_UTC_B (foo.201200624T123456Z.log)
	 * MODULE_FLAG_STYLE_LCL   (foo.20120-06-24T12:34:56-7.log)
	 * MODULE_FLAG_STYLE_LCL_B (foo.201200624T123456-07.log)
	 *
	 * MODULE_FLAG_STYLE_SEC format is used for sequenced (MODULE_FLAG_STYLE_SEQ) files.
	 * aslmanager converts the file names.
	 */

	if (dst->flags & MODULE_FLAG_STYLE_UTC) flags = MODULE_FLAG_STYLE_UTC;
	else if (dst->flags & MODULE_FLAG_STYLE_UTC_B) flags = MODULE_FLAG_STYLE_UTC_B;
	else if (dst->flags & MODULE_FLAG_STYLE_LCL) flags = MODULE_FLAG_STYLE_LCL;
	else if (dst->flags & MODULE_FLAG_STYLE_LCL_B) flags = MODULE_FLAG_STYLE_LCL_B;

	if ((dst->rotate_dir == NULL) && ((dst->flags & MODULE_FLAG_STYLE_SEQ) == 0) && ((dst->flags & MODULE_FLAG_COMPRESS) == 0))
	{
		/* files do not move to a dest dir, get renamed, or get compressed - nothing to do */
		return NULL;
	}

	base = strrchr(dst->path, '/');
	if (base == NULL) return NULL;

	*base = '\0';
	base++;

	out = asl_list_log_files(dst->path, base, true, flags);

	if (base != NULL) *--base = '/';

	return out;
}

/*
 * List the destination files for an output asl_out_dst_data_t
 */
asl_out_file_list_t *
asl_list_dst_files(asl_out_dst_data_t *dst)
{
	char *base, *dst_dir;
	asl_out_file_list_t *out;

	if (dst == NULL) return NULL;
	if (dst->path == NULL) return NULL;

	base = strrchr(dst->path, '/');
	if (base == NULL) return NULL;

	*base = '\0';
	base++;

	dst_dir = dst->rotate_dir;
	if (dst_dir == NULL) dst_dir = dst->path;

	out = asl_list_log_files(dst_dir, base, false, dst->flags);

	if (base != NULL) *--base = '/';

	return out;
}

static int
asl_secure_open_dir(const char *path)
{
	int fd, i;
	char **path_parts;

	if (path == NULL) return -1;
	if (path[0] != '/') return -1;

	path_parts = explode(path + 1, "/");
	if (path_parts == NULL) return 0;

	fd = open("/", O_RDONLY | O_NOFOLLOW, 0);
	if (fd < 0)
	{
		free_string_list(path_parts);
		return -1;
	}

	for (i = 0; path_parts[i] != NULL; i++)
	{
		int fd_next, status;
		struct stat sb;

		fd_next = openat(fd, path_parts[i], O_RDONLY | O_NOFOLLOW, 0);
		close(fd);
		fd = fd_next;
		if (fd < 0)
		{
			free_string_list(path_parts);
			return -1;
		}

		memset(&sb, 0, sizeof(sb));

		status = fstat(fd, &sb);
		if (status < 0)
		{
			free_string_list(path_parts);
			return -1;
		}

		if (!S_ISDIR(sb.st_mode))
		{
			free_string_list(path_parts);
			return -1;
		}
	}

	free_string_list(path_parts);
	return fd;
}

int
asl_secure_chown_chmod_dir(const char *path, uid_t uid, gid_t gid, mode_t mode)
{
	int fd, status;
 
	fd = asl_secure_open_dir(path);
	if (fd < 0) return fd;

	status = fchown(fd, uid, gid);
	if (status < 0)
	{
		close(fd);
		return -1;
	}

	if (mode >= 0) status = fchmod(fd, mode);
	close(fd);

	if (status < 0) return -1;
	return 0;
}
