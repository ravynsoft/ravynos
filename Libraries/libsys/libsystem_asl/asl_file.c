/*
 * Copyright (c) 2007-2013 Apple Inc. All rights reserved.
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

#include <asl_core.h>
#include <asl_msg.h>
#include <asl_file.h>
#include <asl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/acl.h>
#include <membership.h>
#include <time.h>
#include <sys/time.h>
#include <asl_private.h>
#include <asl_legacy1.h>
#include <TargetConditionals.h>

#define forever for(;;)

/*
 * MSG and STR records have (at least) a type (uint16_t) and a length (uint32_t)
 * type and level are both 16 bit fields so that alignment isn't a pain.
 */
#define RECORD_COMMON_LEN 6
#define RECORD_TYPE_LEN 2
#define BUFFER_OFFSET_KVCOUNT 56

#define SCRATCH_BUFFER_SIZE (MSG_RECORD_FIXED_LENGTH + (20 * sizeof(uint64_t)))

typedef struct
{
	uint64_t next;
	uint64_t mid;
	uint64_t time;
	uint32_t nano;
	uint16_t level;
	uint16_t flags;
	uint32_t pid;
	uint32_t uid;
	uint32_t gid;
	uint32_t ruid;
	uint32_t rgid;
	uint32_t refpid;
	uint32_t kvcount;
	uint64_t host;
	uint64_t sender;
	uint64_t facility;
	uint64_t message;
	uint64_t refproc;
	uint64_t session;
	uint64_t prev;
} file_record_t;

typedef struct
{
	asl_file_list_t *list;
	int dir;
} asl_file_match_token_t;

static uint16_t
_asl_get_16(char *h)
{
	uint16_t x;

	memcpy(&x, h, 2);
	return ntohs(x);
}

static void
_asl_put_16(uint16_t i, char *h)
{
	uint16_t x;

	x = htons(i);
	memcpy(h, &x, 2);
}

static uint32_t
_asl_get_32(char *h)
{
	uint32_t x;

	memcpy(&x, h, 4);
	return ntohl(x);
}

static void
_asl_put_32(uint32_t i, char *h)
{
	uint32_t x;

	x = htonl(i);
	memcpy(h, &x, 4);
}

static uint64_t
_asl_get_64(char *h)
{
	uint64_t x;

	memcpy(&x, h, 8);
	return asl_core_ntohq(x);
}

static void
_asl_put_64(uint64_t i, char *h)
{
	uint64_t x;

	x = asl_core_htonq(i);
	memcpy(h, &x, 8);
}

static ASL_STATUS
asl_file_read_uint32(asl_file_t *s, off_t off, uint32_t *out)
{
	uint32_t status, val;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->store == NULL) return ASL_STATUS_INVALID_STORE;
	if ((off + sizeof(uint32_t)) > s->file_size) return ASL_STATUS_READ_FAILED;

	status = fseeko(s->store, off, SEEK_SET);
	if (status != 0) return ASL_STATUS_READ_FAILED;

	val = 0;

	status = fread(&val, sizeof(uint32_t), 1, s->store);
	if (status != 1) return ASL_STATUS_READ_FAILED;

	if (out != NULL) *out = ntohl(val);
	return ASL_STATUS_OK;
}

static ASL_STATUS
asl_file_read_uint64(asl_file_t *s, off_t off, uint64_t *out)
{
	uint32_t status;
	uint64_t val;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->store == NULL) return ASL_STATUS_INVALID_STORE;
	if ((off + sizeof(uint64_t)) > s->file_size) return ASL_STATUS_READ_FAILED;

	status = fseeko(s->store, off, SEEK_SET);
	if (status != 0) return ASL_STATUS_READ_FAILED;

	val = 0;

	status = fread(&val, sizeof(uint64_t), 1, s->store);
	if (status != 1) return ASL_STATUS_READ_FAILED;

	if (out != NULL) *out = asl_core_ntohq(val);
	return ASL_STATUS_OK;
}

static file_string_t *
file_string_create(asl_file_t *s)
{
	if ((s != NULL) && (s->string_spare != NULL))
	{
		file_string_t *out = s->string_spare;
		s->string_spare = NULL;
		return out;
	}

	return (file_string_t *)calloc(1, sizeof(file_string_t));
}

static void
file_string_dispose(asl_file_t *s, file_string_t *x)
{
	if ((s != NULL) && (s->string_spare == NULL))
	{
		s->string_spare = x;
		memset(s->string_spare, 0, sizeof(file_string_t));
	}
	else
	{
		free(x);
	}
}


asl_file_t *
asl_file_retain(asl_file_t *s)
{
	if (s == NULL) return NULL;
	asl_retain((asl_object_t)s);
	return s;
}

void
asl_file_release(asl_file_t *s)
{
	if (s == NULL) return;
	asl_release((asl_object_t)s);
}

ASL_STATUS
asl_file_close(asl_file_t *s)
{
	if (s == NULL) return ASL_STATUS_OK;
	asl_release((asl_object_t)s);
	return ASL_STATUS_OK;
}

static void
_asl_file_free_internal(asl_file_t *s)
{
	file_string_t *x;

	if (s == NULL) return;

	if (s->version == 1)
	{
		asl_legacy1_close((asl_legacy1_t *)s->legacy);
		return;
	}

	while (s->string_list != NULL)
	{
		x = s->string_list->next;
		free(s->string_list);
		s->string_list = x;
	}

	free(s->string_spare);

	if (s->store != NULL) fclose(s->store);
	if (s->scratch != NULL) free(s->scratch);

	memset(s, 0, sizeof(asl_file_t));
	free(s);
}

__private_extern__ ASL_STATUS
asl_file_open_write_fd(int fd, asl_file_t **s)
{
	time_t now;
	int status;
	char buf[DB_HEADER_LEN];
	asl_file_t *out;

	if (fd < 0) return ASL_STATUS_FAILED;
	if (s == NULL) return ASL_STATUS_FAILED;

	out = (asl_file_t *)calloc(1, sizeof(asl_file_t));
	if (out == NULL) return ASL_STATUS_NO_MEMORY;

	out->asl_type = ASL_TYPE_FILE;
	out->refcount = 1;

	out->store = fdopen(fd, "w+");
	if (out->store == NULL)
	{
		free(out);
		return ASL_STATUS_FAILED;
	}

	memset(buf, 0, sizeof(buf));
	memcpy(buf, ASL_DB_COOKIE, ASL_DB_COOKIE_LEN);

	_asl_put_32(DB_VERSION, buf + DB_HEADER_VERS_OFFSET);

	now = time(NULL);
	out->dob = now;
	_asl_put_64(out->dob, buf + DB_HEADER_TIME_OFFSET);

	_asl_put_32(CACHE_SIZE, buf + DB_HEADER_CSIZE_OFFSET);

	status = fwrite(buf, sizeof(buf), 1, out->store);
	if (status != 1)
	{
		fclose(out->store);
		free(out);
		return ASL_STATUS_FAILED;
	}

	/* flush data */
	fflush(out->store);

	out->file_size = sizeof(buf);

	/* scratch buffer for file writes (we test for NULL before using it) */
	out->scratch = malloc(SCRATCH_BUFFER_SIZE);

	*s = out;

	return ASL_STATUS_OK;
}

__private_extern__ int
asl_file_create(const char *path, uid_t uid, gid_t gid, mode_t mode)
{
#if TARGET_OS_IPHONE
	return open(path, O_RDWR | O_CREAT | O_EXCL, mode);
#else
	acl_t acl;
	uuid_t uuid;
	acl_entry_t entry;
	acl_permset_t perms;
	int status;
	int fd = -1;

	/* -1 means don't set ACL for uid or gid */
	if ((uid == -1) && (gid == -1))
	{
		return open(path, O_RDWR | O_CREAT | O_EXCL, mode);
	}

	acl = acl_init(1);

	if ((gid != 0) && (gid != -1))
	{
		status = mbr_gid_to_uuid(gid, uuid);
		if (status != 0) goto asl_file_create_return;

		status = acl_create_entry_np(&acl, &entry, ACL_FIRST_ENTRY);
		if (status != 0) goto asl_file_create_return;

		status = acl_set_tag_type(entry, ACL_EXTENDED_ALLOW);
		if (status != 0) goto asl_file_create_return;

		status = acl_set_qualifier(entry, &uuid);
		if (status != 0) goto asl_file_create_return;

		status = acl_get_permset(entry, &perms);
		if (status != 0) goto asl_file_create_return;

		status = acl_add_perm(perms, ACL_READ_DATA);
		if (status != 0) goto asl_file_create_return;
	}

	if ((uid != 0) && (uid != -1))
	{
		status = mbr_uid_to_uuid(uid, uuid);
		if (status != 0) goto asl_file_create_return;

		status = acl_create_entry_np(&acl, &entry, ACL_FIRST_ENTRY);
		if (status != 0) goto asl_file_create_return;

		status = acl_set_tag_type(entry, ACL_EXTENDED_ALLOW);
		if (status != 0) goto asl_file_create_return;

		status = acl_set_qualifier(entry, &uuid);
		if (status != 0) goto asl_file_create_return;

		status = acl_get_permset(entry, &perms);
		if (status != 0) goto asl_file_create_return;

		status = acl_add_perm(perms, ACL_READ_DATA);
		if (status != 0) goto asl_file_create_return;
	}

	fd = open(path, O_RDWR | O_CREAT | O_EXCL, mode);
	if (fd < 0) goto asl_file_create_return;

	status = acl_set_fd(fd, acl);
	if (status != 0)
	{
		close(fd);
		fd = -1;
		unlink(path);
	}

asl_file_create_return:

	acl_free(acl);
	return fd;
#endif
}

