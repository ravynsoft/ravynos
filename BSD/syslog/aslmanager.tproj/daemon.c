/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <asl.h>
#include <asl_private.h>
#include <asl_core.h>
#include <asl_file.h>
#include <asl_store.h>
#include <copyfile.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <zlib.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include <mach/mach.h>
#include <fcntl.h>
#include <sys/attr.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <os/assumes.h>
#include <vproc_priv.h>
#include <libkern/OSAtomic.h>
#include "daemon.h"
#include "asl_ipc.h"

/* global */
extern bool dryrun;
extern uint32_t debug;
extern FILE *debugfp;
extern dispatch_queue_t work_queue;

static mach_port_t asl_server_port;
static aslclient aslc;
static int asl_aux_fd = -1;

const char *
keep_str(uint8_t mask)
{
	static char str[9];
	uint32_t x = 0;

	memset(str, 0, sizeof(str));
	if (mask & 0x01) str[x++] = '0';
	if (mask & 0x02) str[x++] = '1';
	if (mask & 0x04) str[x++] = '2';
	if (mask & 0x08) str[x++] = '3';
	if (mask & 0x10) str[x++] = '4';
	if (mask & 0x20) str[x++] = '5';
	if (mask & 0x40) str[x++] = '6';
	if (mask & 0x80) str[x++] = '7';
	if (x == 0) str[x++] = '-';
	return str;
}

void
set_debug(int flag, const char *str)
{
	int level, x;

	if (str == NULL) x = ASL_LEVEL_ERR;
	else if (((str[0] == 'L') || (str[0] == 'l')) && ((str[1] >= '0') && (str[1] <= '7')) && (str[2] == '\0')) x = atoi(str+1);
	else if ((str[0] >= '0') && (str[0] <= '7') && (str[1] == '\0')) x = ASL_LEVEL_CRIT + atoi(str);
	else x = ASL_LEVEL_ERR;

	if (x <= 0) x = 0;
	else if (x > 7) x = 7;

	level = debug & DEBUG_LEVEL_MASK;
	if (x > level) level = x;

	debug = debug & DEBUG_FLAG_MASK;
	debug |= flag;
	debug |= level;
}

void
debug_log(int level, char *str, ...)
{
	va_list v;
	char ts[32];

	time_t now = time(NULL);
	memset(ts, 0, sizeof(ts));
	strftime(ts, sizeof(ts), "%b %e %T", localtime(&now));

	if ((debug & DEBUG_STDERR) && (level <= (debug & DEBUG_LEVEL_MASK)))
	{
		fprintf(stderr, "%s: ", ts);
		va_start(v, str);
		vfprintf(stderr, str, v);
		va_end(v);
	}

	if ((debug & DEBUG_FILE) && (debugfp != NULL))
	{
		fprintf(debugfp, "%s: ", ts);
		va_start(v, str);
		vfprintf(debugfp, str, v);
		va_end(v);
	}

	if (debug & DEBUG_ASL)
	{
		char *line = NULL;

		if (aslc == NULL)
		{
			aslc = asl_open("aslmanager", "syslog", 0);
			asl_msg_t *msg = asl_msg_new(ASL_TYPE_MSG);

			asl_msg_set_key_val(msg, ASL_KEY_MSG, "Status Report");
			asl_msg_set_key_val(msg, ASL_KEY_LEVEL, ASL_STRING_NOTICE);
			asl_create_auxiliary_file((asl_object_t)msg, "Status Report", "public.text", &asl_aux_fd);
			asl_msg_release(msg);
		}

		va_start(v, str);
		vasprintf(&line, str, v);
		va_end(v);

		if (line != NULL)
		{
			write(asl_aux_fd, ts, strlen(ts));
			write(asl_aux_fd, line, strlen(line));
		}

		free(line);
	}
}

void
debug_close()
{
	if (asl_aux_fd >= 0) asl_close_auxiliary_file(asl_aux_fd);
	if (debugfp != NULL) fclose(debugfp);
}

name_list_t *
add_to_name_list(name_list_t *l, const char *name, size_t size, uint32_t flags)
{
	name_list_t *e, *x;

	if (name == NULL) return l;

	e = (name_list_t *)calloc(1, sizeof(name_list_t));
	if (e == NULL) return NULL;

	e->name = strdup(name);
	if (e->name == NULL)
	{
		free(e);
		return NULL;
	}

	e->size = size;
	e->flags = flags;

	/* list is sorted by name (i.e. primarily by timestamp) */
	if (l == NULL) return e;

	if (strcmp(e->name, l->name) <= 0)
	{
		e->next = l;
		return e;
	}

	for (x = l; (x->next != NULL) && (strcmp(e->name, x->next->name) > 0) ; x = x->next);

	e->next = x->next;
	x->next = e;
	return l;
}

void
free_name_list(name_list_t *l)
{
	name_list_t *e;

	while (l != NULL)
	{
		e = l;
		l = l->next;
		free(e->name);
		free(e);
	}

	free(l);
}

int
copy_compress_file(asl_out_dst_data_t *asldst, const char *src, const char *dst)
{
	int in, out;
	size_t n;
	gzFile gz;
	char buf[IOBUFSIZE];

	in = open(src, O_RDONLY, 0);
	if (in < 0) return -1;

	out = open(dst, O_WRONLY | O_CREAT, asldst->mode & 0666);
	if (out >= 0) out = asl_out_dst_set_access(out, asldst);
	if (out < 0)
	{
		close(in);
		return -1;
	}

	gz = gzdopen(out, "w");
	if (gz == NULL)
	{
		close(in);
		close(out);
		return -1;
	}

	do {
		n = read(in, buf, sizeof(buf));
		if (n > 0) gzwrite(gz, buf, n);
	} while (n == IOBUFSIZE);

	gzclose(gz);
	close(in);
	close(out);

	return 0;
}

void
filesystem_rename(const char *src, const char *dst)
{
	int status = 0;

	debug_log(ASL_LEVEL_NOTICE, "  rename %s ---> %s\n", src, dst);
	if (dryrun) return;

	status = rename(src, dst);
	if (status != 0) debug_log(ASL_LEVEL_ERR, "  FAILED status %d errno %d [%s] rename %s ---> %s\n", status, errno, strerror(errno), src, dst);
}

