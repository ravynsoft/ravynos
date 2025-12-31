/*
 * Copyright (c) 2007-2011 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asl.h>
#include <asl_private.h>
#include <asl_core.h>
#include <asl_store.h>
#include <notify.h>

extern uint64_t asl_file_cursor(asl_file_t *s);
extern uint32_t asl_file_match_start(asl_file_t *s, uint64_t start_id, int32_t direction);
extern uint32_t asl_file_match_next(asl_file_t *s, asl_msg_list_t *qlist, asl_msg_t **msg, uint64_t *last_id, int32_t direction, int32_t ruid, int32_t rgid);
extern int asl_file_create(const char *path, uid_t uid, gid_t gid, mode_t mode);

#define SECONDS_PER_DAY 86400

/* 
 * The ASL Store is organized as a set of files in a common directory.
 * Files are prefixed by the date (YYYY.MM.DD) of their contents.
 *
 * Messages with no access controls are saved in YYYY.MM.DD.asl
 * Messages with access limited to UID uuu are saved in YYYY.MM.DD.Uuuu.asl
 * Messages with access limited to GID ggg are saved in YYYY.MM.DD.Gggg.asl
 * Messages with access limited to UID uuu and GID ggg are saved in YYYY.MM.DD.Uuuu.Gggg.asl
 *
 * Messages that have a value for ASLExpireTime are saved in BB.YYYY.MM.DD.asl
 * where the timestamp is the "Best Before" date of the file.  Access controls
 * are implemented as above with Uuuu and Gggg in the file name.  Note that the
 * Best Before files are for the last day of the month, so a single file contains
 * messages that expire in that month.
 *
 * An external tool runs daily and deletes "old" files.
 */

static time_t
_asl_start_today()
{
	time_t now;
	struct tm ctm;

	memset(&ctm, 0, sizeof(struct tm));
	now = time(NULL);

	if (localtime_r((const time_t *)&now, &ctm) == NULL) return 0;

	ctm.tm_sec = 0;
	ctm.tm_min = 0;
	ctm.tm_hour = 0;

	return mktime(&ctm);
}

/*
 * The base directory contains a data file which stores
 * the last record ID.
 *
 * | MAX_ID (uint64_t) |
 *
 */
ASL_STATUS
asl_store_open_write(const char *basedir, asl_store_t **s)
{
	asl_store_t *out;
	struct stat sb;
	uint32_t i, flags;
	char path[MAXPATHLEN];
	FILE *sd;
	uint64_t last_id;
	time_t start;

	if (s == NULL) return ASL_STATUS_INVALID_ARG;

	start = _asl_start_today();
	if (start == 0) return ASL_STATUS_FAILED;

	if (basedir == NULL) basedir = PATH_ASL_STORE;

	memset(&sb, 0, sizeof(struct stat));
	if (stat(basedir, &sb) != 0)
	{
		if (errno != ENOENT) return ASL_STATUS_INVALID_STORE;
		if (mkdir(basedir, 0755) != 0) return ASL_STATUS_WRITE_FAILED;
	}
	else
	{
		if (!S_ISDIR(sb.st_mode)) return ASL_STATUS_INVALID_STORE;
	}
	
	snprintf(path, sizeof(path), "%s/%s", basedir, FILE_ASL_STORE_DATA);

	sd = NULL;

	memset(&sb, 0, sizeof(struct stat));
	if (stat(path, &sb) != 0)
	{
		if (errno != ENOENT) return ASL_STATUS_FAILED;

		sd = fopen(path, "w+");

		if (sd == NULL) return ASL_STATUS_FAILED;

		last_id = 0;

		/* Create new StoreData file (8 bytes ID + 4 bytes flags) */

		if (fwrite(&last_id, sizeof(uint64_t), 1, sd) != 1)
		{
			fclose(sd);
			return ASL_STATUS_WRITE_FAILED;
		}

		flags = 0;
		if (fwrite(&flags, sizeof(uint32_t), 1, sd) != 1)
		{
			fclose(sd);
			return ASL_STATUS_WRITE_FAILED;
		}

		/* flush data */
		fflush(sd);
	}
	else
	{
		sd = fopen(path, "r+");

		if (sd == NULL) return ASL_STATUS_FAILED;
		if (fread(&last_id, sizeof(uint64_t), 1, sd) != 1)
		{
			fclose(sd);
			return ASL_STATUS_READ_FAILED;
		}

		last_id = asl_core_ntohq(last_id);
	}

	out = (asl_store_t *)calloc(1, sizeof(asl_store_t));
	if (out == NULL)
	{
		fclose(sd);
		return ASL_STATUS_NO_MEMORY;
	}

	out->asl_type = ASL_TYPE_STORE;
	out->refcount = 1;

	if (basedir == NULL) out->base_dir = strdup(PATH_ASL_STORE);
	else out->base_dir = strdup(basedir);

	if (out->base_dir == NULL)
	{
		fclose(sd);
		free(out);
		return ASL_STATUS_NO_MEMORY;
	}

	out->start_today = start;
	out->start_tomorrow = out->start_today + SECONDS_PER_DAY;
	out->storedata = sd;
	out->next_id = last_id + 1;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		memset(&out->file_cache[i], 0, sizeof(asl_cached_file_t));
		out->file_cache[i].u = -1;
		out->file_cache[i].g = -1;
	}

	*s = out;
	return ASL_STATUS_OK;
}