ASL_STATUS
asl_file_open_write(const char *path, mode_t mode, uid_t uid, gid_t gid, asl_file_t **s)
{
	int i, status, fd;
	struct stat sb;
	char buf[DB_HEADER_LEN];
	asl_file_t *out;
	uint32_t aslstatus, vers, last_len;
	off_t off;

	memset(&sb, 0, sizeof(struct stat));

	status = stat(path, &sb);
	if (status == 0)
	{
		/* must be a plain file */
		if (!S_ISREG(sb.st_mode)) return ASL_STATUS_INVALID_STORE;

		/*
		 * If the file exists, we go with the existing mode, uid, and gid.
		 */

		if (sb.st_size == 0)
		{
			fd = open(path, O_RDWR | O_EXCL, mode);
			if (fd < 0) return ASL_STATUS_FAILED;

			return asl_file_open_write_fd(fd, s);
		}
		else
		{
			out = (asl_file_t *)calloc(1, sizeof(asl_file_t));
			if (out == NULL) return ASL_STATUS_NO_MEMORY;

			out->asl_type = ASL_TYPE_FILE;
			out->refcount = 1;

			out->store = fopen(path, "r+");
			if (out->store == NULL)
			{
				free(out);
				return ASL_STATUS_FAILED;
			}

			i = fread(buf, DB_HEADER_LEN, 1, out->store);
			if (i < 1)
			{
				asl_file_close(out);
				return ASL_STATUS_READ_FAILED;
			}

			/* check cookie */
			if (strncmp(buf, ASL_DB_COOKIE, ASL_DB_COOKIE_LEN))
			{
				asl_file_close(out);
				return ASL_STATUS_INVALID_STORE;
			}

			/* check version */
			vers = _asl_get_32(buf + DB_HEADER_VERS_OFFSET);
			if (vers != DB_VERSION)
			{
				asl_file_close(out);
				return ASL_STATUS_INVALID_STORE;
			}

			out->dob = _asl_get_64(buf + DB_HEADER_TIME_OFFSET);
			out->first = _asl_get_64(buf + DB_HEADER_FIRST_OFFSET);
			out->last = _asl_get_64(buf + DB_HEADER_LAST_OFFSET);
			out->file_size = (size_t)sb.st_size;

			/*
			 * Detect bogus last pointer and check for odd-sized files.
			 * Setting out->last to zero forces asl_file_read_set_position to
			 * follow the linked list of records in the file to the last record.
			 * It's slower, but it's better at preventing crashes in corrupt files.
			 */

			/* records are at least MSG_RECORD_FIXED_LENGTH bytes */
			if ((out->last + MSG_RECORD_FIXED_LENGTH) > out->file_size)
			{
				out->last = 0;
			}
			else
			{
				/* read last record length and make sure the file is at least that large */
				off = out->last + RECORD_TYPE_LEN;
				status = asl_file_read_uint32(out, off, &last_len);
				if (status != ASL_STATUS_OK)
				{
					asl_file_close(out);
					return status;
				}

				if ((out->last + last_len) > out->file_size) out->last = 0;
			}

			if (out->last != 0)
			{
				/* skip type (uint16_t), len (uint32_t), and next (uint64_t) */
				off = out->last + sizeof(uint16_t) + sizeof (uint32_t) + sizeof(uint64_t);
				status = asl_file_read_uint64(out, off, &(out->last_mid));
				if (status != ASL_STATUS_OK)
				{
					asl_file_close(out);
					return status;
				}
			}

			aslstatus = asl_file_read_set_position(out, ASL_FILE_POSITION_LAST);
			if (aslstatus != ASL_STATUS_OK)
			{
				asl_file_close(out);
				return aslstatus;
			}

			out->prev = out->cursor;
			status = fseeko(out->store, 0, SEEK_END);
			if (status != 0)
			{
				asl_file_close(out);
				return ASL_STATUS_READ_FAILED;
			}

			out->file_size = (size_t)ftello(out->store);

			/* scratch buffer for file writes (we test for NULL before using it) */
			out->scratch = malloc(SCRATCH_BUFFER_SIZE);

			*s = out;

			return ASL_STATUS_OK;
		}
	}
	else if (errno != ENOENT)
	{
		/* unexpected status */
		return ASL_STATUS_FAILED;
	}

	/*
	 * If the file does not exist, we set the mode, uid, and gid.
	 */

	fd = asl_file_create(path, uid, gid, mode);
	if (fd < 0) return ASL_STATUS_FAILED;

	aslstatus = asl_file_open_write_fd(fd, s);
	if (aslstatus != ASL_STATUS_OK) unlink(path);

	return aslstatus;
}

ASL_STATUS
asl_file_compact(asl_file_t *s, const char *path, mode_t mode, uid_t uid, gid_t gid)
{
	asl_file_t *new;
	struct stat sb;
	asl_msg_t *m;
	uint64_t xid;
	uint32_t status;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (path == NULL) return ASL_STATUS_INVALID_ARG;

	if (s->version == 1) return ASL_STATUS_FAILED;

	memset(&sb, 0, sizeof(struct stat));

	if (stat(path, &sb) == 0) return ASL_STATUS_FAILED;
	if (errno != ENOENT) return ASL_STATUS_FAILED;

	status = asl_file_read_set_position(s, ASL_FILE_POSITION_FIRST);
	if (status != ASL_STATUS_OK) return status;

	new = NULL;
	status = asl_file_open_write(path, mode, uid, gid, &new);
	if (status != ASL_STATUS_OK) return status;
	new->flags = ASL_FILE_FLAG_UNLIMITED_CACHE | ASL_FILE_FLAG_PRESERVE_MSG_ID;

	while ((status == ASL_STATUS_OK) && (s->cursor != 0))
	{
		m = NULL;
		status = asl_file_fetch_next(s, &m);
		if (status != ASL_STATUS_OK) break;

		xid = 0;
		status = asl_file_save(new, m, &xid);
		asl_msg_release(m);
	}

	asl_file_close(new);
	return status;
}

ASL_STATUS
asl_file_filter(asl_file_t *s, const char *path, asl_msg_list_t *filter, uint32_t flags, mode_t mode, uid_t uid, gid_t gid, uint32_t *dstcount, void (*aux_callback)(const char *auxfile))
{
	asl_file_t *new;
	struct stat sb;
	asl_msg_t *m;
	uint64_t xid = 0;
	uint32_t status, n = 0;
	uint32_t matchflag = flags & ASL_FILE_FILTER_FLAG_KEEP_MATCHES;

	if (dstcount != NULL) *dstcount = n;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (path == NULL) return ASL_STATUS_INVALID_ARG;

	if (s->version == 1) return ASL_STATUS_FAILED;

	memset(&sb, 0, sizeof(struct stat));

	if (stat(path, &sb) == 0) return ASL_STATUS_FAILED;
	if (errno != ENOENT) return ASL_STATUS_FAILED;

	status = asl_file_read_set_position(s, ASL_FILE_POSITION_FIRST);
	if (status != ASL_STATUS_OK) return status;

	new = NULL;
	status = asl_file_open_write(path, mode, uid, gid, &new);
	if (status != ASL_STATUS_OK) return status;
	new->flags = ASL_FILE_FLAG_UNLIMITED_CACHE | ASL_FILE_FLAG_PRESERVE_MSG_ID;

	while ((status == ASL_STATUS_OK) && (s->cursor != 0))
	{
		m = NULL;
		status = asl_file_fetch_next(s, &m);
		if (status != ASL_STATUS_OK) break;

		/*
		 * asl_msg_cmp_list is supposed to return 1 for a match, 0 otherwise,
		 * but just to be sure we only get a 1 or zero, we do an extra test.
		 */
		uint32_t msgmatch = (asl_msg_cmp_list(m, filter) == 0) ? 0 : 1;
		if (msgmatch == matchflag)
		{
			status = asl_file_save(new, m, &xid);
			if (status == ASL_STATUS_OK) n++;
		}
		else if (aux_callback != NULL)
		{
			/* check for ASL_KEY_AUX_URL and pass it to callback */
			const char *auxval = asl_msg_get_val_for_key(m, ASL_KEY_AUX_URL);
			if (auxval != NULL) aux_callback(auxval);
		}

		asl_msg_release(m);
	}

	asl_file_close(new);
	if (dstcount != NULL) *dstcount = n;
	return status;
}

