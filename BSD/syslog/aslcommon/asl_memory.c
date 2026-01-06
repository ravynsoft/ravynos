/*
 * Copyright (c) 2007-2010 Apple Inc. All rights reserved.
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <asl_core.h>
#include <asl_msg.h>
#include <asl_msg_list.h>
#include <asl_private.h>
#include "asl_memory.h"

#define DEFAULT_MAX_RECORDS 2000
#define DEFAULT_MAX_STRING_MEMORY 4096000

#define forever for(;;)

uint32_t
asl_memory_statistics(asl_memory_t *s, asl_msg_t **msg)
{
	asl_msg_t * out;
	uint32_t i, n;
	uint64_t size;
	char str[256];

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;

	out = asl_msg_new(ASL_TYPE_MSG);
	if (out == NULL) return ASL_STATUS_NO_MEMORY;

	size = sizeof(asl_memory_t);
	size += ((s->record_count + 1) * sizeof(mem_record_t));

	for (i = 0; i < s->string_count; i++)
	{
		size += sizeof(mem_string_t);
		if (s->string_cache[i]->str != NULL) size += (strlen(s->string_cache[i]->str) + 1);
	}

	snprintf(str, sizeof(str), "%llu", size);
	asl_msg_set_key_val(out, "Size", str);

	n = 0;
	for (i = 0; i < s->record_count; i++) if (s->record[i]->mid != 0) n++;

	snprintf(str, sizeof(str), "%u", s->record_count);
	asl_msg_set_key_val(out, "MaxRecords", str);

	snprintf(str, sizeof(str), "%u", n);
	asl_msg_set_key_val(out, "RecordCount", str);

	snprintf(str, sizeof(str), "%u", s->string_count);
	asl_msg_set_key_val(out, "StringCount", str);

	snprintf(str, sizeof(str), "%lu", s->curr_string_mem);
	asl_msg_set_key_val(out, "StringMemory", str);

	snprintf(str, sizeof(str), "%lu", s->max_string_mem);
	asl_msg_set_key_val(out, "MaxStringMemory", str);

	*msg = out;
	return ASL_STATUS_OK;
}

uint32_t
asl_memory_close(asl_memory_t *s)
{
	if (s == NULL) return ASL_STATUS_OK;
	
	dispatch_sync(s->queue, ^{
		uint32_t i;

		if (s->record != NULL)
		{
			for (i = 0; i < s->record_count; i++)
			{
				free(s->record[i]);
				s->record[i] = NULL;
			}

			free(s->record);
			s->record = NULL;
		}

		free(s->buffer_record);
		s->buffer_record = NULL;

		if (s->string_cache != NULL)
		{
			for (i = 0; i < s->string_count; i++)
			{
				if (s->string_cache[i] != NULL)
				{
					free(s->string_cache[i]->str);
					free(s->string_cache[i]);
				}

				s->string_cache[i] = NULL;
			}

			free(s->string_cache);
			s->string_cache = NULL;
		}
	});

	dispatch_release(s->queue);

	free(s);

	return ASL_STATUS_OK;
}

uint32_t
asl_memory_open(uint32_t max_records, size_t max_str_mem, asl_memory_t **s)
{
	asl_memory_t *out;
	uint32_t i;

	if (s == NULL) return ASL_STATUS_INVALID_ARG;

	if (max_records == 0) max_records = DEFAULT_MAX_RECORDS;
	if (max_str_mem == 0) max_str_mem = DEFAULT_MAX_STRING_MEMORY;

	out = calloc(1, sizeof(asl_memory_t));
	if (out == NULL) return ASL_STATUS_NO_MEMORY;

	out->queue = dispatch_queue_create("ASL Memory Queue", NULL);
	if (out->queue == NULL)
	{
		free(out);
		return ASL_STATUS_NO_MEMORY;
	}

	out->max_string_mem = max_str_mem;

	out->record_count = max_records;
	out->record = (mem_record_t **)calloc(max_records, sizeof(mem_record_t *));
	if (out->record == NULL)
	{
		dispatch_release(out->queue);
		free(out);
		return ASL_STATUS_NO_MEMORY;
	}

	for (i = 0; i < max_records; i++)
	{
		out->record[i] = (mem_record_t *)calloc(1, sizeof(mem_record_t));
		if (out->record[i] == NULL)
		{
			asl_memory_close(out);
			return ASL_STATUS_NO_MEMORY;
		}
	}

	out->buffer_record = (mem_record_t *)calloc(1, sizeof(mem_record_t));
	if (out->buffer_record == NULL)
	{
		asl_memory_close(out);
		return ASL_STATUS_NO_MEMORY;
	}

	*s = out;
	return ASL_STATUS_OK;
}

static void
asl_memory_reset(asl_memory_t *s)
{
	uint32_t i;
	if (s == NULL) return;

	/* clear all message records */
	for (i = 0; i < s->record_count; i++)
	{
		memset(s->record[i], 0, sizeof(mem_record_t));
	}

	/* reset the string cache */
	if (s->string_cache != NULL)
	{
		for (i = 0; i < s->string_count; i++)
		{
			if (s->string_cache[i] != NULL)
			{
				free(s->string_cache[i]->str);
				free(s->string_cache[i]);
			}
			
			s->string_cache[i] = NULL;
		}
		
		free(s->string_cache);
		s->string_cache = NULL;
	}
	
	s->string_count = 0;
}