void
filesystem_unlink(const char *path)
{
	int status = 0;

	debug_log(ASL_LEVEL_NOTICE, "  remove %s\n", path);
	if (dryrun) return;

	status = unlink(path);
	if (status != 0) debug_log(ASL_LEVEL_ERR, "  FAILED status %d errno %d [%s] unlink %s\n", status, errno, strerror(errno), path);
}

void
filesystem_truncate(const char *path)
{
	int status = 0;

	debug_log(ASL_LEVEL_NOTICE, "  truncate %s\n", path);
	if (dryrun) return;

	status = truncate(path, 0);
	if (status != 0) debug_log(ASL_LEVEL_ERR, "  FAILED status %d errno %d [%s] unlink %s\n", status, errno, strerror(errno), path);
}

void
filesystem_rmdir(const char *path)
{
	int status = 0;

	debug_log(ASL_LEVEL_NOTICE, "  remove directory %s\n", path);
	if (dryrun) return;

	status = rmdir(path);
	if (status != 0) debug_log(ASL_LEVEL_ERR, "  FAILED status %d errno %d [%s] rmdir %s\n", status, errno, strerror(errno), path);
}

/*
 * Copy ASL files by reading and writing each record.
 */
static uint32_t
copy_asl_file(const char *src, const char *dst, mode_t mode)
{
	asl_file_t *fin, *fout;
	uint32_t status;

	if (src == NULL) return ASL_STATUS_INVALID_ARG;
	if (dst == NULL) return ASL_STATUS_INVALID_ARG;

	fin = NULL;
	status = asl_file_open_read(src, &fin);
	if (status != ASL_STATUS_OK) return status;

	fout = NULL;
	status = asl_file_open_write(dst, mode, -1, -1, &fout);
	if (status != ASL_STATUS_OK)
	{
		asl_file_close(fin);
		return status;
	}

	if (fout == NULL)
	{
		asl_file_close(fin);
		return ASL_STATUS_FAILED;
	}

	fout->flags = ASL_FILE_FLAG_PRESERVE_MSG_ID;

	status = asl_file_read_set_position(fin, ASL_FILE_POSITION_FIRST);
	if (status != ASL_STATUS_OK)
	{
		asl_file_close(fin);
		asl_file_close(fout);
		return ASL_STATUS_READ_FAILED;
	}

	while (status == ASL_STATUS_OK)
	{
		uint64_t mid = 0;
		asl_msg_t *msg = NULL;

		status = asl_file_fetch_next(fin, &msg);
		if (msg == NULL)
		{
			status = ASL_STATUS_OK;
			break;
		}

		if (status != ASL_STATUS_OK) break;

		status = asl_file_save(fout, msg, &mid);
		asl_msg_release(msg);
	}

	asl_file_close(fin);
	asl_file_close(fout);

	return status;
}

int32_t
filesystem_copy(asl_out_dst_data_t *asldst, const char *src, const char *dst, uint32_t flags)
{
	char *dot;

	if ((src == NULL) || (dst == NULL)) return 0;

	dot = strrchr(src, '.');
	if ((dot != NULL) && (!strcmp(dot, ".gz"))) flags &= ~MODULE_FLAG_COMPRESS;

	if (((flags & MODULE_FLAG_COMPRESS) == 0) && (!strcmp(src, dst))) return 0;

	if (flags & MODULE_FLAG_TYPE_ASL) debug_log(ASL_LEVEL_NOTICE, "  copy asl %s ---> %s\n", src, dst);
	else if (flags & MODULE_FLAG_COMPRESS) debug_log(ASL_LEVEL_NOTICE, "  copy compress %s ---> %s.gz\n", src, dst);
	else debug_log(ASL_LEVEL_NOTICE, "  copy %s ---> %s\n", src, dst);

	if (dryrun) return 0;

	if (flags & MODULE_FLAG_TYPE_ASL)
	{
		uint32_t status = copy_asl_file(src, dst, asldst->mode);
		if (status != 0)
		{
			debug_log(ASL_LEVEL_ERR, "  FAILED status %u [%s] asl copy %s ---> %s\n", status, asl_core_error(status), src, dst);
			return 0;
		}
	}
	else if (flags & MODULE_FLAG_COMPRESS)
	{
		char gzdst[MAXPATHLEN];

		snprintf(gzdst, sizeof(gzdst), "%s.gz", dst);

		int status = copy_compress_file(asldst, src, gzdst);
		if (status != 0)
		{
			debug_log(ASL_LEVEL_ERR, "  FAILED status %d errno %d [%s] copy & compress %s ---> %s\n", status, errno, strerror(errno), src, dst);
			return 0;
		}
	}
	else
	{
		int status = copyfile(src, dst, NULL, COPYFILE_ALL | COPYFILE_RECURSIVE);
		if (status != 0)
		{
			debug_log(ASL_LEVEL_ERR, "  FAILED status %d errno %d [%s] copy %s ---> %s\n", status, errno, strerror(errno), src, dst);
			return 0;
		}
	}

	return 1;
}

int32_t
filesystem_reset_ctime(const char *path)
{
	struct attrlist attr_list;
	struct timespec now;

	debug_log(ASL_LEVEL_NOTICE, "  reset ctime %s\n", path);

	memset(&attr_list, 0, sizeof(attr_list));
	attr_list.bitmapcount = ATTR_BIT_MAP_COUNT;
	attr_list.commonattr = ATTR_CMN_CRTIME;

	memset(&now, 0, sizeof(now));
	now.tv_sec = time(NULL);

	return setattrlist(path, &attr_list, &now, sizeof(now), 0);
}

int
remove_directory(const char *path)
{
	DIR *dp;
	struct dirent *dent;
	char *str;

	dp = opendir(path);
	if (dp == NULL) return 0;

	while ((dent = readdir(dp)) != NULL)
	{
		if ((!strcmp(dent->d_name, ".")) || (!strcmp(dent->d_name, ".."))) continue;
		asprintf(&str, "%s/%s", path, dent->d_name);
		if (str != NULL)
		{
			filesystem_unlink(str);
			free(str);
			str = NULL;
		}
	}

	closedir(dp);
	filesystem_rmdir(path);

	return 0;
}