ASL_STATUS
asl_file_filter_level(asl_file_t *s, const char *path, uint32_t keep_mask, mode_t mode, uid_t uid, gid_t gid, uint32_t *dstcount, void (*aux_callback)(const char *auxfile))
{
	asl_file_t *new;
	struct stat sb;
	asl_msg_t *m;
	uint64_t xid = 0;
	uint32_t status, n = 0;
	uint8_t kmcur, kmnew;

	if (dstcount != NULL) *dstcount = n;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (path == NULL) return ASL_STATUS_INVALID_ARG;

	if (s->version == 1) return ASL_STATUS_FAILED;

	memset(&sb, 0, sizeof(struct stat));

	if (stat(path, &sb) == 0) return ASL_STATUS_FAILED;
	if (errno != ENOENT) return ASL_STATUS_FAILED;

	kmnew = keep_mask;

	/* get current filter mask in file's header */
	status = fseeko(s->store, DB_HEADER_FILTER_MASK_OFFSET, SEEK_SET);
	if (status != 0) return ASL_STATUS_READ_FAILED;
	fread(&kmcur, 1, 1, s->store);

	status = asl_file_read_set_position(s, ASL_FILE_POSITION_FIRST);
	if (status != ASL_STATUS_OK) return status;

	new = NULL;
	status = asl_file_open_write(path, mode, uid, gid, &new);
	if (status != ASL_STATUS_OK) return status;

	new->flags = ASL_FILE_FLAG_UNLIMITED_CACHE | ASL_FILE_FLAG_PRESERVE_MSG_ID;

	while ((status == ASL_STATUS_OK) && (s->cursor != 0))
	{
		m = NULL;
		status = asl_file_fetch_next(s, &m);
		if (status != ASL_STATUS_OK) break;
		if (m == NULL) continue;
		const char *lval = asl_msg_get_val_for_key(m, ASL_KEY_LEVEL);
		if (lval == NULL) continue;
		uint32_t level_bit = 0x01 << atoi(lval);

		if (level_bit & keep_mask)
		{
			status = asl_file_save(new, m, &xid);
			if (status == ASL_STATUS_OK) n++;
		}
		else if (aux_callback != NULL)
		{
			/* check for ASL_KEY_AUX_URL and pass it to callback */
			const char *auxval = asl_msg_get_val_for_key(m, ASL_KEY_AUX_URL);
			if (auxval != NULL) aux_callback(auxval);
		}

		asl_msg_release(m);
	}

	kmnew = kmcur & kmnew;
	status = fseeko(new->store, DB_HEADER_FILTER_MASK_OFFSET, SEEK_SET);
	if (status != 0) return ASL_STATUS_READ_FAILED;
	fwrite(&kmnew, 1, 1, new->store);

	asl_file_close(new);
	if (dstcount != NULL) *dstcount = n;
	return status;
}

static ASL_STATUS
asl_file_string_encode(asl_file_t *s, const char *str, uint64_t *out)
{
	uint32_t i, hash, len, x32;
	file_string_t *sp, *sx, *sl;
	uint64_t x64;
	uint8_t inls;
	uint16_t type;
	off_t off;
	char *p;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->store == NULL) return ASL_STATUS_INVALID_STORE;
	if (str == NULL) return ASL_STATUS_INVALID_ARG;

	len = strlen(str);

	/* inline strings */
	if (len < 8)
	{
		/* inline string */
		inls = len;
		inls |= 0x80;

		x64 = 0;
		p = (char *)&x64;
		memcpy(p, &inls, 1);
		memcpy(p + 1, str, len);
		*out = asl_core_ntohq(x64);
		return ASL_STATUS_OK;
	}

	/* cached strings include trailing nul */
	len++;

	if (len <= CACHE_MAX_STRING_LEN)
	{
		/* check the cache */
		hash = asl_core_string_hash(str, len);

		sp = NULL;
		for (sx = s->string_list; sx != NULL; sx = sx->next)
		{
			if ((hash == sx->hash) && (!strcmp(str, sx->str)))
			{
				/* Move this string to the head of the list */
				if (sp != NULL)
				{
					sl = s->string_list;
					sp->next = sx->next;
					sx->next = sl;
					s->string_list = sx;
				}

				*out = sx->where;
				return ASL_STATUS_OK;
			}

			sp = sx;
		}
	}

	off = ftello(s->store);

	/* Type */
	type = htons(ASL_FILE_TYPE_STR);
	i = fwrite(&type, sizeof(uint16_t), 1, s->store);
	if (i != 1) return ASL_STATUS_WRITE_FAILED;

	/* Length (includes trailing nul) */
	x32 = htonl(len);
	i = fwrite(&x32, sizeof(uint32_t), 1, s->store);
	if (i != 1) return ASL_STATUS_WRITE_FAILED;

	/* String data (nul terminated) */
	i = fwrite(str, len, 1, s->store);
	if (i != 1) return ASL_STATUS_WRITE_FAILED;

	/* flush data */
	fflush(s->store);

	/*
	 * Create file_string_t and insert into the cache, but only if the
	 * string is small.  This prevents a huge string from eating memory.
	 * It's unlikely that large strings will be very re-usable.
	 */
	if (len <= CACHE_MAX_STRING_LEN)
	{
		sx = file_string_create(s);
		if (sx == NULL) return ASL_STATUS_NO_MEMORY;

		sx->where = off;
		sx->hash = hash;
		sx->next = s->string_list;

		/* includes trailing nul */
		memcpy(sx->str, str, len);

		s->string_list = sx;

		if (((s->flags & ASL_FILE_FLAG_UNLIMITED_CACHE) == 0) && (s->string_cache_count == CACHE_SIZE))
		{
			/* drop last (lru) string from cache */
			sp = s->string_list;
			sx = sp->next;

			/* NB CACHE_SIZE must be > 1 */
			while (sx->next != NULL)
			{
				sp = sx;
				sx = sx->next;
			}

			sp->next = NULL;
			file_string_dispose(s, sx);
		}
		else
		{
			s->string_cache_count++;
		}
	}

	*out = off;
	return ASL_STATUS_OK;
}

/*
 * Encode an asl_msg_t *as a record structure.
 * Creates and caches strings.
 */
#define KVSTACK_SIZE 128