static mem_string_t *
asl_memory_string_new(const char *str, uint32_t len, uint32_t hash)
{
	mem_string_t *out;

	if (str == NULL) return NULL;

	out = (mem_string_t *)calloc(1, sizeof(mem_string_t));
	if (out == NULL) return NULL;

	out->hash = hash;
	out->refcount = 1;
	out->str = malloc(len + 1);
	if (out->str == NULL)
	{
		free(out);
		return NULL;
	}
	
	memcpy(out->str, str, len);
	out->str[len] = 0;

	return out;
}

/*
 * Find the first hash greater than or equal to a given hash in the string cache.
 * Return s->string_count if hash is greater than last hash in the string cache.
 * Caller must check if the hashes match or not.
 *
 * This routine is used both to find strings in the cache and to determine where to insert
 * new strings.  Note that the caller needs to do extra work after calling this routine.
 */
static uint32_t
asl_memory_string_cache_search_hash(asl_memory_t *s, uint32_t hash)
{
	uint32_t top, bot, mid, range;
	mem_string_t *ms;

	if (s->string_count == 0) return 0;
	if (s->string_count == 1)
	{
		ms = s->string_cache[0];
		if (hash < ms->hash) return 0;
		return 1;
	}

	range = top = s->string_count - 1;
	bot = 0;
	mid = top / 2;

	while (range > 1)
	{
		ms = s->string_cache[mid];

		if (hash == ms->hash)
		{
			while (mid > 0)
			{
				ms = s->string_cache[mid - 1];
				if (hash != ms->hash) break;
				mid--;
			}

			return mid;
		}
		else
		{
			ms = s->string_cache[mid];
			if (hash < ms->hash) top = mid;
			else bot = mid;
		}

		range = top - bot;
		mid = bot + (range / 2);
	}

	ms = s->string_cache[bot];
	if (hash <= ms->hash) return bot;

	ms = s->string_cache[top];
	if (hash <= ms->hash) return top;

	return s->string_count;
}

/*
 * Search the string cache.
 * If the string is in the cache, increment refcount and return it.
 * If the string is not in cache and create flag is on, create a new string.
 * Otherwise, return NULL.
 */
static mem_string_t *
asl_memory_string_retain(asl_memory_t *s, const char *str, int create)
{
	uint32_t i, where, hash, len;
	mem_string_t *new;

	if (s == NULL) return NULL;
	if (str == NULL) return NULL;
	len = strlen(str);

	/* check the cache */
	hash = asl_core_string_hash(str, len);
	where = asl_memory_string_cache_search_hash(s, hash);

	/* asl_memory_string_cache_search_hash just tells us where to look */
	if (where < s->string_count)
	{
		while (s->string_cache[where]->hash == hash)
		{
			if (!strcmp(str, s->string_cache[where]->str))
			{
				s->string_cache[where]->refcount++;
				return s->string_cache[where];
			}

			where++;
		}
	}

	/* not found */
	if (create == 0) return NULL;

	/* create a new mem_string_t and insert into the cache at index 'where' */
	new = asl_memory_string_new(str, len, hash);
	if (new == NULL) return NULL;
	
	s->string_cache = (mem_string_t **)reallocf(s->string_cache, (s->string_count + 1) * sizeof(void *));
	if (s->string_cache == NULL)
	{
		s->string_count = 0;
		free(new);
		return NULL;
	}

	for (i = s->string_count; i > where; i--) s->string_cache[i] = s->string_cache[i - 1];

	s->curr_string_mem += (sizeof(mem_string_t) + len + 1);
	s->string_cache[where] = new;
	s->string_count++;

	return s->string_cache[where];
}