size_t
directory_size(const char *path)
{
	DIR *dp;
	struct dirent *dent;
	struct stat sb;
	size_t size;
	char *str;

	dp = opendir(path);
	if (dp == NULL) return 0;

	size = 0;
	while ((dent = readdir(dp)) != NULL)
	{
		if ((!strcmp(dent->d_name, ".")) || (!strcmp(dent->d_name, ".."))) continue;

		memset(&sb, 0, sizeof(struct stat));
		str = NULL;
		asprintf(&str, "%s/%s", path, dent->d_name);

		if ((str != NULL) && (stat(str, &sb) == 0) && S_ISREG(sb.st_mode))
		{
			size += sb.st_size;
			free(str);
		}
	}

	closedir(dp);
	return size;
}

time_t
parse_ymd_name(const char *name)
{
	struct tm ftime;
	time_t created;
	int32_t tzh, tzm, sign = -1;
	const char *x;
	bool legacy = false;

	if (name == NULL) return -1;

	x = name;

	if ((*x == 'T') || (*x == 't'))
	{
		x++;
		created = atol(x);
		if ((created == 0) && (*x != '0')) return -1;

		x = strchr(x, '.');
		if (x == NULL) return -1;

		return created;
	}

	memset(&ftime, 0, sizeof(ftime));

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_year = 1000 * (*x++ - '0');

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_year += 100 * (*x++ - '0');

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_year += 10 * (*x++ - '0');

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_year += *x++ - '0';
	ftime.tm_year -= 1900;

	if (*x == '-') x++;
	else if (*x == '.')
	{
		x++;
		legacy = true;
	}

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_mon = 10 * (*x++ - '0');

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_mon += *x++ - '0';
	ftime.tm_mon -= 1;

	if ((*x == '-') || (*x == '.')) x++;

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_mday = 10 * (*x++ - '0');

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_mday += *x++ - '0';

	if (legacy)
	{
		if ((*x != '.') && (*x != '\0')) return -1;

		/* assume the file was created at midnight */
		ftime.tm_hour = 24;
		ftime.tm_min = 0;
		ftime.tm_sec = 0;
		ftime.tm_isdst = -1;

		created = mktime(&ftime);
		return created;
	}

	if ((*x != 'T') && (*x != 't')) return 1;
	x++;

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_hour = 10 * (*x++ - '0');

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_hour += *x++ - '0';

	if (*x == ':') x++;

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_min = 10 * (*x++ - '0');

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_min += *x++ - '0';

	if (*x == ':') x++;

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_sec = 10 * (*x++ - '0');

	if ((*x < '0') || (*x > '9')) return 1;
	ftime.tm_sec += *x++ - '0';

	if ((*x == 'Z') || (*x == 'z'))
	{
		created = timegm(&ftime);
		return created;
	}

	if ((*x != '+') && (*x != '-')) return 1;

	if (*x == '-') sign = 1;
	x++;

	if ((*x < '0') || (*x > '9')) return 1;
	tzh = 10 * (*x++ - '0');

	if ((*x < '0') || (*x > '9')) tzh /= 10;
	else tzh += *x++ - '0';

	if (tzh > 23) return 1;

	tzm = 0;
	if ((*x == ':') || ((*x >= '0') && (*x <= '9')))
	{
		if (*x != ':') tzm = 10 * (*x - '0');
		x++;

		if ((*x < '0') || (*x > '9'))return -1;
		tzm += *x++ - '0';

		if (tzm > 59) return -1;
	}

	ftime.tm_sec += (sign * (tzh * SECONDS_PER_HOUR) + (tzm * SECONDS_PER_MINUTE));

	if ((*x != '.') && (*x != '\0')) return -1;

	created = timegm(&ftime);
	return created;
}


/*
 * Determine the age (in seconds) of a YMD file from its name.
 * Also determines UID and GID from ".Unnn.Gnnn" part of file name.
 */
uint32_t
ymd_file_age(const char *name, time_t now, uid_t *u, gid_t *g)
{
	struct tm ftime;
	time_t created;
	uint32_t seconds;
	const char *p;

	if (name == NULL) return 0;

	if (now == 0) now = time(NULL);

	memset(&ftime, 0, sizeof(struct tm));

	created = parse_ymd_name(name);
	if (created < 0) return 0;
	if (created > now) return 0;
	seconds = now - created;

	if (u != NULL)
	{
		*u = -1;
		p = strchr(name, 'U');
		if (p != NULL) *u = atoi(p+1);
	}

	if (g != NULL)
	{
		*g = -1;
		p = strchr(name, 'G');
		if (p != NULL) *g = atoi(p+1);
	}

	return seconds;
}


static void
aux_url_callback(const char *url)
{
	if (url == NULL) return;
	if (!strncmp(url, AUX_URL_MINE, AUX_URL_MINE_LEN)) filesystem_unlink(url + AUX_URL_PATH_OFFSET);
}

uint32_t
ymd_file_filter(const char *name, const char *path, uint32_t keep_mask, mode_t ymd_mode, uid_t ymd_uid, gid_t ymd_gid)
{
	asl_file_t *f = NULL;
	uint8_t km = keep_mask;
	uint32_t status, len, dstcount = 0;
	char src[MAXPATHLEN];
	char dst[MAXPATHLEN];

	if (snprintf(src, MAXPATHLEN, "%s/%s", path, name) >= MAXPATHLEN) return ASL_STATUS_FAILED;
	if (snprintf(dst, MAXPATHLEN, "%s/%s", path, name) >= MAXPATHLEN) return ASL_STATUS_FAILED;
	len = strlen(src) - 3;
	snprintf(dst + len, 4, "tmp");

	//TODO: check if src file is already filtered
	debug_log(ASL_LEVEL_NOTICE, "  filter %s %s ---> %s\n", src, keep_str(km), dst);

	status = ASL_STATUS_OK;

	if (!dryrun)
	{
		status = asl_file_open_read(name, &f);
		if (status != ASL_STATUS_OK) return status;

		status = asl_file_filter_level(f, dst, keep_mask, ymd_mode, ymd_uid, ymd_gid, &dstcount, aux_url_callback);
		asl_file_close(f);
	}

	filesystem_unlink(src);
	if ((status != ASL_STATUS_OK) || (dstcount == 0)) filesystem_unlink(dst);
	else filesystem_rename(dst, src);

	return status;
}