ASL_STATUS
asl_file_save(asl_file_t *s, asl_msg_t *in, uint64_t *mid)
{
	char *buf, *p;
	uint32_t i, len, x, status;
	file_record_t r;
	uint64_t k, v;
	uint64_t kvstack[KVSTACK_SIZE];
	uint64_t *kvmalloc = NULL;
	uint64_t *kvlist;
	off_t off;
	asl_msg_t *msg;
	const char *key, *val;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->store == NULL) return ASL_STATUS_INVALID_STORE;
	if (in == NULL) return ASL_STATUS_INVALID_MESSAGE;

	if (s->flags & ASL_FILE_FLAG_READ) return ASL_STATUS_READ_ONLY;

	msg = (asl_msg_t *)in;

	memset(&r, 0, sizeof(file_record_t));

	r.mid = UINT64_MAX;
	if ((mid != NULL ) && (*mid != 0)) r.mid = *mid;

	r.flags = 0;
	r.level = ASL_LEVEL_DEBUG;
	r.pid = -1;
	r.uid = -2;
	r.gid = -2;
	r.ruid = -1;
	r.rgid = -1;
	r.time = 0;
	r.nano = 0;
	r.prev = s->prev;
	kvlist = kvstack;

	key = NULL;
	val = NULL;

	for (x = asl_msg_fetch(msg, 0, &key, &val, NULL); x != IndexNull; x = asl_msg_fetch(msg, x, &key, &val, NULL))
	{
		if (key == NULL)
		{
			continue;
		}
		else if (!strcmp(key, ASL_KEY_TIME))
		{
			if (val != NULL) r.time = asl_core_parse_time(val, NULL);
		}
		else if (!strcmp(key, ASL_KEY_TIME_NSEC))
		{
			if (val != NULL) r.nano = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_HOST))
		{
			if (val != NULL)
			{
				status = asl_file_string_encode(s, val, &(r.host));
				if (status != ASL_STATUS_OK)
				{
					free(kvmalloc);
					return status;
				}
			}
		}
		else if (!strcmp(key, ASL_KEY_SENDER))
		{
			if (val != NULL)
			{
				status = asl_file_string_encode(s, val, &(r.sender));
				if (status != ASL_STATUS_OK)
				{
					free(kvmalloc);
					return status;
				}
			}
		}
		else if (!strcmp(key, ASL_KEY_PID))
		{
			if (val != NULL) r.pid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_REF_PID))
		{
			if (val != NULL) r.refpid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_UID))
		{
			if (val != NULL) r.uid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_GID))
		{
			if (val != NULL) r.gid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_LEVEL))
		{
			if (val != NULL) r.level = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_MSG))
		{
			if (val != NULL)
			{
				status = asl_file_string_encode(s, val, &(r.message));
				if (status != ASL_STATUS_OK)
				{
					free(kvmalloc);
					return status;
				}
			}
		}
		else if (!strcmp(key, ASL_KEY_FACILITY))
		{
			if (val != NULL)
			{
				status = asl_file_string_encode(s, val, &(r.facility));
				if (status != ASL_STATUS_OK)
				{
					free(kvmalloc);
					return status;
				}
			}
		}
		else if (!strcmp(key, ASL_KEY_REF_PROC))
		{
			if (val != NULL)
			{
				status = asl_file_string_encode(s, val, &(r.refproc));
				if (status != ASL_STATUS_OK)
				{
					free(kvmalloc);
					return status;
				}
			}
		}
		else if (!strcmp(key, ASL_KEY_SESSION))
		{
			if (val != NULL)
			{
				status = asl_file_string_encode(s, val, &(r.session));
				if (status != ASL_STATUS_OK)
				{
					free(kvmalloc);
					return status;
				}
			}
		}
		else if (!strcmp(key, ASL_KEY_READ_UID))
		{
			if (((r.flags & ASL_MSG_FLAG_READ_UID_SET) == 0) && (val != NULL))
			{
				r.ruid = atoi(val);
				r.flags |= ASL_MSG_FLAG_READ_UID_SET;
			}
		}
		else if (!strcmp(key, ASL_KEY_READ_GID))
		{
			if (((r.flags & ASL_MSG_FLAG_READ_GID_SET) == 0) && (val != NULL))
			{
				r.rgid = atoi(val);
				r.flags |= ASL_MSG_FLAG_READ_GID_SET;
			}
		}
		else if (!strcmp(key, ASL_KEY_MSG_ID))
		{
			if (s->flags & ASL_FILE_FLAG_PRESERVE_MSG_ID)
			{
				r.mid = atoll(val);
				if (mid != NULL) *mid = r.mid;
			}
		}
		else if (!strcmp(key, ASL_KEY_OPTION))
		{
			/* ignore - we don't save ASLOption */
		}
		else
		{
			status = asl_file_string_encode(s, key, &k);
			if (status != ASL_STATUS_OK)
			{
				free(kvmalloc);
				return status;
			}

			v = 0;
			if (val != NULL)
			{
				status = asl_file_string_encode(s, val, &v);
				if (status != ASL_STATUS_OK)
				{
					free(kvmalloc);
					return status;
				}
			}

			if (r.kvcount >= KVSTACK_SIZE)
			{
				/* out of space for the kvlist on the stack - fall back to malloc */
				kvmalloc = reallocf(kvmalloc, (r.kvcount + 2) * sizeof(uint64_t));
				if (kvmalloc == NULL) return ASL_STATUS_NO_MEMORY;

				kvlist = kvmalloc;

				if (r.kvcount == KVSTACK_SIZE)
				{
					/* copy kvstack to kvmalloc */
					for (i = 0; i < KVSTACK_SIZE; i++) kvmalloc[i] = kvstack[i];
				}
			}

			kvlist[r.kvcount++] = k;
			kvlist[r.kvcount++] = v;
		}
	}

	len = MSG_RECORD_FIXED_LENGTH + (r.kvcount * sizeof(uint64_t));
	buf = NULL;

	/* use the scratch buffer if it exists and is large enough */
	if ((s->scratch != NULL) && (len <= SCRATCH_BUFFER_SIZE))
	{
		memset(s->scratch, 0, SCRATCH_BUFFER_SIZE);
		buf = s->scratch;
	}
	else
	{
		buf = calloc(1, len);
	}

	if (buf == NULL) return ASL_STATUS_NO_MEMORY;

	if (r.mid == UINT64_MAX)
	{
		s->last_mid = s->last_mid + 1;
		r.mid = s->last_mid;
	}

	p = buf;

	/* Type */
	_asl_put_16(ASL_FILE_TYPE_MSG, p);
	p += sizeof(uint16_t);

	/* Length of message (excludes type and length fields) */
	_asl_put_32(len - RECORD_COMMON_LEN, p);
	p += sizeof(uint32_t);

	/* Message data... */

	_asl_put_64(r.next, p);
	p += sizeof(uint64_t);

	_asl_put_64(r.mid, p);
	p += sizeof(uint64_t);

	_asl_put_64(r.time, p);
	p += sizeof(uint64_t);

	_asl_put_32(r.nano, p);
	p += sizeof(uint32_t);

	_asl_put_16(r.level, p);
	p += sizeof(uint16_t);

	_asl_put_16(r.flags, p);
	p += sizeof(uint16_t);

	_asl_put_32(r.pid, p);
	p += sizeof(uint32_t);

	_asl_put_32(r.uid, p);
	p += sizeof(uint32_t);

	_asl_put_32(r.gid, p);
	p += sizeof(uint32_t);

	_asl_put_32(r.ruid, p);
	p += sizeof(uint32_t);

	_asl_put_32(r.rgid, p);
	p += sizeof(uint32_t);

	_asl_put_32(r.refpid, p);
	p += sizeof(uint32_t);

	_asl_put_32(r.kvcount, p);
	p += sizeof(uint32_t);

	_asl_put_64(r.host, p);
	p += sizeof(uint64_t);

	_asl_put_64(r.sender, p);
	p += sizeof(uint64_t);

	_asl_put_64(r.facility, p);
	p += sizeof(uint64_t);

	_asl_put_64(r.message, p);
	p += sizeof(uint64_t);

	_asl_put_64(r.refproc, p);
	p += sizeof(uint64_t);

	_asl_put_64(r.session, p);
	p += sizeof(uint64_t);

	for (i = 0; i < r.kvcount; i++)
	{
		_asl_put_64(kvlist[i], p);
		p += sizeof(uint64_t);
	}

	_asl_put_64(r.prev, p);
	p += sizeof(uint64_t);

	free(kvmalloc);

	/* write record at end of file */
	status = fseeko(s->store, 0, SEEK_END);
	if (status != 0) return ASL_STATUS_WRITE_FAILED;

	s->last = (uint64_t)ftello(s->store);

	v = asl_core_htonq(s->last);

	status = fwrite(buf, len, 1, s->store);
	fflush(s->store);

	/* free the buffer if it was allocated here */
	if (buf != s->scratch) free(buf);

	/* seek to "next" field of previous record, write last offset */
	off = s->prev + RECORD_COMMON_LEN;
	if (s->prev == 0) off = DB_HEADER_FIRST_OFFSET;

	status = fseeko(s->store, off, SEEK_SET);
	if (status != 0) return ASL_STATUS_WRITE_FAILED;

	status = fwrite(&v, sizeof(uint64_t), 1, s->store);
	if (status != 1) return ASL_STATUS_WRITE_FAILED;

	/* seek to DB_HEADER_LAST_OFFSET, write last record offset */
	off = DB_HEADER_LAST_OFFSET;

	status = fseeko(s->store, off, SEEK_SET);
	if (status != 0) return ASL_STATUS_WRITE_FAILED;

	status = fwrite(&v, sizeof(uint64_t), 1, s->store);
	if (status != 1) return ASL_STATUS_WRITE_FAILED;

	/* return to the end of the store (this is expected by other routines) */
	status = fseeko(s->store, 0, SEEK_END);
	if (status != 0) return ASL_STATUS_WRITE_FAILED;

	/* flush data */
	fflush(s->store);

	s->file_size = (size_t)ftello(s->store);

	s->prev = s->last;

	return ASL_STATUS_OK;
}

static ASL_STATUS
asl_file_fetch_object(asl_file_t *s, uint16_t fetch_type, uint64_t where, char **out, uint32_t *outlen)
{
	char ils[9];
	char *p;
	uint32_t len;
	int status;
	uint64_t x64;
	uint8_t inls;
	uint16_t type;
	off_t off;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->store == NULL) return ASL_STATUS_INVALID_STORE;
	if (out == NULL) return ASL_STATUS_INVALID_ARG;
	if (outlen == NULL) return ASL_STATUS_INVALID_ARG;
	if (where == 0) return ASL_STATUS_INVALID_ARG;

	*out = NULL;
	*outlen = 0;

	inls = 0;
	x64 = asl_core_htonq(where);
	memcpy(&inls, &x64, 1);
	if (inls & 0x80)
	{
		if (fetch_type != ASL_FILE_TYPE_STR) return ASL_STATUS_INVALID_STORE;

		/* inline string */
		inls &= 0x0f;
		if (inls > 7) return ASL_STATUS_INVALID_STORE;

		p = 1 + (char *)&x64;
		memset(ils, 0, sizeof(ils));
		memcpy(ils, p, inls);
		*out = strdup(ils);
		if (*out == NULL) return ASL_STATUS_NO_MEMORY;

		*outlen = inls;
		return ASL_STATUS_OK;
	}

	if (fetch_type == ASL_FILE_TYPE_STR)
	{
		/* check the string cache */
		file_string_t *sx, *sp;

		sp = NULL;
		for (sx = s->string_list; sx != NULL; sx = sx->next)
		{
			if (sx->where == where)
			{
				*out = strdup(sx->str);
				if (*out == NULL) return ASL_STATUS_NO_MEMORY;

				/* N.B. hash field is used to hold length when reading */
				*outlen = sx->hash;

				/* Move this string to the head of the list */
				if (sp != NULL)
				{
					file_string_t *sl = s->string_list;
					sp->next = sx->next;
					sx->next = sl;
					s->string_list = sx;
				}

				return ASL_STATUS_OK;
			}

			sp = sx;
		}
	}

	off = where;
	if ((off + sizeof(uint16_t) + sizeof(uint32_t)) > s->file_size) return ASL_STATUS_READ_FAILED;

	status = fseeko(s->store, off, SEEK_SET);
	if (status != 0) return ASL_STATUS_READ_FAILED;

	/* Type */
	status = fread(&type, sizeof(uint16_t), 1, s->store);
	if (status != 1) return ASL_STATUS_READ_FAILED;
	type = ntohs(type);
	off += sizeof(uint16_t);

	if (type != fetch_type) return ASL_STATUS_INVALID_STORE;

	/* Length */
	len = 0;
	status = fread(&len, sizeof(uint32_t), 1, s->store);
	if (status != 1) return ASL_STATUS_READ_FAILED;
	off += sizeof(uint32_t);

	len = ntohl(len);
	if ((off + len) > s->file_size) return ASL_STATUS_READ_FAILED;

	*out = calloc(1, len);
	if (*out == NULL) return ASL_STATUS_NO_MEMORY;

	status = fread(*out, len, 1, s->store);
	if (status != 1)
	{
		free(*out);
		*out = NULL;
		return ASL_STATUS_READ_FAILED;
	}

	*outlen = len;

	if ((fetch_type == ASL_FILE_TYPE_STR) && (len <= CACHE_MAX_STRING_LEN))
	{
		file_string_t *sx = file_string_create(s);
		if (sx != NULL)
		{
			sx->where = where;

			/* N.B. hash field is used to hold length when reading */
			sx->hash = len;
			sx->next = s->string_list;
			memcpy(sx->str, *out, len);

			s->string_list = sx;

			if (((s->flags & ASL_FILE_FLAG_UNLIMITED_CACHE) == 0) && (s->string_cache_count == CACHE_SIZE))
			{
				/* drop last (lru) string from cache */
				file_string_t *sp = s->string_list;
				sx = sp->next;

				/* NB CACHE_SIZE must be > 1 */
				while (sx->next != NULL)
				{
					sp = sx;
					sx = sx->next;
				}

				sp->next = NULL;
				file_string_dispose(s, sx);
			}
			else
			{
				s->string_cache_count++;
			}
		}
	}

	return ASL_STATUS_OK;
}