static uint32_t
asl_memory_string_release(asl_memory_t *s, mem_string_t *m)
{
	uint32_t i, where;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (m == NULL) return ASL_STATUS_OK;

	if (m->refcount > 0) m->refcount--;
	if (m->refcount > 0) return ASL_STATUS_OK;

	where = asl_memory_string_cache_search_hash(s, m->hash);
	if (s->string_cache[where]->hash != m->hash) return ASL_STATUS_OK;

	while (s->string_cache[where] != m)
	{
		if (s->string_cache[where]->hash != m->hash) return ASL_STATUS_OK;

		where++;
		if (where >= s->string_count) return ASL_STATUS_OK;
	}

	for (i = where + 1; i < s->string_count; i++) s->string_cache[i - 1] = s->string_cache[i];

	if (m->str == NULL) s->curr_string_mem -= sizeof(mem_string_t);
	else s->curr_string_mem -= (sizeof(mem_string_t) + strlen(m->str) + 1);

	free(m->str);
	free(m);

	s->string_count--;

	if (s->string_count == 0)
	{
		free(s->string_cache);
		s->string_cache = NULL;
		return ASL_STATUS_OK;
	}

	s->string_cache = (mem_string_t **)reallocf(s->string_cache, s->string_count * sizeof(void *));
	if (s->string_cache == NULL)
	{
		s->string_count = 0;
		return ASL_STATUS_NO_MEMORY;
	}

	return ASL_STATUS_OK;
}

/*
 * Release all a record's strings and reset it's values
 */
static void
asl_memory_record_clear(asl_memory_t *s, mem_record_t *r)
{
	uint32_t i;

	if (s == NULL) return;
	if (r == NULL) return;

	asl_memory_string_release(s, r->host);
	asl_memory_string_release(s, r->sender);
	asl_memory_string_release(s, r->sender_mach_uuid);
	asl_memory_string_release(s, r->facility);
	asl_memory_string_release(s, r->message);
	asl_memory_string_release(s, r->refproc);
	asl_memory_string_release(s, r->session);

	for (i = 0; i < r->kvcount; i++) asl_memory_string_release(s, r->kvlist[i]);

	if (r->kvlist != NULL) free(r->kvlist);
	memset(r, 0, sizeof(mem_record_t));
}

static void
asl_memory_record_free(asl_memory_t *s, mem_record_t *r)
{
	asl_memory_record_clear(s, r);
	free(r);
}

/*
 * Encode an asl_msg_t as a record structure.
 * Creates and caches strings.
 */
static uint32_t
asl_memory_message_encode(asl_memory_t *s, asl_msg_t *msg)
{
	uint32_t x;
	mem_string_t *k, *v;
	mem_record_t *r;
	const char *key, *val;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->buffer_record == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_MESSAGE;

	r = s->buffer_record;

	memset(r, 0, sizeof(mem_record_t));

	r->flags = 0;
	r->level = ASL_LEVEL_DEBUG;
	r->pid = -1;
	r->uid = -2;
	r->gid = -2;
	r->ruid = -1;
	r->rgid = -1;
	r->time = (uint64_t)-1;
	r->nano = (uint32_t)-1;

	key = NULL;
	val = NULL;

	for (x = asl_msg_fetch((asl_msg_t *)msg, 0, &key, &val, NULL); x != IndexNull; x = asl_msg_fetch((asl_msg_t *)msg, x, &key, &val, NULL))
	{
		if (key == NULL) continue;

		else if (!strcmp(key, ASL_KEY_TIME))
		{
			if (val != NULL) r->time = asl_core_parse_time(val, NULL);
		}
		else if (!strcmp(key, ASL_KEY_TIME_NSEC))
		{
			if (val != NULL) r->nano = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_HOST))
		{
			if (val != NULL) r->host = asl_memory_string_retain(s, val, 1);
		}
		else if (!strcmp(key, ASL_KEY_SENDER))
		{
			if (val != NULL) r->sender = asl_memory_string_retain(s, val, 1);
		}
		else if (!strcmp(key, ASL_KEY_PID))
		{
			if (val != NULL) r->pid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_REF_PID))
		{
			if (val != NULL) r->refpid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_UID))
		{
			if (val != NULL) r->uid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_GID))
		{
			if (val != NULL) r->gid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_LEVEL))
		{
			if (val != NULL) r->level = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_MSG))
		{
			if (val != NULL) r->message = asl_memory_string_retain(s, val, 1);
		}
		else if (!strcmp(key, ASL_KEY_SENDER_MACH_UUID))
		{
			if (val != NULL) r->sender_mach_uuid = asl_memory_string_retain(s, val, 1);
		}
		else if (!strcmp(key, ASL_KEY_FACILITY))
		{
			if (val != NULL) r->facility = asl_memory_string_retain(s, val, 1);
		}
		else if (!strcmp(key, ASL_KEY_REF_PROC))
		{
			if (val != NULL) r->refproc = asl_memory_string_retain(s, val, 1);
		}
		else if (!strcmp(key, ASL_KEY_SESSION))
		{
			if (val != NULL) r->session = asl_memory_string_retain(s, val, 1);
		}
		else if (!strcmp(key, ASL_KEY_READ_UID))
		{
			if (((r->flags & ASL_MSG_FLAG_READ_UID_SET) == 0) && (val != NULL))
			{
				r->ruid = atoi(val);
				r->flags |= ASL_MSG_FLAG_READ_UID_SET;
			}
		}
		else if (!strcmp(key, ASL_KEY_READ_GID))
		{
			if (((r->flags & ASL_MSG_FLAG_READ_GID_SET) == 0) && (val != NULL))
			{
				r->rgid = atoi(val);
				r->flags |= ASL_MSG_FLAG_READ_GID_SET;
			}
		}
		else if (!strcmp(key, ASL_KEY_OS_ACTIVITY_ID))
		{
			if (val != NULL) r->os_activity_id = atoll(val);
		}
		else if (!strcmp(key, ASL_KEY_MSG_ID))
		{
			/* Ignore */
			continue;
		}
		else
		{
			k = asl_memory_string_retain(s, key, 1);
			if (k == NULL) continue;

			v = NULL;
			if (val != NULL) v = asl_memory_string_retain(s, val, 1);

			r->kvlist = (mem_string_t **)reallocf(r->kvlist, (r->kvcount + 2) * sizeof(mem_string_t *));
			if (r->kvlist == NULL)
			{
				asl_memory_record_clear(s, r);
				return ASL_STATUS_NO_MEMORY;
			}

			r->kvlist[r->kvcount++] = k;
			r->kvlist[r->kvcount++] = v;
		}
	}

	return ASL_STATUS_OK;
}