int
process_asl_data_store(asl_out_dst_data_t *dst, asl_out_dst_data_t *opts)
{
	time_t now, midnight, since_midnight;
	char *str;
	DIR *dp;
	struct dirent *dent;
	name_list_t *ymd_list, *bb_list, *aux_list, *bb_aux_list, *e;
	size_t file_size, store_size;
	struct stat sb;
	char tstr[128];
	struct tm t_tmp;
	uint32_t ttl = 0;

	ymd_list = NULL;
	bb_list = NULL;
	aux_list = NULL;
	bb_aux_list = NULL;
	store_size = 0;

	if (dst == NULL) return 0;
	if (dst->path == NULL) return 0;

	ttl = dst->ttl[LEVEL_ALL];
	if ((opts != NULL) && (opts->ttl[LEVEL_ALL] > 0)) ttl = opts->ttl[LEVEL_ALL];

	size_t all_max = dst->all_max;
	if ((opts != NULL) && (opts->all_max > 0)) all_max = opts->all_max;

	debug_log(ASL_LEVEL_NOTICE, "----------------------------------------\n");
	debug_log(ASL_LEVEL_NOTICE, "Processing data store %s\n", dst->path);

	if (dst->rotate_dir != NULL)
	{
		/* check archive */
		memset(&sb, 0, sizeof(struct stat));
		if (stat(dst->rotate_dir, &sb) == 0)
		{
			/* must be a directory */
			if (!S_ISDIR(sb.st_mode))
			{
				debug_log(ASL_LEVEL_ERR, "aslmanager error: archive %s is not a directory", dst->rotate_dir);
				return -1;
			}
		}
		else
		{
			if (errno == ENOENT)
			{
				/* archive doesn't exist - create it */
				if (mkdir(dst->rotate_dir, 0755) != 0)
				{
					debug_log(ASL_LEVEL_ERR, "aslmanager error: can't create archive %s: %s\n", dst->rotate_dir, strerror(errno));
					return -1;
				}
			}
			else
			{
				/* stat failed for some other reason */
				debug_log(ASL_LEVEL_ERR, "aslmanager error: can't stat archive %s: %s\n", dst->rotate_dir, strerror(errno));
				return -1;
			}
		}
	}

	chdir(dst->path);

	/* determine current time */
	now = time(NULL);

	localtime_r(&now, &t_tmp);

	t_tmp.tm_sec = 0;
	t_tmp.tm_min = 0;
	t_tmp.tm_hour = 0;

	midnight = mktime(&t_tmp);
	since_midnight = now - midnight;

	dp = opendir(dst->path);
	if (dp == NULL) return -1;

	/* gather a list of YMD files, AUX dirs, BB.AUX dirs, and BB files */
	while ((dent = readdir(dp)) != NULL)
	{
		uint32_t file_flags = 0;
		char *dot = NULL;

		memset(&sb, 0, sizeof(struct stat));
		file_size = 0;
		if (stat(dent->d_name, &sb) == 0) file_size = sb.st_size;

		dot = strrchr(dent->d_name, '.');
		if ((dot != NULL) && !strcmp(dot, ".gz")) file_flags |= NAME_LIST_FLAG_COMPRESSED;

		if ((dent->d_name[0] >= '0') && (dent->d_name[0] <= '9'))
		{
			ymd_list = add_to_name_list(ymd_list, dent->d_name, file_size, file_flags);
			store_size += file_size;
		}
		else if (((dent->d_name[0] == 'T') || (dent->d_name[0] == 't')) && (dent->d_name[1] >= '0') && (dent->d_name[1] <= '9'))
		{
			ymd_list = add_to_name_list(ymd_list, dent->d_name, file_size, file_flags);
			store_size += file_size;
		}
		else if (!strncmp(dent->d_name, "AUX.", 4) && (dent->d_name[4] >= '0') && (dent->d_name[4] <= '9') && S_ISDIR(sb.st_mode))
		{
			file_size = directory_size(dent->d_name);
			aux_list = add_to_name_list(aux_list, dent->d_name, file_size, file_flags);
			store_size += file_size;
		}
		else if (!strncmp(dent->d_name, "BB.AUX.", 7) && (dent->d_name[7] >= '0') && (dent->d_name[7] <= '9') && S_ISDIR(sb.st_mode))
		{
			file_size = directory_size(dent->d_name);
			bb_aux_list = add_to_name_list(bb_aux_list, dent->d_name, file_size, file_flags);
			store_size += file_size;
		}
		else if (!strncmp(dent->d_name, "BB.", 3) && (dent->d_name[3] >= '0') && (dent->d_name[3] <= '9'))
		{
			bb_list = add_to_name_list(bb_list, dent->d_name, file_size, file_flags);
			store_size += file_size;
		}
		else if ((!strcmp(dent->d_name, ".")) || (!strcmp(dent->d_name, "..")))
		{}
		else if ((!strcmp(dent->d_name, "StoreData")) || (!strcmp(dent->d_name, "SweepStore")))
		{}
		else if (!strcmp(dent->d_name, ASL_INTERNAL_LOGS_DIR))
		{}
		else
		{
			debug_log(ASL_LEVEL_ERR, "aslmanager: unexpected file %s in ASL data store\n", dent->d_name);
		}
	}

	closedir(dp);

	debug_log(ASL_LEVEL_NOTICE, "Data Store Size = %lu\n", store_size);
	asl_core_time_to_str(ttl, tstr, sizeof(tstr));
	debug_log(ASL_LEVEL_NOTICE, "Data Store YMD Files (TTL = %s)\n", tstr);
	for (e = ymd_list; e != NULL; e = e->next)
	{
		uint32_t age = ymd_file_age(e->name, now, NULL, NULL);
		asl_core_time_to_str(age, tstr, sizeof(tstr));
		debug_log(ASL_LEVEL_NOTICE, "  %s   %lu (age %s%s)\n", e->name, e->size, tstr, (age > ttl) ? " - expired" : "");
	}

	debug_log(ASL_LEVEL_NOTICE, "Data Store AUX Directories\n");
	for (e = aux_list; e != NULL; e = e->next)
	{
		uint32_t age = ymd_file_age(e->name + 4, now, NULL, NULL) / SECONDS_PER_DAY;
		asl_core_time_to_str(age, tstr, sizeof(tstr));
		debug_log(ASL_LEVEL_NOTICE, "  %s   %lu (age %s)\n", e->name, e->size, tstr, (age > ttl) ? " - expired" : "");
	}

	debug_log(ASL_LEVEL_NOTICE, "Data Store BB.AUX Directories\n");
	for (e = bb_aux_list; e != NULL; e = e->next)
	{
		uint32_t age = ymd_file_age(e->name + 7, now, NULL, NULL);
		asl_core_time_to_str(age, tstr, sizeof(tstr));
		debug_log(ASL_LEVEL_NOTICE, "  %s   %lu (age %s)\n", e->name, e->size, tstr, ((age / SECONDS_PER_DAY) > 0) ? " - expired" : "");
	}

	debug_log(ASL_LEVEL_NOTICE, "Data Store BB Files\n");
	for (e = bb_list; e != NULL; e = e->next)
	{
		uint32_t age = ymd_file_age(e->name + 3, now, NULL, NULL) / SECONDS_PER_DAY;
		asl_core_time_to_str(age, tstr, sizeof(tstr));
		debug_log(ASL_LEVEL_NOTICE, "  %s   %lu (age %s)\n", e->name, e->size, tstr, ((age / SECONDS_PER_DAY) > 0) ? " - expired" : "");
	}

	/* Delete/achive expired YMD files */
	debug_log(ASL_LEVEL_NOTICE, "Start YMD File Scan\n");

	e = ymd_list;
	while (e != NULL)
	{
		uid_t ymd_uid = -1;
		gid_t ymd_gid = -1;
		uint32_t age = ymd_file_age(e->name, now, &ymd_uid, &ymd_gid);

		if (age > ttl)
		{
			/* file has expired, archive it if required, then unlink it */
			if (dst->rotate_dir != NULL)
			{
				str = NULL;
				asprintf(&str, "%s/%s", dst->rotate_dir, e->name);
				if (str == NULL) return -1;

				filesystem_copy(dst, e->name, str, 0);
				free(str);
			}

			filesystem_unlink(e->name);
			store_size -= e->size;
			e->size = 0;
		}
		else if ((e->flags & NAME_LIST_FLAG_COMPRESSED) == 0)
		{
			uint32_t i, bit, keep_mask;
			mode_t ymd_mode = 0600;

			/* check if there are any per-level TTLs and filter the file if required */
			if (age > 0)
			{
				keep_mask = 0x000000ff;
				bit = 1;
				for (i = 0; i <= 7; i++)
				{
					if ((dst->ttl[i] > 0) && (age >= dst->ttl[i])) keep_mask &= ~bit;
					bit *= 2;
				}

				memset(&sb, 0, sizeof(struct stat));
				if (stat(e->name, &sb) == 0) ymd_mode = sb.st_mode & 0777;

				if (keep_mask != 0x000000ff) ymd_file_filter(e->name, dst->path, keep_mask, ymd_mode, ymd_uid, ymd_gid);
			}

			if ((age > since_midnight) && (dst->flags & MODULE_FLAG_COMPRESS))
			{
				char gzdst[MAXPATHLEN];

				snprintf(gzdst, sizeof(gzdst), "%s.gz", e->name);
				debug_log(ASL_LEVEL_NOTICE, "  compress %s ---> %s\n", e->name, gzdst);

				if (!dryrun)
				{
					int status = copy_compress_file(dst, e->name, gzdst);
					if (status == 0)
					{
						filesystem_unlink(e->name);
					}
					else
					{
						debug_log(ASL_LEVEL_ERR, "  FAILED status %d errno %d [%s] compress %s ---> %s\n", status, errno, strerror(errno), e->name, gzdst);
						return 0;
					}
				}
			}
		}

		e = e->next;
	}

	debug_log(ASL_LEVEL_NOTICE, "Finished YMD File Scan\n");

	/* Delete/achive expired YMD AUX directories */
	debug_log(ASL_LEVEL_NOTICE, "Start AUX Directory Scan\n");

	e = aux_list;
	while (e != NULL)
	{
		uint32_t age = ymd_file_age(e->name + 4, now, NULL, NULL);

		if (age > ttl)
		{
			if (dst->rotate_dir != NULL)
			{
				str = NULL;
				asprintf(&str, "%s/%s", dst->rotate_dir, e->name);
				if (str == NULL) return -1;

				filesystem_copy(dst, e->name, str, 0);
				free(str);
			}

			remove_directory(e->name);
			store_size -= e->size;
			e->size = 0;
		}

		e = e->next;
	}

	debug_log(ASL_LEVEL_NOTICE, "Finished AUX Directory Scan\n");

	/* Delete/achive expired BB.AUX directories */
	debug_log(ASL_LEVEL_NOTICE, "Start BB.AUX Directory Scan\n");

	e = bb_aux_list;
	while (e != NULL)
	{
		uint32_t age = ymd_file_age(e->name + 7, now, NULL, NULL);

		if (age > 0)
		{
			if (dst->rotate_dir != NULL)
			{
				str = NULL;
				asprintf(&str, "%s/%s", dst->rotate_dir, e->name);
				if (str == NULL) return -1;

				filesystem_copy(dst, e->name, str, 0);
				free(str);
			}

			remove_directory(e->name);
			store_size -= e->size;
			e->size = 0;
		}

		e = e->next;
	}

	debug_log(ASL_LEVEL_NOTICE, "Finished BB.AUX Directory Scan\n");

	/* Delete/achive expired BB files */
	debug_log(ASL_LEVEL_NOTICE, "Start BB Scan\n");

	e = bb_list;
	while (e != NULL)
	{
		uint32_t age = ymd_file_age(e->name + 3, now, NULL, NULL);

		if (age > 0)
		{
			if (dst->rotate_dir != NULL)
			{
				str = NULL;
				asprintf(&str, "%s/%s", dst->rotate_dir, e->name);
				if (str == NULL) return -1;

				/* syslog -x [str] -f [e->name] */
				filesystem_copy(dst, e->name, str, 0);
				free(str);
			}

			filesystem_unlink(e->name);
			store_size -= e->size;
			e->size = 0;
		}

		e = e->next;
	}

	debug_log(ASL_LEVEL_NOTICE, "Finished BB Scan\n");

	if (all_max > 0)
	{
		/* if data store is over max_size, delete/archive more YMD files */
		if (store_size > all_max) debug_log(ASL_LEVEL_NOTICE, "Additional YMD Scan\n");

		e = ymd_list;
		while ((e != NULL) && (store_size > all_max))
		{
			if (e->size != 0)
			{
				uint32_t age = ymd_file_age(e->name, now, NULL, NULL);
				if (age == 0)
				{
					/* do not touch active file YYYY.MM.DD.asl */
					e = e->next;
					continue;
				}

				if (dst->rotate_dir != NULL)
				{
					str = NULL;
					asprintf(&str, "%s/%s", dst->rotate_dir, e->name);
					if (str == NULL) return -1;

					/* syslog -x [str] -f [e->name] */
					filesystem_copy(dst, e->name, str, 0);
					free(str);
				}

				filesystem_unlink(e->name);
				store_size -= e->size;
				e->size = 0;
			}

			e = e->next;
		}

		/* if data store is over all_max, delete/archive more BB files */
		if (store_size > all_max) debug_log(ASL_LEVEL_NOTICE, "Additional BB Scan\n");

		e = bb_list;
		while ((e != NULL) && (store_size > all_max))
		{
			if (e->size != 0)
			{
				if (dst->rotate_dir != NULL)
				{
					str = NULL;
					asprintf(&str, "%s/%s", dst->rotate_dir, e->name);
					if (str == NULL) return -1;

					/* syslog -x [str] -f [e->name] */
					filesystem_copy(dst, e->name, str, 0);
					free(str);
				}

				filesystem_unlink(e->name);
				store_size -= e->size;
				e->size = 0;
			}

			e = e->next;
		}
	}

	free_name_list(ymd_list);
	free_name_list(bb_list);
	free_name_list(aux_list);
	free_name_list(bb_aux_list);

	debug_log(ASL_LEVEL_NOTICE, "Data Store Size = %lu\n", store_size);

	return 0;
}