static uint16_t
asl_file_fetch_helper_16(asl_file_t *s, char **p, asl_msg_t *m, const char *key)
{
	uint16_t out;
	char str[256];

	out = _asl_get_16(*p);
	*p += sizeof(uint16_t);

	if ((m == NULL) || (key == NULL)) return out;

	snprintf(str, sizeof(str), "%hu", out);
	asl_msg_set_key_val(m, key, str);

	return out;
}

static uint32_t
asl_file_fetch_helper_32(asl_file_t *s, char **p, asl_msg_t *m, const char *key, int ignore, uint32_t ignoreval)
{
	uint32_t out, doit;
	char str[256];

	out = _asl_get_32(*p);
	*p += sizeof(uint32_t);

	if ((m == NULL) || (key == NULL)) return out;

	doit = 1;
	if ((ignore != 0) && (out == ignoreval)) doit = 0;
	if (doit != 0)
	{
		snprintf(str, sizeof(str), "%u", out);
		asl_msg_set_key_val(m, key, str);
	}

	return out;
}

static uint64_t
asl_file_fetch_helper_64(asl_file_t *s, char **p, asl_msg_t *m, const char *key)
{
	uint64_t out;
	char str[256];

	out = _asl_get_64(*p);
	*p += sizeof(uint64_t);

	if ((m == NULL) || (key == NULL)) return out;

	snprintf(str, sizeof(str), "%llu", out);
	asl_msg_set_key_val(m, key, str);

	return out;
}

static uint64_t
asl_file_fetch_helper_str(asl_file_t *s, char **p, asl_msg_t *m, const char *key, uint32_t *err)
{
	uint64_t out;
	char *val;
	uint32_t status, len;

	out = _asl_get_64(*p);
	*p += sizeof(uint64_t);

	val = NULL;
	len = 0;
	status = ASL_STATUS_OK;
	if (out != 0) status = asl_file_fetch_object(s, ASL_FILE_TYPE_STR, out, &val, &len);

	if (err != NULL) *err = status;
	if ((status == ASL_STATUS_OK) && (val != NULL))
	{
		asl_msg_set_key_val(m, key, val);
		free(val);
	}

	return out;
}

static ASL_STATUS
asl_file_fetch_pos(asl_file_t *s, uint64_t where, int dir, asl_msg_t **msg)
{
	char *buf, *p, *k, *v;
	file_record_t r;
	uint32_t i, status, len, buflen, kvn;
	uint64_t x64, kv;
	asl_msg_t *out;
	off_t off;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->store == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;
	if ((s->flags & ASL_FILE_FLAG_READ) == 0) return ASL_STATUS_WRITE_ONLY;

	buf = NULL;
	buflen = 0;
	status = asl_file_fetch_object(s, ASL_FILE_TYPE_MSG, where, &buf, &buflen);
	if ((status != ASL_STATUS_OK) || (buf == NULL))
	{
		s->cursor = 0;
		s->cursor_xid = 0;
		return status;
	}

	/* check buffer size */
	kvn = _asl_get_32(buf + BUFFER_OFFSET_KVCOUNT);
	if (buflen < (MSG_RECORD_FIXED_LENGTH - RECORD_COMMON_LEN + (kvn * sizeof(uint64_t))))
	{
		free(buf);
		s->cursor = 0;
		s->cursor_xid = 0;
		return ASL_STATUS_READ_FAILED;
	}

	out = asl_msg_new(ASL_TYPE_MSG);
	if (out == NULL)
	{
		free(buf);
		return ASL_STATUS_NO_MEMORY;
	}

	memset(&r, 0, sizeof(file_record_t));
	p = buf;

	r.next = asl_file_fetch_helper_64(s, &p, NULL, NULL);
	r.mid = asl_file_fetch_helper_64(s, &p, out, ASL_KEY_MSG_ID);
	r.time = asl_file_fetch_helper_64(s, &p, out, ASL_KEY_TIME);
	r.nano = asl_file_fetch_helper_32(s, &p, out, ASL_KEY_TIME_NSEC, 0, 0);
	r.level = asl_file_fetch_helper_16(s, &p, out, ASL_KEY_LEVEL);
	r.flags = asl_file_fetch_helper_16(s, &p, NULL, NULL);
	r.pid = asl_file_fetch_helper_32(s, &p, out, ASL_KEY_PID, 0, 0);
	r.uid = asl_file_fetch_helper_32(s, &p, out, ASL_KEY_UID, 1, (uint32_t)-1);
	r.gid = asl_file_fetch_helper_32(s, &p, out, ASL_KEY_GID, 1, (uint32_t)-1);
	r.ruid = asl_file_fetch_helper_32(s, &p, out, ASL_KEY_READ_UID, 1, (uint32_t)-1);
	r.rgid = asl_file_fetch_helper_32(s, &p, out, ASL_KEY_READ_GID, 1, (uint32_t)-1);
	r.refpid = asl_file_fetch_helper_32(s, &p, out, ASL_KEY_REF_PID, 1, 0);
	r.kvcount = asl_file_fetch_helper_32(s, &p, NULL, NULL, 0, 0);

	status = ASL_STATUS_OK;
	r.host = asl_file_fetch_helper_str(s, &p, out, ASL_KEY_HOST, &status); /* 68 */
	if (status == ASL_STATUS_OK) r.sender = asl_file_fetch_helper_str(s, &p, out, ASL_KEY_SENDER, &status); /* 76 */
	if (status == ASL_STATUS_OK) r.facility = asl_file_fetch_helper_str(s, &p, out, ASL_KEY_FACILITY, &status); /* 84 */
	if (status == ASL_STATUS_OK) r.message = asl_file_fetch_helper_str(s, &p, out, ASL_KEY_MSG, &status); /* 92 */
	if (status == ASL_STATUS_OK) r.refproc = asl_file_fetch_helper_str(s, &p, out, ASL_KEY_REF_PROC, &status); /* 100 */
	if (status == ASL_STATUS_OK) r.session = asl_file_fetch_helper_str(s, &p, out, ASL_KEY_SESSION, &status); /* 108 */

	if (status != ASL_STATUS_OK)
	{
		asl_msg_release(out);
		free(buf);
		s->cursor = 0;
		s->cursor_xid = 0;
		return status;
	}

	kvn =  r.kvcount / 2;

	for (i = 0; i < kvn; i++)
	{
		k = NULL;
		v = NULL;
		len = 0;

		kv = _asl_get_64(p);
		p += sizeof(uint64_t);

		status = asl_file_fetch_object(s, ASL_FILE_TYPE_STR, kv, &k, &len);
		if (status != ASL_STATUS_OK)
		{
			asl_msg_release(out);
			free(buf);
			s->cursor = 0;
			s->cursor_xid = 0;
			return status;
		}

		kv = _asl_get_64(p);
		p += sizeof(uint64_t);
		len = 0;

		if (kv != 0)
		{
			status = asl_file_fetch_object(s, ASL_FILE_TYPE_STR, kv, &v, &len);
			if (status != ASL_STATUS_OK)
			{
				free(k);
				asl_msg_release(out);
				free(buf);
				s->cursor = 0;
				s->cursor_xid = 0;
				return status;
			}
		}

		if ((status == ASL_STATUS_OK) && (k != NULL))
		{
			asl_msg_set_key_val(out, k, v);
		}

		free(k);
		free(v);
	}

	r.prev = asl_file_fetch_helper_64(s, &p, NULL, NULL); /* 116 */

	free(buf);

	if (dir >= 0)
	{
		if ((r.next != 0) && (r.next <= s->cursor))
		{
			/*
			 * Next offset goes backwards or loops.
			 * The database is corrupt, but we allow this call to fail
			 * quietly so that the current record fetch succeeds.
			 */
			s->cursor = 0;
			s->cursor_xid = 0;
			return ASL_STATUS_OK;
		}

		s->cursor = r.next;
	}
	else
	{
		if ((r.prev != 0) && (r.prev >= s->cursor))
		{
			/*
			 * Prev offset goes forward or loops.
			 * The database is corrupt, but we allow this call to fail
			 * quietly so that the current record fetch succeeds.
			 */
			s->cursor = 0;
			s->cursor_xid = 0;
			return ASL_STATUS_OK;
		}

		s->cursor = r.prev;
	}

	s->cursor_xid = 0;

	if (s->cursor != 0)
	{
		off = s->cursor + RECORD_COMMON_LEN + sizeof(uint64_t);
		if (off > s->file_size)
		{
			s->cursor = 0;
			s->cursor_xid = 0;
			/*
			 * Next record offset is past the end of the file.
			 * This is an error, but we allow it to fail quietly
			 * so that the current record fetch succeeds.
			 */
			*msg = out;
			return ASL_STATUS_OK;
		}

		status = fseeko(s->store, off, SEEK_SET);
		if (status != 0)
		{
			asl_msg_release(out);
			s->cursor = 0;
			s->cursor_xid = 0;
			return ASL_STATUS_READ_FAILED;
		}

		status = fread(&x64, sizeof(uint64_t), 1, s->store);
		if (status != 1)
		{
			asl_msg_release(out);
			s->cursor = 0;
			s->cursor_xid = 0;
			return ASL_STATUS_READ_FAILED;
		}

		s->cursor_xid = asl_core_ntohq(x64);
	}

	*msg = out;
	return ASL_STATUS_OK;
}