uint32_t
asl_store_set_flags(asl_store_t *s, uint32_t flags)
{
	if (s == NULL) return 0;
	uint32_t oldflags = s->flags;
	s->flags = flags;
	return oldflags;
}

ASL_STATUS
asl_store_statistics(asl_store_t *s, asl_msg_t **msg)
{
	asl_msg_t *out;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;

	out = asl_msg_new(ASL_TYPE_MSG);
	if (out == NULL) return ASL_STATUS_NO_MEMORY;

	/* does nothing for now */

	*msg = out;
	return ASL_STATUS_OK;
}

uint32_t
asl_store_open_read(const char *basedir, asl_store_t **s)
{
	asl_store_t *out;
	struct stat sb;

	if (s == NULL) return ASL_STATUS_INVALID_ARG;

	if (basedir == NULL) basedir = PATH_ASL_STORE;

	memset(&sb, 0, sizeof(struct stat));
	if (stat(basedir, &sb) != 0) return ASL_STATUS_INVALID_STORE;
	if (!S_ISDIR(sb.st_mode)) return ASL_STATUS_INVALID_STORE;

	out = (asl_store_t *)calloc(1, sizeof(asl_store_t));
	if (out == NULL) return ASL_STATUS_NO_MEMORY;

	out->asl_type = ASL_TYPE_STORE;
	out->refcount = 1;

	if (basedir == NULL) out->base_dir = strdup(PATH_ASL_STORE);
	else out->base_dir = strdup(basedir);

	if (out->base_dir == NULL)
	{
		free(out);
		return ASL_STATUS_NO_MEMORY;
	}

	*s = out;
	return ASL_STATUS_OK;
}

uint32_t
asl_store_max_file_size(asl_store_t *s, size_t max)
{
	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	s->max_file_size = max;
	return ASL_STATUS_OK;
}

__private_extern__ void
asl_store_file_closeall(asl_store_t *s)
{
	uint32_t i;

	if (s == NULL) return;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if (s->file_cache[i].f != NULL) asl_file_close(s->file_cache[i].f);
		s->file_cache[i].f = NULL;
		if (s->file_cache[i].path != NULL) free(s->file_cache[i].path);
		s->file_cache[i].path = NULL;
		s->file_cache[i].u = -1;
		s->file_cache[i].g = -1;
		s->file_cache[i].bb = 0;
		s->file_cache[i].ts = 0;
	}
}

asl_store_t *
asl_store_retain(asl_store_t *s)
{
	if (s == NULL) return NULL;
	asl_retain((asl_object_t)s);
	return s;
}

void
asl_store_release(asl_store_t *s)
{
	if (s == NULL) return;
	asl_release((asl_object_t)s);
}

ASL_STATUS
asl_store_close(asl_store_t *s)
{
	if (s == NULL) return ASL_STATUS_OK;
	asl_release((asl_object_t)s);
	return ASL_STATUS_OK;
}

static void
_asl_store_free_internal(asl_store_t *s)
{
	if (s == NULL) return;

	if (s->base_dir != NULL) free(s->base_dir);
	s->base_dir = NULL;
	asl_store_file_closeall(s);
	if (s->storedata != NULL) fclose(s->storedata);

	free(s);
}