uint32_t
asl_memory_save(asl_memory_t *s, asl_msg_t *msg, uint64_t *mid)
{
	__block uint32_t status;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->buffer_record == NULL) return ASL_STATUS_INVALID_STORE;

	dispatch_sync(s->queue, ^{
		mem_record_t *t;

		/* asl_memory_message_encode creates and caches strings */
		status = asl_memory_message_encode(s, msg);
		if (status == ASL_STATUS_OK)
		{
			uint32_t loop_start_index = s->record_first;
	
			if (*mid != 0)
			{
				s->buffer_record->mid = *mid;
			}
			else
			{
				s->buffer_record->mid = asl_core_new_msg_id(0);
				*mid = s->buffer_record->mid;
			}
			
			/* clear the first record */
			t = s->record[s->record_first];
			asl_memory_record_clear(s, t);
			
			/* add the new record to the record list (swap in the buffer record) */
			s->record[s->record_first] = s->buffer_record;
			s->buffer_record = t;
			
			/* record list is a circular queue */
			s->record_first++;
			if (s->record_first >= s->record_count) s->record_first = 0;
			
			/* delete records if too much memory is in use */
			while (s->curr_string_mem > s->max_string_mem)
			{
				asl_memory_record_clear(s, s->record[s->record_first]);
				s->record_first++;
				if (s->record_first >= s->record_count) s->record_first = 0;
				if (s->record_first == loop_start_index)
				{
					/* The entire ring has been cleared.  This should never happen. */
					asl_memory_reset(s);
					status = ASL_STATUS_FAILED;
					break;
				}
			}
		}
	});

	return status;
}

/*
 * Decodes a record structure.
 */