ASL_STATUS
asl_file_open_read(const char *path, asl_file_t **s)
{
	asl_file_t *out;
	FILE *f;
	int i;
	uint32_t status, vers, last_len;
	char buf[DB_HEADER_LEN];
	off_t off;
	asl_legacy1_t *legacy;
	struct stat sb;

	memset(&sb, 0, sizeof(struct stat));
	if (stat(path, &sb) != 0) return ASL_STATUS_FAILED;

	f = fopen(path, "r");
	if (f == NULL)
	{
		if (errno == EACCES) return ASL_STATUS_ACCESS_DENIED;
		return ASL_STATUS_FAILED;
	}

	i = fread(buf, DB_HEADER_LEN, 1, f);
	if (i < 1)
	{
		fclose(f);
		return ASL_STATUS_INVALID_STORE;
	}

	/* validate header */
	if (strncmp(buf, ASL_DB_COOKIE, ASL_DB_COOKIE_LEN))
	{
		fclose(f);
		return ASL_STATUS_INVALID_STORE;
	}

	legacy = NULL;

	vers = _asl_get_32(buf + DB_HEADER_VERS_OFFSET);
	if (vers == DB_VERSION_LEGACY_1)
	{
		fclose(f);
		status = asl_legacy1_open(path, &legacy);
		if (status != ASL_STATUS_OK) return status;
	}

	out = (asl_file_t *)calloc(1, sizeof(asl_file_t));
	if (out == NULL)
	{
		fclose(f);
		return ASL_STATUS_NO_MEMORY;
	}

	out->asl_type = ASL_TYPE_FILE;
	out->refcount = 1;

	out->store = f;
	out->flags = ASL_FILE_FLAG_READ;
	out->version = vers;

	if (legacy != NULL)
	{
		out->flags |= ASL_FILE_FLAG_LEGACY_STORE;
		out->legacy = (void *)legacy;

		*s = out;
		return ASL_STATUS_OK;
	}

	out->first = _asl_get_64(buf + DB_HEADER_FIRST_OFFSET);
	out->last = _asl_get_64(buf + DB_HEADER_LAST_OFFSET);
	out->file_size = (size_t)sb.st_size;

	/*
	 * Detect bogus last pointer and check for odd-sized files.
	 * Setting out->last to zero forces us to follow the linked
	 * list of records in the file to the last record.  That's
	 * done in the set_position code.  It's a bit slower, but it's
	 * better at preventing crashes in corrupt files.
	 */

	 /* records are at least MSG_RECORD_FIXED_LENGTH bytes */
	if ((out->last + MSG_RECORD_FIXED_LENGTH) > out->file_size)
	{
		out->last = 0;
	}
	else
	{
		/* read last record length and make sure the file is at least that large */
		off = out->last + RECORD_TYPE_LEN;
		status = asl_file_read_uint32(out, off, &last_len);
		if (status != ASL_STATUS_OK)
		{
			fclose(out->store);
			free(out);
			return status;
		}

		if ((out->last + last_len) > out->file_size) out->last = 0;
	}

	out->cursor = out->first;
	if (out->cursor != 0)
	{
		off = out->cursor + RECORD_COMMON_LEN + sizeof(uint64_t);
		status = asl_file_read_uint64(out, off, &(out->cursor_xid));
		if (status != ASL_STATUS_OK)
		{
			fclose(out->store);
			free(out);
			return status;
		}
	}

	*s = out;
	return ASL_STATUS_OK;
}

static ASL_STATUS
asl_file_read_set_position_first(asl_file_t *s)
{
	uint32_t status;
	off_t off;

	s->cursor = s->first;
	s->cursor_xid = 0;

	if (s->cursor == 0) return ASL_STATUS_OK;

	/* read ID of the first record */
	off = s->cursor + RECORD_COMMON_LEN + sizeof(uint64_t);
	status = asl_file_read_uint64(s, off, &(s->cursor_xid));
	return status;
}

static ASL_STATUS
asl_file_read_set_position_last(asl_file_t *s, int do_count)
{
	uint64_t next;
	uint32_t status;
	off_t off;

	/*
	 * If the file has the offset of the last record, we just go there.
	 * The last record offset was added to improve performance, so it may
	 * or may not be there.  If we don't have the last record offset, we
	 * just iterate down the record links to find the last one.
	 *
	 * Note that s->last may be zero if the file is empty.
	 */

	if ((s->last != 0) && (do_count == 0))
	{
		s->cursor = s->last;
		off = s->last + RECORD_COMMON_LEN + sizeof(uint64_t);

		/* read ID of the last record */
		status = asl_file_read_uint64(s, off, &(s->cursor_xid));
		return status;
	}

	/* start at the first record and iterate */
	s->cursor = s->first;
	s->cursor_xid = 0;
	s->msg_count = 0;

	forever
	{
		off = s->cursor + RECORD_COMMON_LEN;
		next = 0;

		/* read next offset */
		status = asl_file_read_uint64(s, off, &next);
		if (status != ASL_STATUS_OK) return status;

		/* detect bogus next pointer */
		if (((next + MSG_RECORD_FIXED_LENGTH) > s->file_size) || (next <= s->cursor)) next = 0;

		s->msg_count++;

		if (next == 0)
		{
			if (s->cursor == 0) return ASL_STATUS_OK;

			off = s->cursor + RECORD_COMMON_LEN + sizeof(uint64_t);
			status = asl_file_read_uint64(s, off, &(s->cursor_xid));
			return ASL_STATUS_OK;
		}

		s->cursor = next;
	}
}

ASL_STATUS
asl_file_read_set_position(asl_file_t *s, uint32_t pos)
{
	uint64_t next;
	uint32_t len, status;
	off_t off;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->version == 1) return ASL_STATUS_FAILED;

	if (pos == ASL_FILE_POSITION_FIRST) return asl_file_read_set_position_first(s);
	if (pos == ASL_FILE_POSITION_LAST) return asl_file_read_set_position_last(s, 0);

	off = 0;

	if (pos == ASL_FILE_POSITION_PREVIOUS)
	{
		if (s->cursor == s->first) return ASL_STATUS_NO_RECORDS;
		if (s->cursor == 0) return ASL_STATUS_NO_RECORDS;

		off = s->cursor + RECORD_TYPE_LEN;
		status = asl_file_read_uint32(s, off, &len);
		if (status != ASL_STATUS_OK) return status;

		/* set offset to read the "previous" field at the end of the record */
		off = s->cursor + RECORD_COMMON_LEN + len - sizeof(uint64_t);
	}
	else if (pos == ASL_FILE_POSITION_NEXT)
	{
		if (s->cursor == s->last) return ASL_STATUS_NO_RECORDS;
		if (s->cursor == 0) return ASL_STATUS_NO_RECORDS;

		/* set offset to read the "next" field in the current record */
		off = s->cursor + RECORD_COMMON_LEN;
	}
	else return ASL_STATUS_INVALID_ARG;

	s->cursor_xid = 0;

	/*
	 * read offset of next / previous
	 */
	next = 0;
	status = asl_file_read_uint64(s, off, &next);
	if (status != ASL_STATUS_OK) return ASL_STATUS_READ_FAILED;

	/* detect bogus next pointer */
	if ((next + MSG_RECORD_FIXED_LENGTH) > s->file_size) next = 0;
	else if ((pos == ASL_FILE_POSITION_PREVIOUS) && (next >= s->cursor)) next = 0;
	else if ((pos == ASL_FILE_POSITION_NEXT) && (next <= s->cursor)) next = 0;

	s->cursor = next;
	if (s->cursor == 0) return ASL_STATUS_NO_RECORDS;

	/* read ID of the record */
	off = s->cursor + RECORD_COMMON_LEN + sizeof(uint64_t);
	status = asl_file_read_uint64(s, off, &(s->cursor_xid));
	return status;
}

ASL_STATUS
asl_file_fetch_next(asl_file_t *s, asl_msg_t **msg)
{
	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->version == 1) return ASL_STATUS_FAILED;

	return asl_file_fetch_pos(s, s->cursor, 1, msg);
}

ASL_STATUS
asl_file_fetch_previous(asl_file_t *s, asl_msg_t **msg)
{
	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->version == 1) return ASL_STATUS_FAILED;

	return asl_file_fetch_pos(s, s->cursor, -1, msg);
}

ASL_STATUS
asl_file_fetch(asl_file_t *s, uint64_t mid, asl_msg_t **msg)
{
	uint32_t status;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if ((s->flags & ASL_FILE_FLAG_READ) == 0) return ASL_STATUS_WRITE_ONLY;

	if (s->version == 1)
	{
		if (msg == NULL) return ASL_STATUS_OK;
		return asl_legacy1_fetch((asl_legacy1_t *)s->legacy, mid, msg);
	}

	if (s->cursor_xid == 0)
	{
		status = asl_file_read_set_position(s, ASL_FILE_POSITION_FIRST);
		if (status != ASL_STATUS_OK) return status;
		if (s->cursor_xid == 0) return ASL_STATUS_INVALID_ID;
	}

	while (s->cursor_xid < mid)
	{
		status = asl_file_read_set_position(s, ASL_FILE_POSITION_NEXT);
		if (status != ASL_STATUS_OK) return status;
		if (s->cursor_xid > mid) return ASL_STATUS_INVALID_ID;
		if (s->cursor_xid == 0) return ASL_STATUS_INVALID_ID;
	}

	while (s->cursor_xid > mid)
	{
		status = asl_file_read_set_position(s, ASL_FILE_POSITION_PREVIOUS);
		if (status != ASL_STATUS_OK) return status;
		if (s->cursor_xid < mid) return ASL_STATUS_INVALID_ID;
		if (s->cursor_xid == 0) return ASL_STATUS_INVALID_ID;
	}

	if (s->cursor_xid != mid) return ASL_STATUS_INVALID_ID;

	if (msg == NULL) return ASL_STATUS_OK;
	return asl_file_fetch_pos(s, s->cursor, 1, msg);
}