/*
 * Sweep the file cache.
 * Close any files that have not been used in the last FILE_CACHE_TTL seconds.
 * Returns least recently used or unused cache slot.
 */
static uint32_t
asl_store_file_cache_lru(asl_store_t *s, time_t now, uint32_t ignorex)
{
	time_t min;
	uint32_t i, x;

	if (s == NULL) return 0;

	x = 0;
	min = now - FILE_CACHE_TTL;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if ((i != ignorex) && (s->file_cache[i].ts < min))
		{
			asl_file_close(s->file_cache[i].f);
			s->file_cache[i].f = NULL;
			if (s->file_cache[i].path != NULL) free(s->file_cache[i].path);
			s->file_cache[i].path = NULL;
			s->file_cache[i].u = -1;
			s->file_cache[i].g = -1;
			s->file_cache[i].bb = 0;
			s->file_cache[i].ts = 0;
		}

		if (s->file_cache[i].ts < s->file_cache[x].ts) x = i;
	}

	return x;
}

ASL_STATUS
asl_store_sweep_file_cache(asl_store_t *s)
{
	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	asl_store_file_cache_lru(s, time(NULL), FILE_CACHE_SIZE);
	return ASL_STATUS_OK;
}

static char *
asl_store_make_ug_path(const char *dir, const char *base, const char *ext, uid_t ruid, gid_t rgid, uid_t *u, gid_t *g, mode_t *m)
{
	char *path = NULL;

	*u = 0;
	*g = 0;
	*m = 0644;

	if (ruid == -1)
	{
		if (rgid == -1)
		{
			if (ext == NULL) asprintf(&path, "%s/%s", dir, base);
			else asprintf(&path, "%s/%s.%s", dir, base, ext);
		}
		else
		{
			*g = rgid;
			*m = 0600;
			if (ext == NULL) asprintf(&path, "%s/%s.G%d", dir, base, *g);
			else asprintf(&path, "%s/%s.G%d.%s", dir, base, *g, ext);
		}
	}
	else
	{
		*u = ruid;
		if (rgid == -1)
		{
			*m = 0600;
			if (ext == NULL) asprintf(&path, "%s/%s.U%d", dir, base, *u);
			else asprintf(&path, "%s/%s.U%d.%s", dir, base, *u, ext);
		}
		else
		{
			*g = rgid;
			*m = 0600;
			if (ext == NULL) asprintf(&path, "%s/%s.U%d.G%d", dir, base, *u, *g);
			else asprintf(&path, "%s/%s.U%d.G%u.%s", dir, base, *u, *g, ext);
		}
	}

	return path;
}

static ASL_STATUS
asl_store_file_open_write(asl_store_t *s, char *tstring, int32_t ruid, int32_t rgid, time_t bb, asl_file_t **f, time_t now, uint32_t check_cache)
{
	char *path;
	mode_t m;
	int32_t i, x;
	uid_t u;
	gid_t g;
	uint32_t status;
	asl_file_t *out;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	/* see if the file is already open and in the cache */
	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if ((s->file_cache[i].u == ruid) && (s->file_cache[i].g == rgid) && (s->file_cache[i].bb == bb) && (s->file_cache[i].f != NULL))
		{
			s->file_cache[i].ts = now;
			*f = s->file_cache[i].f;
			if (check_cache == 1) asl_store_file_cache_lru(s, now, i);
			return ASL_STATUS_OK;
		}
	}

	u = 0;
	g = 0;
	m = 0644;
	path = asl_store_make_ug_path(s->base_dir, tstring, "asl", (uid_t)ruid, (gid_t)rgid, &u, &g, &m);
	if (path == NULL) return ASL_STATUS_NO_MEMORY;

	out = NULL;
	status = asl_file_open_write(path, m, u, g, &out);
	if (status != ASL_STATUS_OK)
	{
		free(path);
		return status;
	}

	x = asl_store_file_cache_lru(s, now, FILE_CACHE_SIZE);
	if (s->file_cache[x].f != NULL) asl_file_close(s->file_cache[x].f);
	if (s->file_cache[x].path != NULL) free(s->file_cache[x].path);

	s->file_cache[x].f = out;
	s->file_cache[x].path = path;
	s->file_cache[x].u = ruid;
	s->file_cache[x].g = rgid;
	s->file_cache[x].bb = bb;
	s->file_cache[x].ts = time(NULL);

	*f = out;

	return ASL_STATUS_OK;
}