static uint32_t
asl_memory_message_decode(asl_memory_t *s, mem_record_t *r, asl_msg_t **out)
{
	uint32_t i;
	asl_msg_t *msg;
	char tmp[64];
	const char *key, *val;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (r == NULL) return ASL_STATUS_INVALID_ARG;
	if (out == NULL) return ASL_STATUS_INVALID_ARG;

	*out = NULL;

	msg = asl_msg_new(ASL_TYPE_MSG);
	if (msg == NULL) return ASL_STATUS_NO_MEMORY;

	/* Message ID */
	snprintf(tmp, sizeof(tmp), "%llu", r->mid);
	asl_msg_set_key_val(msg, ASL_KEY_MSG_ID, tmp);

	/* Level */
	snprintf(tmp, sizeof(tmp), "%u", r->level);
	asl_msg_set_key_val(msg, ASL_KEY_LEVEL, tmp);

	/* Time */
	if (r->time != (uint64_t)-1)
	{
		snprintf(tmp, sizeof(tmp), "%llu", r->time);
		asl_msg_set_key_val(msg, ASL_KEY_TIME, tmp);
	}

	/* Nanoseconds */
	if (r->nano != (uint32_t)-1)
	{
		snprintf(tmp, sizeof(tmp), "%u", r->nano);
		asl_msg_set_key_val(msg, ASL_KEY_TIME_NSEC, tmp);
	}

	/* Host */
	if (r->host != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_HOST, r->host->str);
	}

	/* Sender */
	if (r->sender != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_SENDER, r->sender->str);
	}

	/* Sender mach UUID */
	if (r->sender_mach_uuid != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_SENDER_MACH_UUID, r->sender_mach_uuid->str);
	}

	/* Facility */
	if (r->facility != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_FACILITY, r->facility->str);
	}

	/* Ref Proc */
	if (r->refproc != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_REF_PROC, r->refproc->str);
	}

	/* Session */
	if (r->session != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_SESSION, r->session->str);
	}

	/* PID */
	if (r->pid != -1)
	{
		snprintf(tmp, sizeof(tmp), "%d", r->pid);
		asl_msg_set_key_val(msg, ASL_KEY_PID, tmp);
	}

	/* REF PID */
	if (r->refpid != 0)
	{
		snprintf(tmp, sizeof(tmp), "%d", r->refpid);
		asl_msg_set_key_val(msg, ASL_KEY_REF_PID, tmp);
	}

	/* UID */
	if (r->uid != -2)
	{
		snprintf(tmp, sizeof(tmp), "%d", r->uid);
		asl_msg_set_key_val(msg, ASL_KEY_UID, tmp);
	}

	/* GID */
	if (r->gid != -2)
	{
		snprintf(tmp, sizeof(tmp), "%d", r->gid);
		asl_msg_set_key_val(msg, ASL_KEY_GID, tmp);
	}

	/* Message */
	if (r->message != NULL)
	{
		asl_msg_set_key_val(msg, ASL_KEY_MSG, r->message->str);
	}

	/* ReadUID */
	if (r->flags & ASL_MSG_FLAG_READ_UID_SET)
	{
		snprintf(tmp, sizeof(tmp), "%d", r->ruid);
		asl_msg_set_key_val(msg, ASL_KEY_READ_UID, tmp);
	}

	/* ReadGID */
	if (r->flags & ASL_MSG_FLAG_READ_GID_SET)
	{
		snprintf(tmp, sizeof(tmp), "%d", r->rgid);
		asl_msg_set_key_val(msg, ASL_KEY_READ_GID, tmp);
	}

	/* OSActivityID */
	if (r->os_activity_id != 0)
	{
		snprintf(tmp, sizeof(tmp), "%llu", r->os_activity_id);
		asl_msg_set_key_val(msg, ASL_KEY_OS_ACTIVITY_ID, tmp);
	}

	/* Key - Value List */
	for (i = 0; i < r->kvcount; i++)
	{
		key = NULL;
		val = NULL;

		if (r->kvlist[i] != NULL) key = r->kvlist[i]->str;
		i++;
		if (r->kvlist[i] != NULL) val = r->kvlist[i]->str;

		if (key != NULL) asl_msg_set_key_val(msg, key, val);
	}

	*out = msg;
	return ASL_STATUS_OK;
}

uint32_t
asl_memory_fetch(asl_memory_t *s, uint64_t mid, asl_msg_t **msg, int32_t ruid, int32_t rgid)
{
	__block uint32_t status;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;

	status = ASL_STATUS_INVALID_ID;

	dispatch_sync(s->queue, ^{
		uint32_t i;

		for (i = 0; i < s->record_count; i++)
		{
			if (s->record[i]->mid == 0) break;

			if (s->record[i]->mid == mid)
			{
				status = asl_core_check_access(s->record[i]->ruid, s->record[i]->rgid, ruid, rgid, s->record[i]->flags);
				if (status != ASL_STATUS_OK) break;

				status = asl_memory_message_decode(s, s->record[i], msg);
				break;
			}
		}
	});

	return status;
}