__private_extern__ uint64_t
asl_file_cursor(asl_file_t *s)
{
	if (s == NULL) return 0;
	if ((s->flags & ASL_FILE_FLAG_READ) == 0) return 0;
	if (s->version == 1) return 0;

	return s->cursor_xid;
}

__private_extern__ ASL_STATUS
asl_file_match_start(asl_file_t *s, uint64_t start, int32_t direction)
{
	uint32_t status, d;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->version == 1) return ASL_STATUS_INVALID_STORE;
	if ((s->flags & ASL_FILE_FLAG_READ) == 0) return ASL_STATUS_WRITE_ONLY;

	d = ASL_FILE_POSITION_NEXT;
	if (direction < 0) d = ASL_FILE_POSITION_PREVIOUS;

	/*
	 * find starting point
	 */
	status = ASL_STATUS_OK;
	if (direction >= 0) status = asl_file_read_set_position(s, ASL_FILE_POSITION_FIRST);
	else status = asl_file_read_set_position(s, ASL_FILE_POSITION_LAST);
	if (status != ASL_STATUS_OK) return status;

	while ((status == ASL_STATUS_OK) && (((direction >= 0) && (s->cursor_xid < start)) || ((direction < 0) && (s->cursor_xid > start))))
	{
		status = asl_file_read_set_position(s, d);
	}

	return status;
}

__private_extern__ ASL_STATUS
asl_file_match_next(asl_file_t *s, asl_msg_list_t *query, asl_msg_t **msg, uint64_t *last, int32_t direction)
{
	uint32_t status, d, i, do_match, did_match;
	asl_msg_t *m;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;
	if (s->version == 1) return ASL_STATUS_INVALID_STORE;
	if ((s->flags & ASL_FILE_FLAG_READ) == 0) return ASL_STATUS_WRITE_ONLY;
	if (s->cursor == 0) return ASL_STATUS_NO_RECORDS;

	*msg = NULL;

	do_match = 1;
	if (query == NULL) do_match = 0;
	else if (query->count == 0) do_match = 0;

	d = ASL_FILE_POSITION_NEXT;
	if (direction < 0) d = ASL_FILE_POSITION_PREVIOUS;

	m = NULL;

	*last = s->cursor_xid;

	status = asl_file_fetch_pos(s, s->cursor, direction, &m);
	if (status == ASL_STATUS_ACCESS_DENIED) return ASL_STATUS_MATCH_FAILED;
	if ((status == ASL_STATUS_INVALID_ARG) && (s->cursor == 0)) return ASL_STATUS_NO_RECORDS;
	if (status != ASL_STATUS_OK) return status;

	did_match = 1;

	if (do_match != 0)
	{
		did_match = 0;

		for (i = 0; (i < query->count) && (did_match == 0); i++)
		{
			did_match = asl_msg_cmp(query->msg[i], m);
		}
	}

	if (did_match != 0)
	{
		*msg = m;
		return ASL_STATUS_OK;
	}

	*msg = NULL;
	asl_msg_release(m);
	return ASL_STATUS_MATCH_FAILED;
}

asl_msg_list_t *
asl_file_match(asl_file_t *s, asl_msg_list_t *qlist, uint64_t *last, uint64_t start, uint32_t count, uint32_t duration, int32_t direction)
{
	uint32_t status, d, i, do_match, did_match, rescount;
	asl_msg_t *m;
	struct timeval now, finish;
	asl_msg_list_t *out = NULL;

	if (s == NULL) return NULL;
	if ((s->flags & ASL_FILE_FLAG_READ) == 0) return NULL;

	if (s->version == 1)
	{
		asl_legacy1_match((asl_legacy1_t *)s->legacy, qlist, &out, last, start, count, direction);
		return out;
	}

	do_match = 1;
	if (qlist == NULL) do_match = 0;
	else if (qlist->count == 0) do_match = 0;

	rescount = 0;

	d = ASL_FILE_POSITION_NEXT;
	if (direction < 0) d = ASL_FILE_POSITION_PREVIOUS;

	/*
	 * find starting point
	 */
	status = ASL_STATUS_OK;
	if (direction >= 0) status = asl_file_read_set_position(s, ASL_FILE_POSITION_FIRST);
	else status = asl_file_read_set_position(s, ASL_FILE_POSITION_LAST);
	if (status != ASL_STATUS_OK) return NULL;

	while ((status == ASL_STATUS_OK) && (((direction >= 0) && (s->cursor_xid < start)) || ((direction < 0) && (s->cursor_xid > start))))
	{
		status = asl_file_read_set_position(s, d);
	}

	/* start the timer if a duration was specified */
	memset(&finish, 0, sizeof(struct timeval));
	if (duration != 0)
	{
		if (gettimeofday(&finish, NULL) == 0)
		{
			finish.tv_sec += (duration / USEC_PER_SEC);
			finish.tv_usec += (duration % USEC_PER_SEC);
			if (finish.tv_usec > USEC_PER_SEC)
			{
				finish.tv_usec -= USEC_PER_SEC;
				finish.tv_sec += 1;
			}
		}
		else
		{
			/* shouldn't happen, but if gettimeofday failed we just run without a timeout */
			memset(&finish, 0, sizeof(struct timeval));
		}
	}

	/*
	 * loop through records
	 */
	forever
	{
		m = NULL;
		status = asl_file_fetch_pos(s, s->cursor, direction, &m);
		if (status == ASL_STATUS_ACCESS_DENIED) continue;
		if (status != ASL_STATUS_OK) break;

		*last = s->cursor_xid;

		did_match = 1;

		if (do_match != 0)
		{
			did_match = 0;

			for (i = 0; (i < qlist->count) && (did_match == 0); i++)
			{
				did_match = asl_msg_cmp(qlist->msg[i], m);
			}
		}

		if (did_match == 1)
		{
			/*  append m to res */
			if (out == NULL)
			{
				out = asl_msg_list_new();
				if (out == NULL) return NULL;
			}

			asl_msg_list_append(out, m);
			rescount++;
			if ((count != 0) && (rescount >= count)) break;

			/* check the timer */
			if ((finish.tv_sec != 0) && (gettimeofday(&now, NULL) == 0))
			{
				if ((now.tv_sec > finish.tv_sec) || ((now.tv_sec == finish.tv_sec) && (now.tv_usec > finish.tv_usec))) break;
			}
		}

		asl_msg_release(m);
	}

	return out;
}

size_t
asl_file_size(asl_file_t *s)
{
	if (s == NULL) return 0;
	return s->file_size;
}

uint64_t
asl_file_ctime(asl_file_t *s)
{
	if (s == NULL) return 0;
	return s->dob;
}

void
asl_file_list_close(asl_file_list_t *head)
{
	asl_file_list_t *next;

	while (head != NULL)
	{
		next = head->next;
		asl_file_close(head->file);
		free(head);
		head = next;
	}
}

static void
asl_file_list_free(asl_file_list_t *head)
{
	asl_file_list_t *next;

	while (head != NULL)
	{
		next = head->next;
		free(head);
		head = next;
	}
}

static asl_file_list_t *
asl_file_list_insert(asl_file_list_t *list, asl_file_t *f, int32_t dir)
{
	asl_file_list_t *a, *b, *tmp;

	if (f == NULL) return list;

	tmp = (asl_file_list_t *)calloc(1, sizeof(asl_file_list_t));
	if (tmp == NULL) return NULL;
	tmp->file = f;

	if (list == NULL) return tmp;

	a = list;
	if (((dir < 0) && (f->cursor_xid > a->file->cursor_xid)) || ((dir >= 0) && (f->cursor_xid < a->file->cursor_xid)))
	{
		tmp->next = list;
		return tmp;
	}

	b = a->next;
	while (b != NULL)
	{
		if (((dir < 0) && (f->cursor_xid > b->file->cursor_xid)) || ((dir >= 0) && (f->cursor_xid < b->file->cursor_xid)))
		{
			tmp->next = b;
			a->next = tmp;
			return list;
		}

		a = b;
		b = a->next;
	}

	a->next = tmp;
	return list;
}

asl_file_list_t *
asl_file_list_add(asl_file_list_t *list, asl_file_t *f)
{
	asl_file_list_t *tmp;

	if (f == NULL) return list;
	if (f->version == 1) return list;

	tmp = (asl_file_list_t *)calloc(1, sizeof(asl_file_list_t));
	if (tmp == NULL) return NULL;
	tmp->file = f;

	tmp->next = list;
	return tmp;
}

void *
asl_file_list_match_start(asl_file_list_t *list, uint64_t start, int32_t direction)
{
	uint32_t status;
	asl_file_list_t *n;
	asl_file_match_token_t *out;

	if (list == NULL) return NULL;

	out = (asl_file_match_token_t *)calloc(1, sizeof(asl_file_match_token_t));
	if (out == NULL) return NULL;

	for (n = list; n != NULL; n = n->next)
	{
		/* init file for the search */
		status = asl_file_match_start(n->file, start, direction);
		if (status != ASL_STATUS_OK) continue;
		if (n->file->cursor_xid == 0) continue;

		out->list = asl_file_list_insert(out->list, n->file, direction);
	}

	out->dir = direction;
	return out;
}