/* move sequenced source files to dst dir, renaming as we go */
int
module_copy_rename(asl_out_dst_data_t *dst)
{
	asl_out_file_list_t *src_list, *dst_list, *f;
	char *dst_dir;
	char fpathsrc[MAXPATHLEN], fpathdst[MAXPATHLEN];
	uint32_t src_count, dst_count;
	int32_t x, moved;

	if (dst == NULL) return -1;
	if (dst->path == NULL) return -1;

	src_list = asl_list_src_files(dst);

	/*
	 * Note: the unmarked file (e.g. system.log) is included in src_list.
	 * If it is from a MODULE_FLAG_EXTERNAL dst and it is less than 24 hours old,
	 * we ignore it.  If it is not external, we also ignore it since syslogd will
	 * checkpoint it to create system.log.Tnnnnnnnnnn.
	 */
	if ((src_list != NULL) && (src_list->stamp == STAMP_STYLE_NULL))
	{
		bool ignore_it = false;

		if (dst->flags & MODULE_FLAG_EXTERNAL)
		{
			if ((time(NULL) - src_list->ftime) < SECONDS_PER_DAY)
			{
				debug_log(ASL_LEVEL_INFO, "    ignore src file %s since it is external and less than a day old\n", src_list->name);
				ignore_it = true;
			}
		}
		else
		{
			debug_log(ASL_LEVEL_INFO, "    ignore src file %s since it is internal and syslogd will checkpoint it when it needs to be renamed\n", src_list->name);
			ignore_it = true;
		}

		if (ignore_it)
		{
			asl_out_file_list_t *first = src_list;
			src_list = src_list->next;
			first->next = NULL;
			asl_out_file_list_free(first);
		}
	}

	if (src_list == NULL)
	{
		debug_log(ASL_LEVEL_INFO, "    no src files\n");
		return 0;
	}

	debug_log(ASL_LEVEL_INFO, "    src files\n");

	src_count = 0;
	for (f = src_list; f != NULL; f = f->next)
	{
		debug_log(ASL_LEVEL_INFO, "      %s\n", f->name);
		src_count++;
	}

	dst_list = asl_list_dst_files(dst);

	dst_dir = dst->rotate_dir;
	if (dst_dir == NULL) dst_dir = dst->dir;

	dst_count = 0;

	if (dst_list == NULL) debug_log(ASL_LEVEL_INFO, "    no dst files\n");
	else debug_log(ASL_LEVEL_INFO, "    dst files\n");

	for (f = dst_list; f != NULL; f = f->next)
	{
		debug_log(ASL_LEVEL_INFO, "      %s\n", f->name);
		dst_count++;
	}

	if (dst->style_flags & MODULE_NAME_STYLE_STAMP_SEQ)
	{
		for (f = dst_list; f != NULL; f = f->next)
		{
			int is_gz = 0;
			char *dot = strrchr(f->name, '.');
			if ((dot != NULL) && (!strcmp(dot, ".gz"))) is_gz = 1;

			snprintf(fpathsrc, sizeof(fpathsrc), "%s/%s", dst_dir, f->name);

			if (dst->style_flags & MODULE_NAME_STYLE_FORMAT_BS)
			{
				snprintf(fpathdst, sizeof(fpathdst), "%s/%s.%d%s", dst_dir, dst->base, f->seq+src_count, (is_gz == 1) ? ".gz" : "");
			}
			else if (dst->style_flags & MODULE_NAME_STYLE_FORMAT_BES)
			{
				snprintf(fpathdst, sizeof(fpathdst), "%s/%s.%s.%d%s", dst_dir, dst->base, dst->ext, f->seq+src_count, (is_gz == 1) ? ".gz" : "");
			}
			else if (dst->style_flags & MODULE_NAME_STYLE_FORMAT_BSE)
			{
				snprintf(fpathdst, sizeof(fpathdst), "%s/%s.%d.%s%s", dst_dir, dst->base, f->seq+src_count, dst->ext, (is_gz == 1) ? ".gz" : "");
			}

			filesystem_rename(fpathsrc, fpathdst);
		}

		for (f = src_list, x = 0; f != NULL; f = f->next, x++)
		{
			snprintf(fpathsrc, sizeof(fpathsrc), "%s/%s", dst->dir, f->name);

			if (dst->style_flags & MODULE_NAME_STYLE_FORMAT_BS)
			{
				snprintf(fpathdst, sizeof(fpathdst), "%s/%s.%d", dst_dir, dst->base, x);
			}
			else if (dst->style_flags & MODULE_NAME_STYLE_FORMAT_BES)
			{
				snprintf(fpathdst, sizeof(fpathdst), "%s/%s.%s.%d", dst_dir, dst->base, dst->ext, x);
			}
			else if (dst->style_flags & MODULE_NAME_STYLE_FORMAT_BSE)
			{
				snprintf(fpathdst, sizeof(fpathdst), "%s/%s.%d.%s", dst_dir, dst->base, x, dst->ext);
			}

			moved = filesystem_copy(dst, fpathsrc, fpathdst, dst->flags);
			if (moved != 0)
			{
				if (dst->flags & MODULE_FLAG_TRUNCATE)
				{
					filesystem_truncate(fpathsrc);
					filesystem_reset_ctime(fpathsrc);
				}
				else
				{
					filesystem_unlink(fpathsrc);
				}
			}
		}
	}
	else
	{
		for (f = src_list; f != NULL; f = f->next)
		{
			/* final / active base stamped file looks like a checkpointed file - ignore it */
			if ((dst->flags & MODULE_FLAG_BASESTAMP) && (f->next == NULL)) break;

			snprintf(fpathsrc, sizeof(fpathsrc), "%s/%s", dst->dir, f->name);

			/* MODULE_FLAG_EXTERNAL files are not decorated with a timestamp */
			if (dst->flags & MODULE_FLAG_EXTERNAL)
			{
				char tstamp[32];

				asl_make_timestamp(f->ftime, dst->style_flags, tstamp, sizeof(tstamp));

				if (dst->style_flags & MODULE_NAME_STYLE_FORMAT_BS)
				{
					snprintf(fpathdst, sizeof(fpathdst), "%s/%s.%s", dst_dir, dst->base, tstamp);
				}
				else if (dst->style_flags & MODULE_NAME_STYLE_FORMAT_BES)
				{
					snprintf(fpathdst, sizeof(fpathdst), "%s/%s.%s.%s", dst_dir, dst->base, dst->ext, tstamp);
				}
				else if (dst->style_flags & MODULE_NAME_STYLE_FORMAT_BSE)
				{
					snprintf(fpathdst, sizeof(fpathdst), "%s/%s.%s.%s", dst_dir, dst->base, tstamp, dst->ext);
				}

			}
			else
			{
				snprintf(fpathdst, sizeof(fpathdst), "%s/%s", dst_dir, f->name);
			}

			moved = filesystem_copy(dst, fpathsrc, fpathdst, dst->flags);
			if (moved != 0)
			{
				if (dst->flags & MODULE_FLAG_TRUNCATE) filesystem_truncate(fpathsrc);
				else filesystem_unlink(fpathsrc);
			}
		}
	}

	asl_out_file_list_free(src_list);
	asl_out_file_list_free(dst_list);

	return 0;
}