static mem_record_t *
asl_memory_query_to_record(asl_memory_t *s, asl_msg_t *q, uint32_t *type)
{
	mem_record_t *out;
	uint32_t i, x;
	uint16_t op;
	mem_string_t *mkey, *mval;
	const char *key, *val;

	if (type == NULL) return NULL;

	if (s == NULL)
	{
		*type = ASL_QUERY_MATCH_ERROR;
		return NULL;
	}

	/* NULL query matches anything */
	*type = ASL_QUERY_MATCH_TRUE;
	if (q == NULL) return NULL;
	if (asl_msg_count((asl_msg_t *)q) == 0) return NULL;


	/* we can only do fast match on equality tests */
	*type = ASL_QUERY_MATCH_SLOW;

	for (x = asl_msg_fetch((asl_msg_t *)q, 0, NULL, NULL, &op); x != IndexNull; x = asl_msg_fetch((asl_msg_t *)q, x, NULL, NULL, &op))
	{
		if (op != ASL_QUERY_OP_EQUAL) return NULL;
	}

	out = (mem_record_t *)calloc(1, sizeof(mem_record_t));
	if (out == NULL)
	{
		*type = ASL_QUERY_MATCH_ERROR;
		return NULL;
	}

	for (x = asl_msg_fetch((asl_msg_t *)q, 0, &key, &val, &op); x != IndexNull; x = asl_msg_fetch((asl_msg_t *)q, x, &key, &val, &op))
	{
		if (key == NULL) continue;

		else if (!strcmp(key, ASL_KEY_MSG_ID))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_MSG_ID)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_MSG_ID;
			out->mid = atoll(val);
		}
		else if (!strcmp(key, ASL_KEY_TIME))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_TIME)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_TIME;
			out->time = asl_core_parse_time(val, NULL);
		}
		else if (!strcmp(key, ASL_KEY_TIME_NSEC))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_NANO)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_NANO;
			out->nano = atoll(val);
		}
		else if (!strcmp(key, ASL_KEY_LEVEL))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_LEVEL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_LEVEL;
			out->level = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_PID))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_PID)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_PID;
			out->pid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_UID))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_UID)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_UID;
			out->uid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_GID))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_GID)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_GID;
			out->gid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_READ_UID))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_RUID)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_RUID;
			out->ruid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_READ_GID))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_RGID)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_RGID;
			out->rgid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_REF_PID))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_REF_PID)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_REF_PID;
			out->refpid = atoi(val);
		}
		else if (!strcmp(key, ASL_KEY_HOST))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_HOST)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_HOST;
			out->host = asl_memory_string_retain(s, val, 0);
			if (out->host == NULL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_FALSE;
				return NULL;
			}
		}
		else if (!strcmp(key, ASL_KEY_SENDER))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_SENDER)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_SENDER;
			out->sender = asl_memory_string_retain(s, val, 0);
			if (out->sender == NULL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_FALSE;
				return NULL;
			}
		}
		else if (!strcmp(key, ASL_KEY_SENDER_MACH_UUID))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_SMUUID)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_SMUUID;
			out->sender = asl_memory_string_retain(s, val, 0);
			if (out->sender_mach_uuid == NULL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_FALSE;
				return NULL;
			}
		}
		else if (!strcmp(key, ASL_KEY_FACILITY))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_FACILITY)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_FACILITY;
			out->facility = asl_memory_string_retain(s, val, 0);
			if (out->facility == NULL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_FALSE;
				return NULL;
			}
		}
		else if (!strcmp(key, ASL_KEY_MSG))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_MESSAGE)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_MESSAGE;
			out->message = asl_memory_string_retain(s, val, 0);
			if (out->message == NULL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_FALSE;
				return NULL;
			}
		}
		else if (!strcmp(key, ASL_KEY_REF_PROC))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_REF_PROC)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_REF_PROC;
			out->refproc = asl_memory_string_retain(s, val, 0);
			if (out->refproc == NULL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_FALSE;
				return NULL;
			}
		}
		else if (!strcmp(key, ASL_KEY_SESSION))
		{
			if (val == NULL) continue;

			if (*type & ASL_QUERY_MATCH_SESSION)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_SLOW;
				return NULL;
			}

			*type |= ASL_QUERY_MATCH_SESSION;
			out->session = asl_memory_string_retain(s, val, 0);
			if (out->session == NULL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_FALSE;
				return NULL;
			}
		}
		else
		{
			mkey = asl_memory_string_retain(s, key, 0);
			if (mkey == NULL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_FALSE;
				return NULL;
			}

			for (i = 0; i < out->kvcount; i += 2)
			{
				if (out->kvlist[i] == mkey)
				{
					asl_memory_record_free(s, out);
					*type = ASL_QUERY_MATCH_SLOW;
					return NULL;
				}
			}

			mval = asl_memory_string_retain(s, val, 0);

			if (out->kvcount == 0)
			{
				out->kvlist = (mem_string_t **)calloc(2, sizeof(mem_string_t *));
			}
			else
			{
				out->kvlist = (mem_string_t **)reallocf(out->kvlist, (out->kvcount + 2) * sizeof(mem_string_t *));
			}

			if (out->kvlist == NULL)
			{
				asl_memory_record_free(s, out);
				*type = ASL_QUERY_MATCH_ERROR;
				return NULL;
			}

			out->kvlist[out->kvcount++] = mkey;
			out->kvlist[out->kvcount++] = mval;
		}
	}

	return out;
}