ASL_STATUS
asl_file_list_match_next(void *token, asl_msg_list_t *qlist, asl_msg_list_t **res, uint32_t count)
{
	uint32_t status, rescount;
	asl_file_list_t *n;
	asl_msg_t *m;
	asl_file_match_token_t *work;
	uint64_t last;

	if (token == NULL) return ASL_STATUS_OK;
	if (res == NULL) return ASL_STATUS_INVALID_ARG;

	work = (asl_file_match_token_t *)token;

	rescount = 0;
	last = 0;

	while ((work->list != NULL) && ((rescount < count) || (count == 0)))
	{
		m = NULL;
		status = asl_file_match_next(work->list->file, qlist, &m, &last, work->dir);
		if (m != NULL)
		{
			if (*res == NULL) *res = asl_msg_list_new();
			if (*res == NULL)
			{
				asl_file_list_free(work->list);
				work->list = NULL;
				return ASL_STATUS_NO_MEMORY;
			}

			asl_msg_list_append(*res, m);
			asl_msg_release(m);
			rescount++;
		}

		if ((status != ASL_STATUS_OK) || (work->list->file->cursor_xid == 0))
		{
			n = work->list->next;
			free(work->list);
			work->list = n;
		}

		if (work->list != NULL)
		{
			n = work->list->next;
			if (n != NULL)
			{
				if (((work->dir < 0) && (work->list->file->cursor_xid <= n->file->cursor_xid)) || ((work->dir >= 0) && (work->list->file->cursor_xid > n->file->cursor_xid)))
				{
					n = work->list;
					work->list = work->list->next;
					n->next = NULL;
					work->list = asl_file_list_insert(work->list, n->file, work->dir);
					free(n);
				}
			}
		}
	}

	return ASL_STATUS_OK;
}

void
asl_file_list_match_end(void *token)
{
	asl_file_match_token_t *work;

	if (token == NULL) return;

	work = (asl_file_match_token_t *)token;
	asl_file_list_free(work->list);
	work->list = NULL;

	free(token);
}

asl_msg_list_t *
asl_file_list_match(asl_file_list_t *list, asl_msg_list_t *qlist, uint64_t *last, uint64_t start, uint32_t count, uint32_t duration, int32_t direction)
{
	uint32_t status, rescount;
	asl_file_list_t *files, *n;
	asl_msg_t *m;
	struct timeval now, finish;
	asl_msg_list_t *out = NULL;

	if (list == NULL) return NULL;
	if (last == NULL) return NULL;

	files = NULL;

	for (n = list; n != NULL; n = n->next)
	{
		/* init file for the search */
		status = asl_file_match_start(n->file, start, direction);
		if (status != ASL_STATUS_OK) continue;
		if (n->file->cursor_xid == 0) continue;

		files = asl_file_list_insert(files, n->file, direction);
	}

	if (files == NULL)
	{
		asl_file_list_free(files);
		return NULL;
	}

	/* start the timer if a timeout was specified */
	memset(&finish, 0, sizeof(struct timeval));
	if (duration != 0)
	{
		if (gettimeofday(&finish, NULL) == 0)
		{
			finish.tv_sec += (duration / USEC_PER_SEC);
			finish.tv_usec += (duration % USEC_PER_SEC);
			if (finish.tv_usec > USEC_PER_SEC)
			{
				finish.tv_usec -= USEC_PER_SEC;
				finish.tv_sec += 1;
			}
		}
		else
		{
			/* shouldn't happen, but if gettimeofday failed we just run without a timeout */
			memset(&finish, 0, sizeof(struct timeval));
		}
	}

	rescount = 0;
	while ((files != NULL) && ((rescount < count) || (count == 0)))
	{
		m = NULL;
		status = asl_file_match_next(files->file, qlist, &m, last, direction);
		if (m != NULL)
		{
			if (out == NULL) out = asl_msg_list_new();
			if (out == NULL)
			{
				asl_file_list_free(files);
				return NULL;
			}

			asl_msg_list_append(out, m);
			asl_msg_release(m);
			rescount++;
		}

		if (files->file->cursor_xid == 0)
		{
			n = files->next;
			free(files);
			files = n;
		}

		if (files != NULL)
		{
			n = files->next;
			if (n != NULL)
			{
				if (((direction < 0) && (files->file->cursor_xid <= n->file->cursor_xid)) || ((direction >= 0) && (files->file->cursor_xid > n->file->cursor_xid)))
				{
					n = files;
					files = files->next;
					n->next = NULL;
					files = asl_file_list_insert(files, n->file, direction);
					free(n);
				}
			}
		}

		/* check the timer */
		if ((finish.tv_sec != 0) && (gettimeofday(&now, NULL) == 0))
		{
			if ((now.tv_sec > finish.tv_sec) || ((now.tv_sec == finish.tv_sec) && (now.tv_usec > finish.tv_usec))) break;
		}
	}

	asl_file_list_free(files);
	return out;
}

#pragma mark -
#pragma mark asl_object support

static void
_jump_dealloc(asl_object_private_t *obj)
{
	_asl_file_free_internal((asl_file_t *)obj);
}

static size_t
_jump_count(asl_object_private_t *obj)
{
	asl_file_t *s = (asl_file_t *)obj;
	if (s == NULL) return 0;
	if ((s->flags & ASL_FILE_FLAG_READ) == 0) return 0;

	uint64_t cursor = s->cursor;
	uint64_t cursor_xid = s->cursor_xid;

	if (asl_file_read_set_position_last((asl_file_t *)obj, 1) != ASL_STATUS_OK) return 0;

	s->cursor = cursor;
	s->cursor_xid = cursor_xid;
	return s->msg_count;
}

static asl_object_private_t *
_jump_next(asl_object_private_t *obj)
{
	asl_msg_t *msg = NULL;
	if (asl_file_fetch_next((asl_file_t *)obj, &msg) != ASL_STATUS_OK) return NULL;
	return (asl_object_private_t *)msg;
}

static asl_object_private_t *
_jump_prev(asl_object_private_t *obj)
{
	asl_msg_t *msg = NULL;
	if (asl_file_fetch_previous((asl_file_t *)obj, &msg) != ASL_STATUS_OK) return NULL;
	return (asl_object_private_t *)msg;
}

/* we don't really need to support this, but we are generous */
static asl_object_private_t *
_jump_get_object_at_index(asl_object_private_t *obj, size_t n)
{
	asl_msg_t *msg = NULL;
	uint64_t mid = n;
	if (asl_file_fetch((asl_file_t *)obj, mid, &msg) != ASL_STATUS_OK) return NULL;
	return (asl_object_private_t *)msg;
}

static void
_jump_set_iteration_index(asl_object_private_t *obj, size_t n)
{
	asl_file_t *s = (asl_file_t *)obj;
	if (s == NULL) return;
	if ((s->flags & ASL_FILE_FLAG_READ) == 0) return;

	if (n == 0)
	{
		asl_file_read_set_position_first(s);
	}
	else if (n == SIZE_MAX)
	{
		asl_file_read_set_position_last(s, 0);
	}
	else
	{
		/* we don't really need to support this, but we are generous */
		asl_file_fetch(s, n, NULL);
	}
}

static void
_jump_append(asl_object_private_t *obj, asl_object_private_t *newobj, void *addr)
{
	uint64_t xid;
	asl_file_t *s = (asl_file_t *)obj;
	int type = asl_get_type((asl_object_t)newobj);
	if (s == NULL) return;
	if (s->flags & ASL_FILE_FLAG_READ) return;

	if (type == ASL_TYPE_LIST)
	{
		asl_msg_t *msg;
		asl_msg_list_reset_iteration((asl_msg_list_t *)newobj, 0);
		while (NULL != (msg = asl_msg_list_next((asl_msg_list_t *)newobj)))
		{
			if (asl_file_save(s, msg, &xid) != ASL_STATUS_OK) return;
		}
	}
	else if ((type == ASL_TYPE_MSG) || (type == ASL_TYPE_QUERY))
	{
		asl_file_save(s, (asl_msg_t *)newobj, &xid);
	}
}

static asl_object_private_t *
_jump_search(asl_object_private_t *obj, asl_object_private_t *query)
{
	asl_file_t *s = (asl_file_t *)obj;
	int type = asl_get_type((asl_object_t)query);
	asl_msg_list_t *out = NULL;
	asl_msg_list_t *ql = NULL;
	uint64_t last;

	if (query == NULL)
	{
		return (asl_object_private_t *)asl_file_match(s, NULL, &last, 0, 0, 0, 1);
	}
	else if (type == ASL_TYPE_LIST)
	{
		return (asl_object_private_t *)asl_file_match(s, (asl_msg_list_t *)query, &last, 0, 0, 0, 1);
	}
	else if ((type == ASL_TYPE_MSG) || (type == ASL_TYPE_QUERY))
	{
		ql = asl_msg_list_new();
		asl_msg_list_append(ql, query);

		out = asl_file_match(s, ql, &last, 0, 0, 0, 1);
		asl_msg_list_release(ql);
		return (asl_object_private_t *)out;
	}

	return NULL;
}

static asl_object_private_t *
_jump_match(asl_object_private_t *obj, asl_object_private_t *qlist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t dir)
{
	uint64_t x;
	asl_msg_list_t *out = asl_file_match((asl_file_t *)obj, (asl_msg_list_t *)qlist, &x, start, count, duration, dir);
	*last = x;
	return (asl_object_private_t *)out;
}

__private_extern__ const asl_jump_table_t *
asl_file_jump_table()
{
	static const asl_jump_table_t jump =
	{
		.alloc = NULL,
		.dealloc = &_jump_dealloc,
		.set_key_val_op = NULL,
		.unset_key = NULL,
		.get_val_op_for_key = NULL,
		.get_key_val_op_at_index = NULL,
		.count = &_jump_count,
		.next = &_jump_next,
		.prev = &_jump_prev,
		.get_object_at_index = &_jump_get_object_at_index,
		.set_iteration_index = &_jump_set_iteration_index,
		.remove_object_at_index = NULL,
		.append = &_jump_append,
		.prepend = NULL,
		.search = &_jump_search,
		.match = &_jump_match
	};

	return &jump;
}