__private_extern__ char *
asl_store_file_path(asl_store_t *s, asl_file_t *f)
{
	uint32_t i;

	if (s == NULL) return NULL;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if (s->file_cache[i].f == f)
		{
			if (s->file_cache[i].path == NULL) return NULL;
			return strdup(s->file_cache[i].path);
		}
	}

	return NULL;
}

__private_extern__ void
asl_store_file_close(asl_store_t *s, asl_file_t *f)
{
	uint32_t i;

	if (s == NULL) return;
	if (f == NULL) return;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if (s->file_cache[i].f == f)
		{
			asl_file_close(s->file_cache[i].f);
			s->file_cache[i].f = NULL;
			if (s->file_cache[i].path != NULL) free(s->file_cache[i].path);
			s->file_cache[i].path = NULL;
			s->file_cache[i].u = -1;
			s->file_cache[i].g = -1;
			s->file_cache[i].bb = 0;
			s->file_cache[i].ts = 0;
			return;
		}
	}
}

ASL_STATUS
asl_store_save(asl_store_t *s, asl_msg_t *msg)
{
	struct tm ctm;
	time_t msg_time, now, bb;
	char *path;
	char tstring[128], tmp_path[MAXPATHLEN];
	const char *val;
	uid_t ruid;
	gid_t rgid;
	asl_file_t *f;
	uint32_t status, check_cache, trigger_aslmanager, len;
	uint64_t xid, ftime;
	size_t fsize;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;

	now = time(NULL);

	check_cache = 0;
	if ((s->last_write + FILE_CACHE_TTL) <= now) check_cache = 1;

	trigger_aslmanager = 0;

	msg_time = 0;
	val = NULL;

	if (asl_msg_lookup(msg, ASL_KEY_TIME, &val, NULL) != 0) msg_time = now;
	else if (val == NULL) msg_time = now;
	else msg_time = asl_core_parse_time(val, NULL);

	if (msg_time >= s->start_tomorrow)
	{
		if (now >= s->start_tomorrow)
		{
			/* new day begins */
			check_cache = 0;
			asl_store_file_closeall(s);

			/*
			 * _asl_start_today should never fail, but if it does,
			 * just push forward one day.  That will probably be correct, and if
			 * it isn't, the next message that gets saved will push it ahead again
			 * until we get to the right date.
			 */
			s->start_today = _asl_start_today();
			if (s->start_today == 0) s->start_today = s->start_tomorrow;

			s->start_tomorrow = s->start_today + SECONDS_PER_DAY;
		}
	}

	ruid = -1;
	rgid = -1;
	if ((s->flags & ASL_STORE_FLAG_NO_ACLS) == 0)
	{
		val = NULL;
		if ((asl_msg_lookup(msg, ASL_KEY_READ_UID, &val, NULL) == 0) && (val != NULL)) ruid = atoi(val);

		val = NULL;
		if ((asl_msg_lookup(msg, ASL_KEY_READ_GID, &val, NULL) == 0) && (val != NULL)) rgid = atoi(val);
	}

	bb = 0;
	if ((s->flags & ASL_STORE_FLAG_NO_TTL) == 0)
	{
		val = NULL;
		if ((asl_msg_lookup(msg, ASL_KEY_EXPIRE_TIME, &val, NULL) == 0) && (val != NULL))
		{
			bb = 1;
			msg_time = asl_core_parse_time(val, NULL);
		}
	}

	if (fseeko(s->storedata, 0, SEEK_SET) != 0) return ASL_STATUS_WRITE_FAILED;

	xid = asl_core_htonq(s->next_id);
	if (fwrite(&xid, sizeof(uint64_t), 1, s->storedata) != 1) return ASL_STATUS_WRITE_FAILED;

	/* flush data */
	fflush(s->storedata);

	xid = s->next_id;
	s->next_id++;

	s->last_write = now;

	if (localtime_r((const time_t *)&msg_time, &ctm) == NULL) return ASL_STATUS_FAILED;

	if (bb == 1)
	{
		/*
		 * This supports 12 monthly "Best Before" buckets.
		 * We advance the actual expiry time to day zero of the following month.
		 * mktime() is clever enough to know that you actually mean the last day
		 * of the previous month.  What we get back from localtime is the last
		 * day of the month in which the message expires, which we use in the name.
		 */
		ctm.tm_sec = 0;
		ctm.tm_min = 0;
		ctm.tm_hour = 0;
		ctm.tm_mday = 0;
		ctm.tm_mon += 1;

		bb = mktime(&ctm);

		if (localtime_r((const time_t *)&bb, &ctm) == NULL) return ASL_STATUS_FAILED;
		snprintf(tstring, sizeof(tstring), "BB.%d.%02d.%02d", ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
	}
	else
	{
		snprintf(tstring, sizeof(tstring), "%d.%02d.%02d", ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
	}

	status = asl_store_file_open_write(s, tstring, ruid, rgid, bb, &f, now, check_cache);
	if (status != ASL_STATUS_OK) return status;

	status = asl_file_save(f, msg, &xid);
	if (status != ASL_STATUS_OK) return status;

	fsize = asl_file_size(f);
	ftime = asl_file_ctime(f);

	/* if file is larger than max_file_size, rename it and trigger aslmanager */
	if ((s->max_file_size != 0) && (fsize > s->max_file_size))
	{
		trigger_aslmanager = 1;
		status = ASL_STATUS_OK;

		path = asl_store_file_path(s, f);

		asl_store_file_close(s, f);

		if (path != NULL)
		{
			len = strlen(path);
			if ((len >= 4) && (!strcmp(path + len - 4, ".asl")))
			{
				/* rename xxxxxxx.asl to xxxxxxx.timestamp.asl */
				char scratch[MAXPATHLEN];
				snprintf(scratch, sizeof(scratch), "%s", path);
				scratch[len - 4] = '\0';
				snprintf(tmp_path, sizeof(tmp_path), "%s.%llu.asl", scratch, ftime);
			}
			else
			{
				/* append timestamp */
				snprintf(tmp_path, sizeof(tmp_path), "%s.%llu", path, ftime);
			}

			if (rename(path, tmp_path) != 0) status = ASL_STATUS_FAILED;

			free(path);
		}
	}

	if (trigger_aslmanager != 0) asl_trigger_aslmanager();

	return status;
}

static ASL_STATUS
asl_store_mkdir(asl_store_t *s, const char *dir, mode_t m)
{
	char tstring[MAXPATHLEN];
	int status;
	struct stat sb;

	snprintf(tstring, sizeof(tstring), "%s/%s", s->base_dir, dir);

	memset(&sb, 0, sizeof(struct stat));
	status = stat(tstring, &sb);

	if (status == 0)
	{
		/* must be a directory */
		if (!S_ISDIR(sb.st_mode)) return ASL_STATUS_INVALID_STORE;
	}
	else
	{
		if (errno == ENOENT)
		{
			/* doesn't exist - create it */
			if (mkdir(tstring, m) != 0) return ASL_STATUS_WRITE_FAILED;
		}
		else
		{
			/* stat failed for some other reason */
			return ASL_STATUS_FAILED;
		}
	}

	return ASL_STATUS_OK;
}

ASL_STATUS
asl_store_open_aux(asl_store_t *s, asl_msg_t *msg, int *out_fd, char **url)
{
	struct tm ctm;
	time_t msg_time, bb;
	char *path;
	char tstring[128], dir[128];
	const char *val;
	uid_t ruid, u;
	gid_t rgid, g;
	mode_t m;
	uint32_t status;
	uint64_t fid;
	int fd;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;
	if (out_fd == NULL) return ASL_STATUS_INVALID_ARG;
	if (url == NULL) return ASL_STATUS_INVALID_ARG;

	msg_time = time(NULL);

	ruid = -1;
	rgid = -1;
	if ((s->flags & ASL_STORE_FLAG_NO_ACLS) == 0)
	{
		val = NULL;
		if ((asl_msg_lookup(msg, ASL_KEY_READ_UID, &val, NULL) == 0) && (val != NULL)) ruid = atoi(val);

		val = NULL;
		if ((asl_msg_lookup(msg, ASL_KEY_READ_GID, &val, NULL) == 0) && (val != NULL)) rgid = atoi(val);
	}

	bb = 0;
	if ((s->flags & ASL_STORE_FLAG_NO_TTL) == 0)
	{
		val = NULL;
		if ((asl_msg_lookup(msg, ASL_KEY_EXPIRE_TIME, &val, NULL) == 0) && (val != NULL))
		{
			bb = 1;
			msg_time = asl_core_parse_time(val, NULL);
		}
	}

	if (localtime_r((const time_t *)&msg_time, &ctm) == NULL) return ASL_STATUS_FAILED;

	if (bb == 1)
	{
		/*
		 * This supports 12 monthly "Best Before" buckets.
		 * We advance the actual expiry time to day zero of the following month.
		 * mktime() is clever enough to know that you actually mean the last day
		 * of the previous month.  What we get back from localtime is the last
		 * day of the month in which the message expires, which we use in the name.
		 */
		ctm.tm_sec = 0;
		ctm.tm_min = 0;
		ctm.tm_hour = 0;
		ctm.tm_mday = 0;
		ctm.tm_mon += 1;

		bb = mktime(&ctm);

		if (localtime_r((const time_t *)&bb, &ctm) == NULL) return ASL_STATUS_FAILED;
		snprintf(dir, sizeof(dir), "BB.AUX.%d.%02d.%02d", ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
	}
	else
	{
		snprintf(dir, sizeof(dir), "AUX.%d.%02d.%02d", ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
	}

	status = asl_store_mkdir(s, dir, 0755);
	if (status != ASL_STATUS_OK) return status;

	fid = s->next_id;
	s->next_id++;

	snprintf(tstring, sizeof(tstring), "%s/%llu", dir, fid);

	u = 0;
	g = 0;
	m = 0644;
	path = asl_store_make_ug_path(s->base_dir, tstring, NULL, ruid, rgid, &u, &g, &m);
	if (path == NULL) return ASL_STATUS_NO_MEMORY;

	fd = asl_file_create(path, u, g, m);
	if (fd < 0)
	{
		free(path);
		*out_fd = -1;
		return ASL_STATUS_WRITE_FAILED;
	}

	/* URL is file://<path> */
	*url = NULL;
	asprintf(url, "file://%s", path);
	free(path);

	*out_fd = fd;

	return status;
}

asl_msg_list_t *
asl_store_match(asl_store_t *s, asl_msg_list_t *qlist, uint64_t *last_id, uint64_t start_id, uint32_t count, uint32_t duration, int32_t direction)
{
	DIR *dp;
	struct dirent *dent;
	uint32_t status;
	asl_file_t *f;
	char path[MAXPATHLEN];
	asl_file_list_t *files;
	asl_msg_list_t *res;

	if (s == NULL) return NULL;

	files = NULL;

	/*
	 * Open all readable files
	 */
	dp = opendir(s->base_dir);
	if (dp == NULL) return NULL;

	while ((dent = readdir(dp)) != NULL)
	{
		if (dent->d_name[0] == '.') continue;

		snprintf(path, sizeof(path), "%s/%s", s->base_dir, dent->d_name);

		/* NB asl_file_open_read will fail if path is NULL, if the file is not an ASL store file, or if it isn't readable */
		status = asl_file_open_read(path, &f);
		if ((status != ASL_STATUS_OK) || (f == NULL)) continue;

		files = asl_file_list_add(files, f);
	}

	closedir(dp);

	res = asl_file_list_match(files, qlist, last_id, start_id, count, duration, direction);
	asl_file_list_close(files);
	return res;
}

/* 
 * PRIVATE FOR DEV TOOLS SUPPORT
 * DO NOT USE THIS INTERFACE OTHERWISE
 *
 * This is only called by a client that compiled with a 10.9 SDK, but is running
 * with an new 10.10 libasl.
 *
 * Only searches the ASL database, so the store (first parameter) is ignored.
 *
 * The query and result are old-style message lists.
 *
 */
typedef struct
{
	uint32_t count;
	uint32_t curr;
	void **msg;
} asl_msg_list_v1_t;

ASL_STATUS
asl_store_match_timeout(void *ignored, void *query_v1, void **result_v1, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction, uint32_t usec)
{
	asl_store_t *asldb = NULL;
	asl_msg_list_v1_t *listv1;
	asl_msg_list_t *qlist = NULL;
	uint32_t status, n;

	if (result_v1 == NULL) return ASL_STATUS_FAILED;
	*result_v1 = NULL;

	status = asl_store_open_read(NULL, &asldb);
	if (status != ASL_STATUS_OK) return status;

	/* convert query_v1 into an asl_msg_list_t */
	listv1 = (asl_msg_list_v1_t *)query_v1;
	if (listv1 != NULL)
	{
		if (listv1->count > 0) qlist = (asl_msg_list_t *)asl_new(ASL_TYPE_LIST);

		for (listv1->curr = 0; listv1->curr < listv1->count; listv1->curr++)
		{
			asl_append((asl_object_t)qlist, (asl_object_t)listv1->msg[listv1->curr]);
		}
	}

	asl_msg_list_t *result = asl_store_match(asldb, qlist, last_id, start_id, count, usec, direction);
	asl_release((asl_object_t)asldb);
	asl_release((asl_object_t)qlist);

	if (result == NULL) return ASL_STATUS_OK;

	n = asl_count((asl_object_t)result);
	if (n == 0)
	{
		asl_release((asl_object_t)result);
		return ASL_STATUS_OK;
	}

	listv1 = (asl_msg_list_v1_t *)calloc(1, sizeof(asl_msg_list_v1_t));
	if (listv1 == NULL)
	{
		asl_release((asl_object_t)result);
		return ASL_STATUS_NO_MEMORY;
	}

	listv1->count = n;
	listv1->msg = (void **)calloc(listv1->count, sizeof(void *));
	if (listv1 == NULL)
	{
		free(listv1);
		asl_release((asl_object_t)result);
		return ASL_STATUS_NO_MEMORY;
	}

	for (listv1->curr = 0; listv1->curr < listv1->count; listv1->curr++)
	{
		listv1->msg[listv1->curr] = asl_retain(asl_next((asl_object_t)result));
	}

	listv1->curr = 0;
	*result_v1 = listv1;

	asl_release((asl_object_t)result);
	return ASL_STATUS_OK;
}

ASL_STATUS
asl_store_match_start(asl_store_t *s, uint64_t start_id, int32_t direction)
{
	DIR *dp;
	struct dirent *dent;
	uint32_t status;
	asl_file_t *f;
	char path[MAXPATHLEN];
	asl_file_list_t *files;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	if (s->work != NULL) asl_file_list_match_end(s->work);
	s->work = NULL;

	files = NULL;

	/*
	 * Open all readable files
	 */
	dp = opendir(s->base_dir);
	if (dp == NULL) return ASL_STATUS_READ_FAILED;

	while ((dent = readdir(dp)) != NULL)
	{
		if (dent->d_name[0] == '.') continue;

		snprintf(path, sizeof(path), "%s/%s", s->base_dir, dent->d_name);

		/*
		 * NB asl_file_open_read will fail if path is NULL,
		 * if it is not an ASL store file, or if it isn't readable.
		 * We expect that.
		 */
		status = asl_file_open_read(path, &f);
		if ((status != ASL_STATUS_OK) || (f == NULL)) continue;

		files = asl_file_list_add(files, f);
	}

	closedir(dp);

	s->work = asl_file_list_match_start(files, start_id, direction);
	if (s->work == NULL) return ASL_STATUS_FAILED;

	return ASL_STATUS_OK;
}

ASL_STATUS
asl_store_match_next(asl_store_t *s, asl_msg_list_t *qlist, asl_msg_list_t **res, uint32_t count)
{
	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->work == NULL) return ASL_STATUS_OK;

	return asl_file_list_match_next(s->work, qlist, res, count);
}

#pragma mark -
#pragma mark asl_object support

static void
_jump_dealloc(asl_object_private_t *obj)
{
	_asl_store_free_internal((asl_store_t *)obj);
}

static asl_object_private_t *
_jump_next(asl_object_private_t *obj)
{
	asl_store_t *s = (asl_store_t *)obj;
	asl_msg_list_t *list;
	asl_msg_t *out = NULL;
	uint64_t last = 0;

	if (s == NULL) return NULL;
	if (s->curr == SIZE_MAX) return NULL;

	s->curr++;
	list = asl_store_match(s, NULL, &last, s->curr, 1, 0, 1);
	if (list == NULL)
	{
		s->curr = SIZE_MAX;
		return NULL;
	}

	s->curr = last;
	out = asl_msg_list_get_index(list, 0);
	asl_msg_list_release(list);

	return (asl_object_private_t *)out;
}

static asl_object_private_t *
_jump_prev(asl_object_private_t *obj)
{
	asl_store_t *s = (asl_store_t *)obj;
	asl_msg_list_t *list;
	asl_msg_t *out = NULL;
	uint64_t last = 0;

	if (s == NULL) return NULL;
	if (s->curr == 0) return NULL;

	s->curr--;
	if (s->curr == 0) return NULL;

	list = asl_store_match(s, NULL, &last, s->curr, 1, 0, -1);
	if (list == NULL)
	{
		s->curr = 0;
		return NULL;
	}

	s->curr = last;
	out = asl_msg_list_get_index(list, 0);
	asl_msg_list_release(list);

	return (asl_object_private_t *)out;
}

static void
_jump_set_iteration_index(asl_object_private_t *obj, size_t n)
{
	asl_store_t *s = (asl_store_t *)obj;
	if (s == NULL) return;

	s->curr = n;
}

static void
_jump_append(asl_object_private_t *obj, asl_object_private_t *newobj, void *addr)
{
	asl_store_t *s = (asl_store_t *)obj;
	int type = asl_get_type((asl_object_t)newobj);
	if (s == NULL) return;
	if (s->flags & ASL_FILE_FLAG_READ) return;

	if (type == ASL_TYPE_LIST)
	{
		asl_msg_t *msg;
		asl_msg_list_reset_iteration((asl_msg_list_t *)newobj, 0);
		while (NULL != (msg = asl_msg_list_next((asl_msg_list_t *)newobj)))
		{
			if (asl_store_save(s, msg) != ASL_STATUS_OK) return;
		}
	}
	else if ((type == ASL_TYPE_MSG) || (type == ASL_TYPE_QUERY))
	{
		asl_store_save(s, (asl_msg_t *)newobj);
	}
}

static asl_object_private_t *
_jump_search(asl_object_private_t *obj, asl_object_private_t *query)
{
	asl_store_t *s = (asl_store_t *)obj;
	int type = asl_get_type((asl_object_t)query);
	asl_msg_list_t *out = NULL;
	asl_msg_list_t *ql = NULL;
	uint64_t last;

	if (query == NULL)
	{
		out = asl_store_match(s, NULL, &last, 0, 0, 0, 1);
	}
	else if (type == ASL_TYPE_LIST)
	{
		out = asl_store_match(s, (asl_msg_list_t *)query, &last, 0, 0, 0, 1);
	}
	else if ((type == ASL_TYPE_MSG) || (type == ASL_TYPE_QUERY))
	{
		ql = asl_msg_list_new();
		asl_msg_list_append(ql, query);

		out = asl_store_match(s, ql, &last, 0, 0, 0, 1);
		asl_msg_list_release(ql);
	}

	return (asl_object_private_t *)out;
}

static asl_object_private_t *
_jump_match(asl_object_private_t *obj, asl_object_private_t *qlist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t dir)
{
	uint64_t x;
	asl_msg_list_t *out;

	out = asl_store_match((asl_store_t *)obj, (asl_msg_list_t *)qlist, &x, start, count, duration, dir);
	*last = x;
	return (asl_object_private_t *)out;
}

__private_extern__ const asl_jump_table_t *
asl_store_jump_table()
{
	static const asl_jump_table_t jump =
	{
		.alloc = NULL,
		.dealloc = &_jump_dealloc,
		.set_key_val_op = NULL,
		.unset_key = NULL,
		.get_val_op_for_key = NULL,
		.get_key_val_op_at_index = NULL,
		.count = NULL,
		.next = &_jump_next,
		.prev = &_jump_prev,
		.get_object_at_index = NULL,
		.set_iteration_index = &_jump_set_iteration_index,
		.remove_object_at_index = NULL,
		.append = &_jump_append,
		.prepend = NULL,
		.search = &_jump_search,
		.match = &_jump_match
	};

	return &jump;
}