/* delete expired files */
int
module_expire(asl_out_dst_data_t *dst, asl_out_dst_data_t *opts)
{
	asl_out_file_list_t *dst_list, *f;
	char *base, *dst_dir, fpath[MAXPATHLEN];
	time_t now, ttl, age;

	if (dst == NULL) return -1;
	if (dst->path == NULL) return -1;
	if (dst->ttl[LEVEL_ALL] == 0) return 0;

	ttl = dst->ttl[LEVEL_ALL];
	if ((opts != NULL) && (opts->ttl[LEVEL_ALL] > 0)) ttl = opts->ttl[LEVEL_ALL];

	now = time(NULL);
	if (ttl > now) return 0;

	base = strrchr(dst->path, '/');
	if (base == NULL) return -1;

	dst_list = asl_list_dst_files(dst);

	*base = '\0';

	dst_dir = dst->rotate_dir;
	if (dst_dir == NULL) dst_dir = dst->dir;

	if (dst_list == NULL)
	{
		debug_log(ASL_LEVEL_INFO, "    no dst files\n");
	}
	else
	{
		debug_log(ASL_LEVEL_INFO, "    dst files\n");
		for (f = dst_list; f != NULL; f = f->next)
		{
			char tstr[150];
			age = now - f->ftime;

			asl_core_time_to_str(age, tstr, sizeof(tstr));
			debug_log(ASL_LEVEL_INFO, "      %s (age %s%s)\n", f->name, tstr, (age > ttl) ? " - expired" : "");
		}
	}

	for (f = dst_list; f != NULL; f = f->next)
	{
		age = now - f->ftime;
		if (age > ttl)
		{
			snprintf(fpath, sizeof(fpath), "%s/%s", dst_dir, f->name);
			filesystem_unlink(fpath);
		}
	}

	asl_out_file_list_free(dst_list);

	if (base != NULL) *base = '/';

	return 0;
}