static uint32_t
asl_memory_fast_match(asl_memory_t *s, mem_record_t *r, uint32_t qtype, mem_record_t *q)
{
	uint32_t i, j;

	if (s == NULL) return 0;
	if (r == NULL) return 0;
	if (q == NULL) return 1;

	if ((qtype & ASL_QUERY_MATCH_MSG_ID) && (q->mid != r->mid)) return 0;
	if ((qtype & ASL_QUERY_MATCH_TIME) && (q->time != r->time)) return 0;
	if ((qtype & ASL_QUERY_MATCH_NANO) && (q->nano != r->nano)) return 0;
	if ((qtype & ASL_QUERY_MATCH_LEVEL) && (q->level != r->level)) return 0;
	if ((qtype & ASL_QUERY_MATCH_PID) && (q->pid != r->pid)) return 0;
	if ((qtype & ASL_QUERY_MATCH_UID) && (q->uid != r->uid)) return 0;
	if ((qtype & ASL_QUERY_MATCH_GID) && (q->gid != r->gid)) return 0;
	if ((qtype & ASL_QUERY_MATCH_RUID) && (q->ruid != r->ruid)) return 0;
	if ((qtype & ASL_QUERY_MATCH_RGID) && (q->rgid != r->rgid)) return 0;
	if ((qtype & ASL_QUERY_MATCH_REF_PID) && (q->refpid != r->refpid)) return 0;
	if ((qtype & ASL_QUERY_MATCH_HOST) && (q->host != r->host)) return 0;
	if ((qtype & ASL_QUERY_MATCH_SENDER) && (q->sender != r->sender)) return 0;
	if ((qtype & ASL_QUERY_MATCH_SMUUID) && (q->sender_mach_uuid != r->sender_mach_uuid)) return 0;
	if ((qtype & ASL_QUERY_MATCH_FACILITY) && (q->facility != r->facility)) return 0;
	if ((qtype & ASL_QUERY_MATCH_MESSAGE) && (q->message != r->message)) return 0;
	if ((qtype & ASL_QUERY_MATCH_REF_PROC) && (q->refproc != r->refproc)) return 0;
	if ((qtype & ASL_QUERY_MATCH_SESSION) && (q->session != r->session)) return 0;

	for (i = 0; i < q->kvcount; i += 2)
	{
		for (j = 0; j < r->kvcount; j += 2)
		{
			if (q->kvlist[i] == r->kvlist[j])
			{
				if (q->kvlist[i + 1] == r->kvlist[j + 1]) break;
				return 0;
			}
		}

		if (j >= r->kvcount) return 0;
	}

	return 1;
}

static uint32_t
asl_memory_slow_match(asl_memory_t *s, mem_record_t *r, asl_msg_t *rawq)
{
	asl_msg_t *rawm;
	uint32_t status;

	rawm = NULL;
	status = asl_memory_message_decode(s, r, &rawm);
	if (status != ASL_STATUS_OK) return 0;

	status = 0;
	if (asl_msg_cmp((asl_msg_t *)rawq, (asl_msg_t *)rawm) != 0) status = 1;
	asl_msg_release(rawm);
	return status;
}