/*
 * Check all_max size and delete files (oldest first) to stay within size limit.
 * If query is true, then just report total size.
 */
int
module_check_size(asl_out_dst_data_t *dst, asl_out_dst_data_t *opts, bool query, size_t *msize)
{
	asl_out_file_list_t *dst_list, *f, *dst_end;
	char *dst_dir, fpath[MAXPATHLEN];
	size_t total;

	size_t all_max = dst->all_max;
	if ((opts != NULL) && (opts->all_max > 0)) all_max = opts->all_max;

	if (dst == NULL) return -1;
	if (dst->path == NULL) return -1;

	if (all_max == 0) return 0;

	dst_list = asl_list_dst_files(dst);
	if (dst_list == NULL)
	{
		debug_log(ASL_LEVEL_INFO, "    no dst files\n");
		return 0;
	}

	dst_dir = dst->rotate_dir;
	if (dst_dir == NULL) dst_dir = dst->dir;

	debug_log(ASL_LEVEL_INFO, "    dst files\n");
	dst_end = dst_list;
	for (f = dst_list; f != NULL; f = f->next)
	{
		dst_end = f;
		debug_log(ASL_LEVEL_INFO, "      %s size %lu\n", f->name, f->size);
	}

	total = 0;
	for (f = dst_list; f != NULL; f = f->next) total += f->size;

	if (!query)
	{
		for (f = dst_list; (total > all_max) && (f != NULL); f = f->next)
		{
			snprintf(fpath, sizeof(fpath), "%s/%s", dst_dir, f->name);
			filesystem_unlink(fpath);
			total -= f->size;
		}
	}

	if (msize != NULL) *msize = total;

	asl_out_file_list_free(dst_list);

	return 0;
}

int
process_dst(asl_out_dst_data_t *dst, asl_out_dst_data_t *opts)
{
	uint32_t ttl = dst->ttl[LEVEL_ALL];
	if ((opts != NULL) && (opts->ttl[LEVEL_ALL] > 0)) ttl = opts->ttl[LEVEL_ALL];

	size_t all_max = dst->all_max;
	if ((opts != NULL) && (opts->all_max > 0)) all_max = opts->all_max;

	if (dst == NULL)
	{
		debug_log(ASL_LEVEL_NOTICE, "NULL dst data for output rule - skipped\n");
	}
	else if (dst->flags & MODULE_FLAG_ROTATE)
	{
		debug_log(ASL_LEVEL_NOTICE, "Checking file %s\n", dst->path);
		debug_log(ASL_LEVEL_NOTICE, "- Rename, move to destination directory, and compress as required\n");

		module_copy_rename(dst);

		if ((ttl > 0) && !(dst->flags & MODULE_FLAG_SIZE_ONLY))
		{
			char tstr[150];

			asl_core_time_to_str(ttl, tstr, sizeof(tstr));
			debug_log(ASL_LEVEL_NOTICE, "- Check for expired files - TTL = %s\n", tstr);
			module_expire(dst, opts);
		}

		if (all_max > 0)
		{
			debug_log(ASL_LEVEL_NOTICE, "- Check total storage used - MAX = %lu\n", all_max);
			module_check_size(dst, opts, false, NULL);
		}
	}
	else if ((dst->flags & MODULE_FLAG_TYPE_ASL_DIR) && (ttl > 0))
	{
		process_asl_data_store(dst, opts);
	}

	return 0;
}

int
process_module(asl_out_module_t *mod, asl_out_dst_data_t *opts)
{
	asl_out_rule_t *r;
	uint32_t flags = 0;

	if (mod == NULL) return -1;

	if (opts != NULL) flags = opts->flags;

	debug_log(ASL_LEVEL_NOTICE, "----------------------------------------\n");
	debug_log(ASL_LEVEL_NOTICE, "Processing module %s\n", (mod->name == NULL) ? "asl.conf" : mod->name);

	for (r = mod->ruleset; r != NULL; r = r->next)
	{
		if (r->action == ACTION_OUT_DEST)
		{
			if ((flags == 0) || ((flags & r->dst->flags) != 0)) process_dst(r->dst, opts);
		}
	}

	debug_log(ASL_LEVEL_NOTICE, "Finished processing module %s\n", (mod->name == NULL) ? "asl.conf" : mod->name);
	return 0;
}

asl_msg_list_t *
control_query(asl_msg_t *a)
{
	asl_msg_list_t *out;
	char *qstr, *str, *res;
	uint32_t len, reslen, status;
	uint64_t cmax, qmin;
	kern_return_t kstatus;
	caddr_t vmstr;

	if (asl_server_port == MACH_PORT_NULL)
	{
		kstatus = bootstrap_look_up2(bootstrap_port, ASL_SERVICE_NAME, &asl_server_port, 0, BOOTSTRAP_PRIVILEGED_SERVER);
		if (asl_server_port == MACH_PORT_NULL) return NULL;
	}

	qstr = asl_msg_to_string((asl_msg_t *)a, &len);

	str = NULL;
	if (qstr == NULL)
	{
		asprintf(&str, "1\nQ [= ASLOption control]\n");
	}
	else
	{
		asprintf(&str, "1\n%s [= ASLOption control]\n", qstr);
		free(qstr);
	}

	if (str == NULL) return NULL;

	/* length includes trailing nul */
	len = strlen(str) + 1;
	out = NULL;
	qmin = 0;
	cmax = 0;

	res = NULL;
	reslen = 0;
	status = ASL_STATUS_OK;

	kstatus = vm_allocate(mach_task_self(), (vm_address_t *)&vmstr, len, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_ASL));
	if (kstatus != KERN_SUCCESS) return NULL;

	memmove(vmstr, str, len);
	free(str);

	status = 0;
	kstatus = _asl_server_query_2(asl_server_port, vmstr, len, qmin, 1, 0, (caddr_t *)&res, &reslen, &cmax, (int *)&status);
	if (kstatus != KERN_SUCCESS) return NULL;

	if (res == NULL) return NULL;

	out = asl_msg_list_from_string(res);
	vm_deallocate(mach_task_self(), (vm_address_t)res, reslen);

	return out;
}

int
checkpoint(const char *name)
{
	/* send checkpoint message to syslogd */
	debug_log(ASL_LEVEL_NOTICE, "Checkpoint module %s\n", (name == NULL) ? "*" : name);
	if (dryrun) return 0;

	asl_msg_t *qmsg = asl_msg_new(ASL_TYPE_QUERY);
	char *tmp = NULL;
	asl_msg_list_t *res;

	asprintf(&tmp, "%s checkpoint", (name == NULL) ? "*" : name);
	asl_msg_set_key_val_op(qmsg, "action", tmp, ASL_QUERY_OP_EQUAL);
	free(tmp);

	res = control_query(qmsg);

	asl_msg_list_release(res);
	return 0;
}