static uint32_t
asl_memory_match_restricted_uuid_internal(asl_memory_t *s, asl_msg_list_t *query, asl_msg_list_t **res, uint64_t *last_id, uint64_t start_id, uint32_t count, uint32_t duration, int32_t direction, int32_t ruid, int32_t rgid, const char *uuid_str)
{
	uint32_t status, i, where, start, j, do_match, did_match, rescount, *qtype;
	mem_record_t **qp;
	asl_msg_t *m;
	size_t qcount;
	struct timeval now, finish;

	qp = NULL;
	qtype = NULL;
	rescount = 0;
	qcount = asl_msg_list_count(query);

	if (qcount == 0)
	{
		do_match = 0;
	}
	else
	{
		qp = (mem_record_t **)calloc(qcount, sizeof(mem_record_t *));
		if (qp == NULL) return ASL_STATUS_NO_MEMORY;

		qtype = (uint32_t *)calloc(qcount, sizeof(uint32_t));
		if (qtype == NULL)
		{
			free(qp);
			return ASL_STATUS_NO_MEMORY;
		}

		do_match = 0;
		for (i = 0; i < qcount; i++)
		{
			qp[i] = asl_memory_query_to_record(s, asl_msg_list_get_index(query, i), &(qtype[i]));
			if (qtype[i] == ASL_QUERY_MATCH_ERROR)
			{
				for (j = 0; j < i; j++) asl_memory_record_free(s, qp[j]);
				free(qp);
				free(qtype);
				return ASL_STATUS_FAILED;
			}

			if (qtype[i] != ASL_QUERY_MATCH_TRUE) do_match = 1;
		}
	}

	for (i = 0; i < s->record_count; i++)
	{
		if (direction >= 0)
		{
			where = (s->record_first + i) % s->record_count;
			if (s->record[where]->mid == 0) continue;
			if (s->record[where]->mid >= start_id) break;
		}
		else
		{
			where = ((s->record_count - (i + 1)) + s->record_first) % s->record_count;
			if (s->record[where]->mid == 0) continue;
			if (s->record[where]->mid <= start_id) break;
		}
	}

	if (i >= s->record_count)
	{
		if (qp != NULL)
		{
			for (i = 0; i < qcount; i++) asl_memory_record_free(s, qp[i]);
			free(qp);
			free(qtype);
		}

		return ASL_STATUS_OK;
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

	start = where;

	/* 
	 * loop through records
	 */
	for (i = 0; i < s->record_count; i++)
	{
		status = ASL_STATUS_INVALID_ID;
		if (s->record[where]->mid != 0) status = asl_core_check_access(s->record[where]->ruid, s->record[where]->rgid, ruid, rgid, s->record[where]->flags);

		if ((status == ASL_STATUS_OK) && (uuid_str != NULL))
		{
			if (s->record[where]->sender_mach_uuid == NULL) status = ASL_STATUS_INVALID_ID;
			else if (strcmp(s->record[where]->sender_mach_uuid->str, uuid_str) != 0) status = ASL_STATUS_INVALID_ID;
		}

		if (status != ASL_STATUS_OK)
		{
			if (direction >= 0)
			{
				where++;
				if (where >= s->record_count) where = 0;
			}
			else
			{
				if (where == 0) where = s->record_count - 1;
				else where--;
			}

			if (where == s->record_first) break;
			continue;
		}

		s->record[where]->flags &= ASL_MSG_FLAG_SEARCH_CLEAR;
		*last_id = s->record[where]->mid;
		did_match = 1;

		if (do_match != 0)
		{
			did_match = 0;

			for (j = 0; (j < qcount) && (did_match == 0); j++)
			{
				if (qtype[j] == ASL_QUERY_MATCH_TRUE)
				{
					did_match = 1;
				}
				else if (qtype[j] == ASL_QUERY_MATCH_FALSE)
				{
					did_match = 0;
				}
				else if (qtype[j] == ASL_QUERY_MATCH_SLOW)
				{
					did_match = asl_memory_slow_match(s, s->record[where], asl_msg_list_get_index(query, j));
				}
				else
				{
					did_match = asl_memory_fast_match(s, s->record[where], qtype[j], qp[j]);
				}
			}
		}

		if (did_match == 1)
		{
			s->record[where]->flags |= ASL_MSG_FLAG_SEARCH_MATCH;
			rescount++;
			if ((count != 0) && (rescount >= count)) break;
		}

		/* check the timer */
		if ((finish.tv_sec != 0) && (gettimeofday(&now, NULL) == 0))
		{
			if ((now.tv_sec > finish.tv_sec) || ((now.tv_sec == finish.tv_sec) && (now.tv_usec > finish.tv_usec))) break;
		}

		if (direction >= 0)
		{
			where++;
			if (where >= s->record_count) where = 0;
		}
		else
		{
			if (where == 0) where = s->record_count - 1;
			else where--;
		}

		if (where == s->record_first) break;
	}

	if (qp != NULL)
	{
		for (i = 0; i < qcount; i++) asl_memory_record_free(s, qp[i]);
		free(qp);
		free(qtype);
	}

	*res = NULL;
	if (rescount == 0) return ASL_STATUS_OK;

	*res = asl_msg_list_new();
	if (*res == NULL) return ASL_STATUS_NO_MEMORY;

	where = start;
	forever
	{
		int n = 0;

		if (s->record[where]->flags & ASL_MSG_FLAG_SEARCH_MATCH)
		{
			s->record[where]->flags &= ASL_MSG_FLAG_SEARCH_CLEAR;

			status = asl_memory_message_decode(s, s->record[where], &m);
			if (status != ASL_STATUS_OK)
			{
				asl_msg_list_release(*res);
				*res = NULL;
				return status;
			}

			asl_msg_list_append(*res, m);
			asl_msg_release(m);
			n++;
			if (n == rescount) break;
		}

		if (direction >= 0)
		{
			where++;
			if (where >= s->record_count) where = 0;
		}
		else
		{
			if (where == 0) where = s->record_count - 1;
			else where--;
		}

		if (where == s->record_first) break;
	}

	return ASL_STATUS_OK;
}

uint32_t
asl_memory_match_restricted_uuid(asl_memory_t *s, asl_msg_list_t *query, asl_msg_list_t **res, uint64_t *last_id, uint64_t start_id, uint32_t count, uint32_t duration, int32_t direction, int32_t ruid, int32_t rgid, const char *uuid_str)
{
	__block uint32_t status;
 
	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (res == NULL) return ASL_STATUS_INVALID_ARG;
	
	dispatch_sync(s->queue, ^{
		status = asl_memory_match_restricted_uuid_internal(s, query, res, last_id, start_id, count, duration, direction, ruid, rgid, uuid_str);
	});
	
	return status;
}

uint32_t
asl_memory_match(asl_memory_t *s, asl_msg_list_t *query, asl_msg_list_t **res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction, int32_t ruid, int32_t rgid)
{
	__block uint32_t status;
 
	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (res == NULL) return ASL_STATUS_INVALID_ARG;
	
	dispatch_sync(s->queue, ^{
		status = asl_memory_match_restricted_uuid_internal(s, query, res, last_id, start_id, count, 0, direction, ruid, rgid, NULL);
	});

	return status;
}
