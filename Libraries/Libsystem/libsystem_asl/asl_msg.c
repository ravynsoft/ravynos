/*
 * Copyright (c) 2009-2012 Apple Inc. All rights reserved.
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <regex.h>
#include <syslog.h>
#include <notify.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <asl.h>
#include <asl_object.h>
#include <asl_private.h>
#include <asl_core.h>
#include <asl_client.h>
#include <sys/types.h>
#include <libkern/OSAtomic.h>
#include <asl_msg.h>
#include <asl_msg_list.h>

#define TOKEN_NULL  0
#define TOKEN_OPEN  1
#define TOKEN_CLOSE 2
#define TOKEN_WORD  3
#define TOKEN_INT   4

#define MFMT_RAW 0
#define MFMT_STD 1
#define MFMT_BSD 2
#define MFMT_XML 3
#define MFMT_STR 4
#define MFMT_MSG 5

#define SEC_PER_HOUR 3600

#define PAGE_OBJECT 1

#define forever for(;;)

#ifndef ASL_QUERY_OP_FALSE
#define ASL_QUERY_OP_FALSE 0
#endif

#define AUX_0_TIME      0x00000001
#define AUX_0_TIME_NSEC 0x00000002
#define AUX_0_HOST      0x00000004
#define AUX_0_SENDER    0x00000008
#define AUX_0_FACILITY  0x00000010
#define AUX_0_PID       0x00000020
#define AUX_0_UID       0x00000040
#define AUX_0_GID       0x00000080
#define AUX_0_MSG       0x00000100
#define AUX_0_OPTION    0x00000200
#define AUX_0_LEVEL     0x00000400

/* from asl_util.c */
int asl_is_utf8(const char *str);
uint8_t *asl_b64_encode(const uint8_t *buf, size_t len);

void _asl_msg_dump(FILE *f, const char *comment, asl_msg_t *msg);

#pragma mark -
#pragma mark standard message keys

static const char *ASLStandardKey[] =
{
	ASL_KEY_TIME,
	ASL_KEY_TIME_NSEC,
	ASL_KEY_HOST,
	ASL_KEY_SENDER,
	ASL_KEY_FACILITY,
	ASL_KEY_PID,
	ASL_KEY_UID,
	ASL_KEY_GID,
	ASL_KEY_LEVEL,
	ASL_KEY_MSG,
	ASL_KEY_READ_UID,
	ASL_KEY_READ_GID,
	ASL_KEY_SESSION,
	ASL_KEY_REF_PID,
	ASL_KEY_REF_PROC,
	ASL_KEY_MSG_ID,
	ASL_KEY_EXPIRE_TIME,
	ASL_KEY_OPTION,
	ASL_KEY_FREE_NOTE
};

static const char *MTStandardKey[] =
{
	"com.apple.message.domain",
	"com.apple.message.domain_scope",
	"com.apple.message.result",
	"com.apple.message.signature",
	"com.apple.message.signature2",
	"com.apple.message.signature3",
	"com.apple.message.success",
	"com.apple.message.uuid",
	"com.apple.message.value",
	"com.apple.message.value2",
	"com.apple.message.value3",
	"com.apple.message.value4",
	"com.apple.message.value5"
};

static uint16_t
_asl_msg_std_key(const char *s, uint32_t len)
{
	if ((len > 18) && (streq_len(s, "com.apple.message.", 18)))
	{
		if (streq(s + 18, "domain")) return ASL_MT_KEY_DOMAIN;
		else if (streq(s + 18, "domain_scope")) return ASL_MT_KEY_SCOPE;
		else if (streq(s + 18, "result")) return ASL_MT_KEY_RESULT;
		else if (streq(s + 18, "signature")) return ASL_MT_KEY_SIG;
		else if (streq(s + 18, "signature2")) return ASL_MT_KEY_SIG2;
		else if (streq(s + 18, "signature3")) return ASL_MT_KEY_SIG3;
		else if (streq(s + 18, "success")) return ASL_MT_KEY_SUCCESS;
		else if (streq(s + 18, "uuid")) return ASL_MT_KEY_UUID;
		else if (streq(s + 18, "value")) return ASL_MT_KEY_VAL;
		else if (streq(s + 18, "value2")) return ASL_MT_KEY_VAL2;
		else if (streq(s + 18, "value3")) return ASL_MT_KEY_VAL3;
		else if (streq(s + 18, "value4")) return ASL_MT_KEY_VAL4;
		else if (streq(s + 18, "value5")) return ASL_MT_KEY_VAL5;

		return 0;
	}

	switch (len)
	{
		case 3:
		{
			if streq(s, ASL_KEY_PID) return ASL_STD_KEY_PID;
			else if streq(s, ASL_KEY_UID) return ASL_STD_KEY_UID;
			else if streq(s, ASL_KEY_GID) return ASL_STD_KEY_GID;
		}
		case 4:
		{
			if streq(s, ASL_KEY_TIME) return ASL_STD_KEY_TIME;
			else if streq(s, ASL_KEY_HOST) return ASL_STD_KEY_HOST;
		}
		case 5:
		{
			if streq(s, ASL_KEY_LEVEL) return ASL_STD_KEY_LEVEL;
		}
		case 6:
		{
			if streq(s, ASL_KEY_SENDER) return ASL_STD_KEY_SENDER;
			else if streq(s, ASL_KEY_REF_PID) return ASL_STD_KEY_REF_PID;
		}
		case 7:
		{
			if streq(s, ASL_KEY_MSG) return ASL_STD_KEY_MESSAGE;
			else if streq(s, ASL_KEY_SESSION) return ASL_STD_KEY_SESSION;
			else if streq(s, ASL_KEY_READ_UID) return ASL_STD_KEY_READ_UID;
			else if streq(s, ASL_KEY_READ_GID) return ASL_STD_KEY_READ_GID;
			else if streq(s, ASL_KEY_REF_PROC) return ASL_STD_KEY_REF_PROC;
		}
		case 8:
		{
			if streq(s, ASL_KEY_FACILITY) return ASL_STD_KEY_FACILITY;
		}
		case 9:
		{
			if streq(s, ASL_KEY_OPTION) return ASL_STD_KEY_OPTION;
		}
		case 11:
		{
			if streq(s, ASL_KEY_TIME_NSEC) return ASL_STD_KEY_NANO;
		}
		case 12:
		{
			if streq(s, ASL_KEY_MSG_ID) return ASL_STD_KEY_MSG_ID;
		}
		case 13:
		{
			if streq(s, ASL_KEY_EXPIRE_TIME) return ASL_STD_KEY_EXPIRE;
		}
		case 14:
		{
			if streq(s, ASL_KEY_FREE_NOTE) return ASL_STD_KEY_FREE_NOTE;
		}
		default:
		{
			return 0;
		}
	}

	return 0;
}

#pragma mark -
#pragma mark asl_msg

static uint32_t
_slot_count(asl_msg_t *m)
{
	if (m == NULL) return 0;
	if (m->asl_type == ASL_TYPE_MSG) return ASL_MSG_KVO_MSG_SLOTS;
	if (m->asl_type == ASL_TYPE_QUERY) return ASL_MSG_KVO_QUERY_SLOTS;
	return 0;
}


static uint16_t
_get_slot_key(asl_msg_t *m, uint32_t slot)
{
	if (m == NULL) return 0;

	if (m->asl_type == ASL_TYPE_MSG)
	{
		if (slot < ASL_MSG_KVO_MSG_SLOTS) return m->kvo[slot];
		return 0;
	}

	if (m->asl_type == ASL_TYPE_QUERY)
	{
		if (slot < ASL_MSG_KVO_QUERY_SLOTS) return m->kvo[slot];
		return 0;
	}

	return 0;
}


static void
_set_slot_key(asl_msg_t *m, uint32_t slot, uint16_t x)
{
	if (m == NULL) return;

	if (m->asl_type == ASL_TYPE_MSG)
	{
		if (slot < ASL_MSG_KVO_MSG_SLOTS) m->kvo[slot] = x;
		return;
	}

	if (m->asl_type == ASL_TYPE_QUERY)
	{
		if (slot < ASL_MSG_KVO_QUERY_SLOTS) m->kvo[slot] = x;
		return;
	}
}


static uint16_t
_get_slot_val(asl_msg_t *m, uint32_t slot)
{
	if (m == NULL) return 0;
	if (m->asl_type == ASL_TYPE_MSG)
	{
		if (slot < ASL_MSG_KVO_MSG_SLOTS) return m->kvo[slot + ASL_MSG_KVO_MSG_SLOTS];
		return 0;
	}

	if (m->asl_type == ASL_TYPE_QUERY)
	{
		if (slot < ASL_MSG_KVO_QUERY_SLOTS) return m->kvo[slot + ASL_MSG_KVO_QUERY_SLOTS];
		return 0;
	}

	return 0;
}

static void
_set_slot_val(asl_msg_t *m, uint32_t slot, uint16_t x)
{
	if (m == NULL) return;

	if (m->asl_type == ASL_TYPE_MSG)
	{
		if (slot < ASL_MSG_KVO_MSG_SLOTS) m->kvo[slot + ASL_MSG_KVO_MSG_SLOTS] = x;
		return;
	}

	if (m->asl_type == ASL_TYPE_QUERY)
	{
		if (slot < ASL_MSG_KVO_QUERY_SLOTS) m->kvo[slot + ASL_MSG_KVO_QUERY_SLOTS] = x;
		return;
	}
}

static uint16_t
_get_slot_op(asl_msg_t *m, uint32_t slot)
{
	if (m == NULL) return 0;

	if (m->asl_type == ASL_TYPE_QUERY)
	{
		if (slot < ASL_MSG_KVO_QUERY_SLOTS) return m->kvo[slot + (ASL_MSG_KVO_QUERY_SLOTS * 2)];
		return 0;
	}

	return 0;
}

static void
_set_slot_op(asl_msg_t *m, uint32_t slot, uint16_t x)
{
	if (m == NULL) return;

	if (m->asl_type == ASL_TYPE_QUERY)
	{
		if (slot < ASL_MSG_KVO_QUERY_SLOTS) m->kvo[slot + (ASL_MSG_KVO_QUERY_SLOTS * 2)] = x;
	}
}

static asl_msg_t *
_asl_msg_make_page(uint32_t type)
{
	uint32_t i, n = 0;
	asl_msg_t *out = (asl_msg_t *)calloc(1, sizeof(asl_msg_t));
	if (out == NULL) return NULL;

	if (type == ASL_TYPE_MSG) n = ASL_MSG_KVO_MSG_SLOTS * 2;
	else if (type == ASL_TYPE_QUERY) n = ASL_MSG_KVO_QUERY_SLOTS * 2;

	for (i = 0; i < n; i++) out->kvo[i] = ASL_MSG_SLOT_FREE;

	out->mem_size = sizeof(asl_msg_t);
	out->asl_type = type;

	return out;
}

asl_msg_t *
asl_msg_retain(asl_msg_t *msg)
{
	if (msg == NULL) return NULL;
	asl_retain((asl_object_t)msg);
	return msg;
}

void
asl_msg_release(asl_msg_t *msg)
{
	if (msg == NULL) return;
	asl_release((asl_object_t)msg);
}

static const char *
_asl_msg_slot_key(asl_msg_t *page, uint32_t slot)
{
	const char *out;
	uint16_t x, k;

	if (page == NULL) return NULL;

	if ((page->asl_type == ASL_TYPE_MSG) && (slot >= ASL_MSG_KVO_MSG_SLOTS)) return NULL;
	else if ((page->asl_type == ASL_TYPE_QUERY) && (slot >= ASL_MSG_KVO_QUERY_SLOTS)) return NULL;

	k = _get_slot_key(page, slot);

	if (k == ASL_MSG_SLOT_FREE) return NULL;

	switch (k & ASL_MSG_KV_MASK)
	{
		case ASL_MSG_KV_INLINE:
		{
			return page->data + k;
		}
		case ASL_MSG_KV_DICT:
		{
			if ((k > ASL_STD_KEY_BASE) && (k <= ASL_STD_KEY_LAST))
			{
				x = k - ASL_STD_KEY_BASE - 1;
				return ASLStandardKey[x];
			}
			else if ((k > ASL_MT_KEY_BASE) && (k <= ASL_MT_KEY_LAST))
			{
				x = k - ASL_MT_KEY_BASE - 1;
				return MTStandardKey[x];
			}

			return NULL;
		}
		case ASL_MSG_KV_EXTERN:
		{
			x = k & ASL_MSG_OFFSET_MASK;
			memcpy(&out, page->data + x, sizeof(char *));
			return out;
		}
	}

	return NULL;
}

static const char *
_asl_msg_slot_val(asl_msg_t *page, uint32_t slot)
{
	const char *out;
	uint16_t x, v, type;

	if (page == NULL) return NULL;

	if ((page->asl_type == ASL_TYPE_MSG) && (slot >= ASL_MSG_KVO_MSG_SLOTS)) return NULL;
	else if ((page->asl_type == ASL_TYPE_QUERY) && (slot >= ASL_MSG_KVO_QUERY_SLOTS)) return NULL;

	v = _get_slot_val(page, slot);

	if (v == ASL_MSG_SLOT_FREE) return NULL;

	type = v & ASL_MSG_KV_MASK;

	if (type == ASL_MSG_KV_INLINE)
	{
		return page->data + v;
	}
	else if (type == ASL_MSG_KV_EXTERN)
	{
		x = v & ASL_MSG_OFFSET_MASK;
		memcpy(&out, page->data + x, sizeof(char *));
		return out;
	}

	return NULL;
}

/*
 * asl_new: create a new log message.
 */
asl_msg_t *
asl_msg_new(uint32_t type)
{
	asl_msg_t *out;

	out = _asl_msg_make_page(type);
	if (out == NULL) return NULL;

	out->asl_type = type;
	out->refcount = 1;

	return out;
}

static void
_asl_msg_free_page(asl_msg_t *page)
{
	uint32_t i, mslots;
	char *p;

	if (page == NULL) return;

	mslots = _slot_count(page);

	for (i = 0; i < mslots; i++)
	{
		uint16_t k = _get_slot_key(page, i);
		uint16_t v = _get_slot_val(page, i);

		if (k == ASL_STD_KEY_FREE_NOTE)
		{
			const char *x = _asl_msg_slot_val(page, i);
			if (x != NULL) notify_post(x);
		}

		if ((k & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
		{
			memcpy(&p, page->data + (k & ASL_MSG_OFFSET_MASK), sizeof(char *));
			free(p);
		}

		if ((v & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
		{
			memcpy(&p, page->data + (v & ASL_MSG_OFFSET_MASK), sizeof(char *));
			free(p);
		}
	}

	free(page);
}

uint32_t
asl_msg_type(asl_msg_t *msg)
{
	if (msg == NULL) return 0;
	return msg->asl_type;
}

uint32_t
asl_msg_count(asl_msg_t *msg)
{
	uint32_t total;

	total = 0;

	for (; msg != NULL; msg = msg->next) total += msg->count;
	return total;
}

static void
_asl_msg_dump_kv(FILE *f, asl_msg_t *msg, uint16_t x)
{
	if (x == ASL_MSG_SLOT_FREE)
	{
		fprintf(f, "-free-");
		return;
	}

	if ((x & ASL_MSG_KV_MASK) == ASL_MSG_KV_DICT)
	{
		switch (x)
		{
			case ASL_STD_KEY_TIME: fprintf(f, "(dict: Time)"); return;
			case ASL_STD_KEY_NANO: fprintf(f, "(dict: Nano)"); return;
			case ASL_STD_KEY_HOST: fprintf(f, "(dict: Host)"); return;
			case ASL_STD_KEY_SENDER: fprintf(f, "(dict: Sender)"); return;
			case ASL_STD_KEY_FACILITY: fprintf(f, "(dict: Facility)"); return;
			case ASL_STD_KEY_PID: fprintf(f, "(dict: PID)"); return;
			case ASL_STD_KEY_UID: fprintf(f, "(dict: UID)"); return;
			case ASL_STD_KEY_GID: fprintf(f, "(dict: GID)"); return;
			case ASL_STD_KEY_LEVEL: fprintf(f, "(dict: Level)"); return;
			case ASL_STD_KEY_MESSAGE: fprintf(f, "(dict: Message)"); return;
			case ASL_STD_KEY_READ_UID: fprintf(f, "(dict: ReadUID)"); return;
			case ASL_STD_KEY_READ_GID: fprintf(f, "(dict: ReadGID)"); return;
			case ASL_STD_KEY_SESSION: fprintf(f, "(dict: Session)"); return;
			case ASL_STD_KEY_REF_PID: fprintf(f, "(dict: PID)"); return;
			case ASL_STD_KEY_REF_PROC: fprintf(f, "(dict: RefProc)"); return;
			case ASL_STD_KEY_MSG_ID: fprintf(f, "(dict: ASLMessageID)"); return;
			case ASL_STD_KEY_EXPIRE: fprintf(f, "(dict: Expire)"); return;
			case ASL_STD_KEY_OPTION: fprintf(f, "(dict: ASLOption)"); return;
			case ASL_MT_KEY_DOMAIN: fprintf(f, "(dict: com.apple.message.domain)"); return;
			case ASL_MT_KEY_SCOPE: fprintf(f, "(dict: com.apple.message.domain_scope)"); return;
			case ASL_MT_KEY_RESULT: fprintf(f, "(dict: com.apple.message.result)"); return;
			case ASL_MT_KEY_SIG: fprintf(f, "(dict: com.apple.message.signature)"); return;
			case ASL_MT_KEY_SIG2: fprintf(f, "(dict: com.apple.message.signature2)"); return;
			case ASL_MT_KEY_SIG3: fprintf(f, "(dict: com.apple.message.signature3)"); return;
			case ASL_MT_KEY_SUCCESS: fprintf(f, "(dict: com.apple.message.success)"); return;
			case ASL_MT_KEY_UUID: fprintf(f, "(dict: com.apple.message.uuid)"); return;
			case ASL_MT_KEY_VAL: fprintf(f, "(dict: com.apple.message.value)"); return;
			case ASL_MT_KEY_VAL2: fprintf(f, "(dict: com.apple.message.value2)"); return;
			case ASL_MT_KEY_VAL3: fprintf(f, "(dict: com.apple.message.value3)"); return;
			case ASL_MT_KEY_VAL4: fprintf(f, "(dict: com.apple.message.value4)"); return;
			case ASL_MT_KEY_VAL5: fprintf(f, "(dict: com.apple.message.value5)"); return;
		}

		fprintf(f, "(dict: -unknown-)");
		return;
	}

	if ((x & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
	{
		const char *c;
		size_t z = x & ASL_MSG_OFFSET_MASK;
		memcpy(&c, msg->data + z, sizeof(char *));
		fprintf(f, "(extern: %s)", c);
		return;
	}

	fprintf(f, "%s", msg->data + x);
}

void
_asl_msg_dump(FILE *f, const char *comment, asl_msg_t *msg)
{
	uint32_t i, mslots, page1 = 1;

	if (f == NULL) return;
	if (msg == NULL)
	{
		fprintf(f, "asl_msg %s: NULL\n", comment);
		return;
	}

	mslots = _slot_count(msg);

	while (msg != NULL)
	{
		if (page1 == 1)
		{
			fprintf(f, "asl_msg %s: %p\n", comment, msg);
			fprintf(f, "    refcount: %u\n", msg->refcount);
			fprintf(f, "    asl_type: %u\n", msg->asl_type);
			page1 = 0;
		}
		else
		{
			fprintf(f, "  page: %p\n", msg);
		}

		fprintf(f, "    count: %u\n", msg->count);
		fprintf(f, "    data_size: %u\n", msg->data_size);
		fprintf(f, "    mem_size: %llu\n", msg->mem_size);
		fprintf(f, "    next: %p\n", msg->next);

		for (i = 0; i < mslots; i++)
		{
			fprintf(f, "    slot[%d]: ", i);
			_asl_msg_dump_kv(f, msg, _get_slot_key(msg, i));
			fprintf(f, " ");
			_asl_msg_dump_kv(f, msg, _get_slot_val(msg, i));
			if (msg->asl_type == ASL_TYPE_QUERY) fprintf(f, " 0x%04x\n", _get_slot_op(msg, i));
		}

		msg = msg->next;
	}
}

#pragma mark -
#pragma mark fetching contents

/*
 * Find the slot and page for an input key.
 */
static uint32_t
_asl_msg_index(asl_msg_t *msg, const char *key, uint32_t *oslot, asl_msg_t **opage)
{
	uint32_t i, len, slot, mslots;
	uint16_t kx;
	asl_msg_t *page;
	const char *kp;

	if (msg == NULL) return IndexNull;
	if (key == NULL) return IndexNull;

	i = 0;
	slot = 0;
	mslots = _slot_count(msg);

	if (oslot != NULL) *oslot = slot;

	page = msg;
	if (opage != NULL) *opage = page;

	len = strlen(key);
	kx = _asl_msg_std_key(key, len);

	forever
	{
		if (_get_slot_key(page, slot) != ASL_MSG_SLOT_FREE)
		{
			if (kx != 0)
			{
				if (_get_slot_key(page, slot) == kx) return i;
			}
			else if ((_get_slot_key(page, slot) & ASL_MSG_KV_MASK) == ASL_MSG_KV_DICT)
			{
				/* _get_slot_key(page, slot) is a dictionary key, but key is not (kx == 0) so skip this slot */
			}
			else if ((_get_slot_key(page, slot) & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
			{
				memcpy(&kp, page->data + (_get_slot_key(page, slot) & ASL_MSG_OFFSET_MASK), sizeof(char *));
				if (streq(key, kp)) return i;
			}
			else
			{
				kp = page->data + _get_slot_key(page, slot);
				if (streq(key, kp)) return i;
			}
		}

		i++;
		slot++;
		if (oslot != NULL) *oslot = slot;

		if (slot >= mslots)
		{
			if (page->next == NULL) return IndexNull;

			slot = 0;
			if (oslot != NULL) *oslot = slot;

			page = page->next;
			if (opage != NULL) *opage = page;
		}
	}

	return IndexNull;
}

/*
 * Find page and slot for an "index".
 */
static int
_asl_msg_resolve_index(asl_msg_t *msg, uint32_t n, asl_msg_t **page, uint32_t *slot)
{
	uint32_t i, sx, mslots;
	asl_msg_t *px;

	if (msg == NULL) return -1;

	*slot = IndexNull;
	*page = NULL;

	mslots = _slot_count(msg);

	sx = 0;

	/* find page */
	for (px = msg; px != NULL; px = px->next)
	{
		if (n > (sx + px->count))
		{
			sx += px->count;
			continue;
		}

		*page = px;

		/* find slot */
		for (i = 0; i < mslots; i++)
		{
			if (px->kvo[i] != ASL_MSG_SLOT_FREE)
			{
				if (sx == n)
				{
					*slot = i;
					return 0;
				}

				sx++;
			}
		}
	}

	return -1;
}

/*
 * asl_msg_fetch: iterate over entries
 * initial value of n should be 0.  Subseqent calls should use the last
 * returned value.  Returns IndexNull when there are no more entries
 * Sets the pointers for the next key, value, and op in the msg.
 * The iterator encodes a page number and a slot number.
 */

uint32_t
asl_msg_fetch(asl_msg_t *msg, uint32_t x, const char **keyout, const char **valout, uint16_t *opout)
{
	uint32_t p, xpn, xsn, mslots;
	asl_msg_t *page = NULL;

	if (msg == NULL) return IndexNull;

	mslots = _slot_count(msg);

	xsn = x >> 24;
	xpn = x & 0x00ffffff;

	/* slot number 0xff means we have run out entries */
	if (xsn == 0x000000ff) return IndexNull;

	page = msg;
	for (p = 0; p < xpn; p++)
	{
		page = page->next;
		if (page == NULL) return IndexNull;
	}

	if (keyout != NULL) *keyout = _asl_msg_slot_key(page, xsn);
	if (valout != NULL) *valout = _asl_msg_slot_val(page, xsn);
	if (opout != NULL) *opout = _get_slot_op(page, xsn);

	/* advance to the next slot */
	forever
	{
		xsn++;

		if (xsn >= mslots)
		{
			if (page->next == NULL) return 0xff000000;
			xsn = 0;
			page = page->next;
			xpn++;
		}

		if (page->kvo[xsn] != ASL_MSG_SLOT_FREE) return ((xsn << 24) | xpn);
	}

	return IndexNull;
}

int
asl_msg_lookup(asl_msg_t *msg, const char *key, const char **valout, uint16_t *opout)
{
	uint32_t i, slot;
	asl_msg_t *page;

	if (msg == NULL) return -1;
	if (valout != NULL) *valout = NULL;
	if (opout != NULL) *opout = 0;

	slot = IndexNull;
	page = NULL;

	i = _asl_msg_index(msg, key, &slot, &page);
	if (i == IndexNull) return -1;

	if (valout != NULL) *valout = _asl_msg_slot_val(page, slot);
	if (opout != NULL) *opout = _get_slot_op(page, slot);

	return 0;
}

const char *
asl_msg_get_val_for_key(asl_msg_t *msg, const char *key)
{
	uint32_t slot;
	asl_msg_t *page;

	if (msg == NULL) return NULL;

	slot = IndexNull;
	page = NULL;

	if (_asl_msg_index(msg, key, &slot, &page) == IndexNull) return NULL;

	return _asl_msg_slot_val(page, slot);
}

const char *
asl_msg_key(asl_msg_t *msg, uint32_t n)
{
	uint32_t slot, i, mslots;
	asl_msg_t *page;

	if (msg == NULL) return NULL;

	mslots = _slot_count(msg);

	i = 0;
	for (page = msg; page != NULL; page = page->next)
	{
		for (slot = 0; slot < mslots; slot++)
		{
			if (_get_slot_key(page, slot) != ASL_MSG_SLOT_FREE)
			{
				if (i == n) return _asl_msg_slot_key(page, slot);
				i++;
			}
		}
	}

	return NULL;
}

#pragma mark -
#pragma mark adding and replacing contents

static int
_asl_msg_new_key_val_op(asl_msg_t *msg, const char *key, const char *val, uint32_t op)
{
	uint32_t slot, keylen, vallen, total, mslots;
	uint64_t klen, vlen;
	uint16_t kx, k, v, o;
	asl_msg_t *page, *last;
	char *extkey, *extval;

	if (msg == NULL) return -1;
	if (key == NULL) return -1;

	mslots = _slot_count(msg);

	o = op;
	extkey = NULL;
	extval = NULL;

	keylen = strlen(key);
	klen = keylen;
	kx = _asl_msg_std_key(key, keylen);

	if (kx == 0) keylen++;
	else keylen = 0;

	total = keylen;

	vallen = 0;
	if (val != NULL)
	{
		vallen = strlen(val) + 1;
		vlen = vallen;
		total += vallen;
	}

	/* check if one or both of key and value must be "external" (in its own malloced space) */
	if (keylen > ASL_MSG_PAGE_DATA_SIZE)
	{
		extkey = strdup(key);
		keylen = sizeof(char *);
	}

	if (vallen > ASL_MSG_PAGE_DATA_SIZE)
	{
		extval = strdup(val);
		vallen = sizeof(char *);
	}

	total = keylen + vallen;
	if ((total > ASL_MSG_PAGE_DATA_SIZE) && (extval == NULL) && (keylen > 0))
	{
		extval = strdup(val);
		vallen = sizeof(char *);
		total = keylen + vallen;
	}

	if ((total > ASL_MSG_PAGE_DATA_SIZE) && (extkey == NULL))
	{
		extkey = strdup(key);
		keylen = sizeof(char *);
		total = keylen + vallen;
	}

	if (total > ASL_MSG_PAGE_DATA_SIZE)
	{
		/* can't happen, but... */
		if (extkey != NULL) free(extkey);
		if (extval != NULL) free(extval);
		return -1;
	}

	/* find a page with space for the key and value and a free slot*/
	slot = 0;
	last = msg;

	for (page = msg; page != NULL; page = page->next)
	{
		last = page;

		if (total <= (ASL_MSG_PAGE_DATA_SIZE - page->data_size))
		{
			/* check for a free slot */
			for (slot = 0; (slot < mslots) && (_get_slot_key(page, slot) != ASL_MSG_SLOT_FREE); slot++);
			if (slot < mslots) break;
		}
	}

	if (page == NULL)
	{
		/* allocate a new page and attach it */
		page = _asl_msg_make_page(msg->asl_type);
		if (page == NULL)
		{
			if (extkey != NULL) free(extkey);
			if (extval != NULL) free(extval);
			return -1;
		}

		last->next = page;
		slot = 0;
	}

	/* copy key or external key pointer into page data */
	if (kx != 0)
	{
		k = kx;
	}
	else if (extkey == NULL)
	{
		k = page->data_size;
		memcpy(page->data + page->data_size, key, keylen);
	}
	else
	{
		k = page->data_size | ASL_MSG_KV_EXTERN;
		memcpy(page->data + page->data_size, &extkey, keylen);
		page->mem_size += klen;
	}

	_set_slot_key(page, slot, k);
	page->data_size += keylen;

	/* copy val or external val pointer into page data */

	v = ASL_MSG_SLOT_FREE;

	if (val != NULL)
	{
		if (extval == NULL)
		{
			v = page->data_size;
			memcpy(page->data + page->data_size, val, vallen);
		}
		else
		{
			v = page->data_size | ASL_MSG_KV_EXTERN;
			memcpy(page->data + page->data_size, &extval, vallen);
			page->mem_size += vlen;
		}

		_set_slot_val(page, slot, v);
		page->data_size += vallen;
	}

	/* set op */
	_set_slot_op(page, slot, o);

	/* update page count */
	page->count++;

	return 0;
}

/*
 * Most of the code in asl_msg_set_key_val_op is concerned with trying to re-use
 * space in an asl_msg_t page when given a new value for an existing key.
 * If the key is new, we just call _asl_msg_new_key_val_op.
 *
 * Note that queries can have duplicate keys, so for ASL_TYPE_QUERY we just
 * call through to _asl_msg_new_key_val_op.
 */
static int
_asl_msg_set_kvo(asl_msg_t *msg, const char *key, const char *val, uint32_t op)
{
	uint32_t i, slot, mslots, newexternal;
	asl_msg_t *page;
	uint32_t intvallen, extvallen, newvallen;
	char *intval, *extval, *newval;
	uint16_t k, v, o;

	if (msg == NULL) return -1;
	if (key == NULL) return -1;

	mslots = _slot_count(msg);

	slot = IndexNull;
	page = NULL;

	o = op;

	if ((msg->asl_type == ASL_TYPE_QUERY) || (IndexNull == _asl_msg_index(msg, key, &slot, &page)))
	{
		/* add key */
		return _asl_msg_new_key_val_op(msg, key, val, op);
	}

	intval = NULL;
	intvallen = 0;

	extval = NULL;
	extvallen = 0;

	v = _get_slot_val(page, slot);

	if (v != ASL_MSG_SLOT_FREE)
	{
		if ((v & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
		{
			i = v & ASL_MSG_OFFSET_MASK;
			memcpy(&extval, page->data + i, sizeof(char *));
			extvallen = sizeof(char *);
		}
		else
		{
			intval = page->data + v;
			intvallen = strlen(intval) + 1;
		}
	}

	/* replace val and op for existing entry */

	/* easy case  - remove val */
	if (val == NULL)
	{
		if (extval != NULL)
		{
			page->mem_size -= (strlen(extval) + 1);
			free(extval);
		}

		_set_slot_val(page, slot, ASL_MSG_SLOT_FREE);
		if (op != IndexNull) _set_slot_op(page, slot, o);
		return 0;
	}

	/* trivial case - internal val doesn't change */
	if ((intval != NULL) && (streq(val, intval)))
	{
		if (op != IndexNull) _set_slot_op(page, slot, o);
		return 0;
	}

	/* trivial case - external val doesn't change */
	if ((extval != NULL) && (streq(val, extval)))
	{
		if (op != IndexNull) _set_slot_op(page, slot, o);
		return 0;
	}

	/*
	 * special case: we generally don't compress out holes in the data
	 * space, but if this is the last string in the currently used data space
	 * we can just back up the data_size and reset page->val[slot] (a.k.a. page->kvo[slot + mslots])
	 */
	i = v & ASL_MSG_OFFSET_MASK;
	if ((intval != NULL) && ((i + intvallen) == page->data_size))
	{
		_set_slot_val(page, slot, ASL_MSG_SLOT_FREE);
		page->data_size -= intvallen;
		intval = NULL;
		intvallen = 0;
	}
	else if ((extval != NULL) && ((i + extvallen) == page->data_size))
	{
		_set_slot_val(page, slot, ASL_MSG_SLOT_FREE);
		page->data_size -= extvallen;
		page->mem_size -= (strlen(extval) + 1);
		free(extval);
		extval = NULL;
		extvallen = 0;
	}

	/* val changes - see if it needs to be external */
	newvallen = strlen(val) + 1;
	newexternal = 0;

	if (newvallen > ASL_MSG_PAGE_DATA_SIZE)
	{
		newexternal = 1;
		newvallen = sizeof(char *);
	}

	/* check if there is room to change val in place */
	if (((extval != NULL) && (newvallen <= extvallen)) || ((extval == NULL) && (newvallen <= intvallen)))
	{
		if (extval != NULL)
		{
			page->mem_size -= (strlen(extval) + 1);
			free(extval);
		}

		extval = NULL;

		/* we can re-use the space of the old value */
		i = v & ASL_MSG_OFFSET_MASK;

		if (newexternal == 1)
		{
			/* create an external val and copy in the new pointer */
			newval = strdup(val);
			if (newval == NULL) return -1;

			page->mem_size += (strlen(newval) + 1);
			_set_slot_val(page, slot, i | ASL_MSG_KV_EXTERN);
			memcpy(page->data + i, &newval, sizeof(char *));
		}
		else
		{
			/* new internal value */
			_set_slot_val(page, slot, i);
			memcpy(page->data + i, val, newvallen);
		}

		if (op != IndexNull) _set_slot_op(page, slot, o);
		return 0;
	}

	/* we're done with the old value if it is external - free it now */
	if (extval != NULL)
	{
		page->mem_size -= (strlen(extval) + 1);
		free(extval);
	}

	extval = NULL;

	if (newvallen <= (ASL_MSG_PAGE_DATA_SIZE - page->data_size))
	{
		/* can't re-use the old space, but there's room on the page */
		i = page->data_size;
		page->data_size += newvallen;

		if (newexternal == 1)
		{
			/* create an external val and copy in the new pointer */
			newval = strdup(val);
			if (newval == NULL) return -1;

			page->mem_size += (strlen(newval) + 1);
			_set_slot_val(page, slot, i | ASL_MSG_KV_EXTERN);
			memcpy(page->data + i, &newval, sizeof(char *));
		}
		else
		{
			/* new internal value */
			_set_slot_val(page, slot, i);
			memcpy(page->data + i, val, newvallen);
		}

		if (op != IndexNull) _set_slot_op(page, slot, o);
		return 0;

	}

	/* no room on this page - free up existing entry and treat this as a new entry */
	if ((k & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
	{
		memcpy(&extval, page->data + (k & ASL_MSG_OFFSET_MASK), sizeof(char *));
		page->mem_size -= (strlen(extval) + 1);
		free(extval);
	}

	_set_slot_key(page, slot, ASL_MSG_SLOT_FREE);
	_set_slot_val(page, slot, ASL_MSG_SLOT_FREE);
	_set_slot_op(page, slot, 0);

	return _asl_msg_new_key_val_op(msg, key, val, op);
}

int
asl_msg_set_key_val_op(asl_msg_t *msg, const char *key, const char *val, uint32_t op)
{
	char *special, buf[512];
	uint32_t i, len;
	int status;

	if (msg == NULL) return -1;
	if (key == NULL) return -1;

	/* Special case handling */
	special = NULL;

	/* if query modifier is set but op is zero, default to equality test */
	if ((op != 0) && ((op & ASL_QUERY_OP_TRUE) == 0)) op |= ASL_QUERY_OP_EQUAL;

	/* convert "Level" values to "0" through "7" */
	if (streq(key, ASL_KEY_LEVEL))
	{
		if (val == NULL) val = "7";
		else if ((val[0] >= '0') && (val[0] <= '7') && (val[1] == '\0')) /* do nothing */;
		else if (strcaseeq(val, ASL_STRING_EMERG)) val = "0";
		else if (strcaseeq(val, ASL_STRING_ALERT)) val = "1";
		else if (strcaseeq(val, ASL_STRING_CRIT)) val = "2";
		else if (strcaseeq(val, ASL_STRING_ERR)) val = "3";
		else if (strcaseeq(val, ASL_STRING_WARNING)) val = "4";
		else if (strcaseeq(val, ASL_STRING_NOTICE)) val = "5";
		else if (strcaseeq(val, ASL_STRING_INFO)) val = "6";
		else if (strcaseeq(val, ASL_STRING_DEBUG)) val = "7";
		else val = "7";
	}

	/* strip trailing newlines from "Message" values */
	if ((streq(key, ASL_KEY_MSG)) && (val != NULL))
	{
		len = strlen(val);
		i = len;
		while ((i > 0) && (val[i - 1] == '\n')) i--;
		if (i == 0) val = NULL;
		else if (i < len)
		{
			/* use buf if it is big enough, else malloc a temporary buffer */
			if (i < sizeof(buf))
			{
				memcpy(buf, val, i);
				buf[i] = 0;
				val = (const char *)buf;
			}
			else
			{
				special = malloc(i + 1);
				if (special == NULL) return -1;
				memcpy(special, val, i);
				special[i] = 0;
				val = (const char *)special;
			}
		}
	}

	status = _asl_msg_set_kvo(msg, key, val, op);

	if (special != NULL) free(special);
	return status;
}

int
asl_msg_set_key_val(asl_msg_t *msg, const char *key, const char *val)
{
	return asl_msg_set_key_val_op(msg, key, val, 0);
}

static void
_asl_msg_unset_page_slot(asl_msg_t *page, uint32_t slot)
{
	char *ext;
	uint16_t k, v;

	if (page == NULL) return;

	if (slot >= _slot_count(page)) return;

	k = _get_slot_key(page, slot);
	v = _get_slot_val(page, slot);

	if (k == ASL_MSG_SLOT_FREE) return;

	if ((k & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
	{
		memcpy(&ext, page->data + (k & ASL_MSG_OFFSET_MASK), sizeof(char *));
		page->mem_size -= (strlen(ext) + 1);
		free(ext);
	}

	if ((v & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
	{
		memcpy(&ext, page->data + (v & ASL_MSG_OFFSET_MASK), sizeof(char *));
		page->mem_size -= (strlen(ext) + 1);
		free(ext);
	}

	_set_slot_key(page, slot, ASL_MSG_SLOT_FREE);
	_set_slot_val(page, slot, ASL_MSG_SLOT_FREE);
	_set_slot_op(page, slot, 0);

	page->count--;
}

/*
 * asl_msg_unset
 * Frees external key and val strings, but does not try to reclaim data space.
 */
void
asl_msg_unset(asl_msg_t *msg, const char *key)
{
	uint32_t i, slot;
	asl_msg_t *page;

	if (msg == NULL) return;
	if (key == NULL) return;

	slot = IndexNull;
	page = NULL;

	i = _asl_msg_index(msg, key, &slot, &page);
	if (i == IndexNull) return;

	_asl_msg_unset_page_slot(page, slot);
}

void
asl_msg_unset_index(asl_msg_t *msg, uint32_t n)
{
	uint32_t slot = IndexNull;
	asl_msg_t *page = NULL;

	if (msg == NULL) return;

	if  (0 != _asl_msg_resolve_index(msg, n, &page, &slot)) return;
	_asl_msg_unset_page_slot(page, slot);
}

#pragma mark -
#pragma mark copy and merge

/*
 * Merge a key / val into a message (only ASL_TYPE_MSG).
 * Adds the key / val if the key is not found.
 * Does not replace the value if the key is found.
 */
static void
_asl_msg_merge_key_val_op(asl_msg_t *msg, const char *key, const char *val, uint16_t op)
{
	uint32_t i, slot;
	asl_msg_t *page;

	if (msg == NULL) return;
	if (key == NULL) return;

	slot = IndexNull;
	page = NULL;

	i = _asl_msg_index(msg, key, &slot, &page);
	if (i != IndexNull) return;

	asl_msg_set_key_val_op(msg, key, val, op);
}

/*
 * Merge msg into target (does not replace existing keys).
 * Creates a new asl_msg_t if target is NULL.
 * Returns target.
 */
asl_msg_t *
asl_msg_merge(asl_msg_t *target, asl_msg_t *msg)
{
	uint32_t x, type, isnew = 0;
	uint16_t op;
	const char *key, *val;

	if (msg == NULL) return target;

	type = asl_get_type((asl_object_t)msg);

	if (target == NULL)
	{
		isnew = 1;
		target = asl_msg_new(type);
	}

	for (x = asl_msg_fetch(msg, 0, &key, &val, &op); x != IndexNull; x =asl_msg_fetch(msg, x, &key, &val, &op))
	{
		if (type == ASL_TYPE_MSG) op = 0;
		if (isnew == 1) asl_msg_set_key_val_op(target, key, val, op);
		else _asl_msg_merge_key_val_op(target, key, val, op);
	}

	return target;
}

/*
 * replace key/value pairs from msg in target
 * Creates a new asl_msg_t if target is NULL.
 * Returns target.
 */
static asl_msg_t *
asl_msg_replace(asl_msg_t *target, asl_msg_t *msg)
{
	uint32_t x, type;
	uint16_t op;
	const char *key, *val;

	if (msg == NULL) return target;

	type = asl_get_type((asl_object_t)msg);

	if (target == NULL) target = asl_msg_new(type);

	for (x = asl_msg_fetch(msg, 0, &key, &val, &op); x != IndexNull; x =asl_msg_fetch(msg, x, &key, &val, &op))
	{
		if (type == ASL_TYPE_MSG) op = 0;
		asl_msg_set_key_val_op(target, key, val, op);
	}

	return target;
}


/*
 * Copy msg.
 */
asl_msg_t *
asl_msg_copy(asl_msg_t *msg)
{
	return asl_msg_merge(NULL, msg);
}

#pragma mark -
#pragma mark compare and test

/*
 * Compare messages
 */
static int
_asl_msg_equal(asl_msg_t *a, asl_msg_t *b)
{
	uint32_t x;
	uint16_t oa, ob;
	const char *key, *va, *vb;

	if (asl_msg_count(a) != asl_msg_count(b)) return 0;

	key = NULL;
	va = NULL;
	oa = 0;

	for (x = asl_msg_fetch(a, 0, &key, &va, &oa); x != IndexNull; x = asl_msg_fetch(a, x, &key, &va, &oa))
	{
		if (asl_msg_lookup(b, key, &vb, &ob) != 0) return 0;
		if (strcmp(va, vb)) return 0;
		if ((a->asl_type == ASL_TYPE_QUERY) && (oa != ob)) return 0;
	}

	return 1;
}

static int
_asl_isanumber(const char *s)
{
	int i;

	if (s == NULL) return 0;

	i = 0;
	if ((s[0] == '-') || (s[0] == '+')) i = 1;

	if (s[i] == '\0') return 0;

	for (; s[i] != '\0'; i++)
	{
		if (!isdigit(s[i])) return 0;
	}

	return 1;
}

static int
_asl_msg_basic_test(uint32_t op, const char *q, const char *m, uint32_t n)
{
	int cmp;
	uint32_t t;
	int64_t nq, nm;
	int rflags;
	regex_t rex;

	t = op & ASL_QUERY_OP_TRUE;

	/* NULL value from query or message string fails */
	if ((q == NULL) || (m == NULL)) return (t & ASL_QUERY_OP_NOT_EQUAL);

	if (op & ASL_QUERY_OP_REGEX)
	{
		/* greater than or less than make no sense in substring search */
		if ((t == ASL_QUERY_OP_GREATER) || (t == ASL_QUERY_OP_LESS)) return 0;

		memset(&rex, 0, sizeof(regex_t));

		rflags = REG_EXTENDED | REG_NOSUB;
		if (op & ASL_QUERY_OP_CASEFOLD) rflags |= REG_ICASE;

		/* A bad reqular expression matches nothing */
		if (regcomp(&rex, q, rflags) != 0) return (t & ASL_QUERY_OP_NOT_EQUAL);

		cmp = regexec(&rex, m, 0, NULL, 0);
		regfree(&rex);

		if (t == ASL_QUERY_OP_NOT_EQUAL) return (cmp != 0);
		return (cmp == 0);
	}

	if (op & ASL_QUERY_OP_NUMERIC)
	{
		if (_asl_isanumber(q) == 0) return (t == ASL_QUERY_OP_NOT_EQUAL);
		if (_asl_isanumber(m) == 0) return (t == ASL_QUERY_OP_NOT_EQUAL);

		nq = atoll(q);
		nm = atoll(m);

		switch (t)
		{
			case ASL_QUERY_OP_EQUAL: return (nm == nq);
			case ASL_QUERY_OP_GREATER: return (nm > nq);
			case ASL_QUERY_OP_GREATER_EQUAL: return (nm >= nq);
			case ASL_QUERY_OP_LESS: return (nm < nq);
			case ASL_QUERY_OP_LESS_EQUAL: return (nm <= nq);
			case ASL_QUERY_OP_NOT_EQUAL: return (nm != nq);
			default: return (t == ASL_QUERY_OP_NOT_EQUAL);
		}
	}

	cmp = 0;
	if (op & ASL_QUERY_OP_CASEFOLD)
	{
		if (n == 0) cmp = strcasecmp(m, q);
		else cmp = strncasecmp(m, q, n);
	}
	else
	{
		if (n == 0) cmp = strcmp(m, q);
		else cmp = strncmp(m, q, n);
	}

	switch (t)
	{
		case ASL_QUERY_OP_EQUAL: return (cmp == 0);
		case ASL_QUERY_OP_GREATER: return (cmp > 0);
		case ASL_QUERY_OP_GREATER_EQUAL: return (cmp >= 0);
		case ASL_QUERY_OP_LESS: return (cmp < 0);
		case ASL_QUERY_OP_LESS_EQUAL: return (cmp <= 0);
		case ASL_QUERY_OP_NOT_EQUAL: return (cmp != 0);
	}

	return (t == ASL_QUERY_OP_NOT_EQUAL);
}

static int
_asl_msg_test_substring(uint32_t op, const char *q, const char *m)
{
	uint32_t t, i, d, lm, lq, match, newop;

	t = op & ASL_QUERY_OP_TRUE;

	lm = 0;
	if (m != NULL) lm = strlen(m);

	lq = 0;
	if (q != NULL) lq = strlen(q);

	/* NULL is a substring of any string */
	if (lq == 0) return (t & ASL_QUERY_OP_EQUAL);

	/* A long string is defined to be not equal to a short string */
	if (lq > lm) return (t == ASL_QUERY_OP_NOT_EQUAL);

	/* greater than or less than make no sense in substring search */
	if ((t == ASL_QUERY_OP_GREATER) || (t == ASL_QUERY_OP_LESS)) return 0;

	/*
	 * We scan the string doing an equality test.
	 * If the input test is equality, we stop as soon as we hit a match.
	 * Otherwise we keep scanning the whole message string.
	 */
	newop = op & 0xff0;
	newop |= ASL_QUERY_OP_EQUAL;

	match = 0;
	d = lm - lq;
	for (i = 0; i <= d; i++)
	{
		if (_asl_msg_basic_test(newop, q, m + i, lq) != 0)
		{
			if (t & ASL_QUERY_OP_EQUAL) return 1;
			match++;
		}
	}

	/* If the input test was for equality, no matches were found */
	if (t & ASL_QUERY_OP_EQUAL) return 0;

	/* The input test was for not equal.  Return true if no matches were found */
	return (match == 0);
}

static int
_asl_msg_test_prefix(uint32_t op, const char *q, const char *m)
{
	uint32_t lm, lq, t;

	t = op & ASL_QUERY_OP_TRUE;

	lm = 0;
	if (m != NULL) lm = strlen(m);

	lq = 0;
	if (q != NULL) lq = strlen(q);

	/* NULL is a prefix of any string */
	if (lq == 0) return (t & ASL_QUERY_OP_EQUAL);

	/* A long string is defined to be not equal to a short string */
	if (lq > lm) return (t == ASL_QUERY_OP_NOT_EQUAL);

	/* Compare two equal-length strings */
	return _asl_msg_basic_test(op, q, m, lq);
}

static int
_asl_msg_test_suffix(uint32_t op, const char *q, const char *m)
{
	uint32_t lm, lq, d, t;

	t = op & ASL_QUERY_OP_TRUE;

	lm = 0;
	if (m != NULL) lm = strlen(m);

	lq = 0;
	if (q != NULL) lq = strlen(q);

	/* NULL is a suffix of any string */
	if (lq == 0) return (t & ASL_QUERY_OP_EQUAL);

	/* A long string is defined to be not equal to a short string */
	if (lq > lm) return (t == ASL_QUERY_OP_NOT_EQUAL);

	/* Compare two equal-length strings */
	d = lm - lq;
	return _asl_msg_basic_test(op, q, m + d, lq);
}

/*
 * Splits out prefix, suffix, and substring tests.
 * Sends the rest to _asl_msg_basic_test().
 */
static int
_asl_msg_test_expression(uint32_t op, const char *q, const char *m)
{
	uint32_t t;

	t = op & ASL_QUERY_OP_TRUE;
	if (t == ASL_QUERY_OP_TRUE) return 1;

	if (op & ASL_QUERY_OP_PREFIX)
	{
		if (op & ASL_QUERY_OP_SUFFIX) return _asl_msg_test_substring(op, q, m);
		return _asl_msg_test_prefix(op, q, m);
	}
	if (op & ASL_QUERY_OP_SUFFIX) return _asl_msg_test_suffix(op, q, m);

	return _asl_msg_basic_test(op, q, m, 0);
}

/*
 * Special case for comparing time values.
 * If both inputs are time strings, this compares the time
 * value in seconds.  Otherwise it just does normal matching.
 */
static int
_asl_msg_test_time_expression(uint32_t op, const char *q, const char *m)
{
	time_t tq, tm;
	uint32_t t;

	if ((op & ASL_QUERY_OP_PREFIX) || (op & ASL_QUERY_OP_SUFFIX) || (op & ASL_QUERY_OP_REGEX)) return _asl_msg_test_expression(op, q, m);
	if ((q == NULL) || (m == NULL)) return _asl_msg_test_expression(op, q, m);

	tq = asl_core_parse_time(q, NULL);
	if (tq < 0) return _asl_msg_test_expression(op, q, m);

	tm = asl_core_parse_time(m, NULL);
	if (tm < 0) return _asl_msg_test_expression(op, q, m);

	t = op & ASL_QUERY_OP_TRUE;

	switch (t)
	{
		case ASL_QUERY_OP_FALSE:
		{
			return 0;
		}
		case ASL_QUERY_OP_EQUAL:
		{
			if (tm == tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_GREATER:
		{
			if (tm > tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_GREATER_EQUAL:
		{
			if (tm >= tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_LESS:
		{
			if (tm < tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_LESS_EQUAL:
		{
			if (tm <= tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_NOT_EQUAL:
		{
			if (tm != tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_TRUE:
		{
			return 1;
		}
	}

	/* NOTREACHED */
	return 0;
}

/* test a query against a message */
__private_extern__ int
_asl_msg_test(asl_msg_t *q, asl_msg_t *m)
{
	uint32_t i, t, x;
	uint16_t op;
	int cmp;
	const char *kq, *vq, *vm;

	/*
	 * Check each simple expression (key op val) separately.
	 * The query suceeds (returns 1) if all simple expressions
	 * succeed (i.e. AND the simple expressions).
	 */

	kq = NULL;
	vq = NULL;
	op = 0;

	for (x = asl_msg_fetch(q, 0, &kq, &vq, &op); x != IndexNull; x = asl_msg_fetch(q, x, &kq, &vq, &op))
	{
		/* Find query key in the message */
		vm = NULL;
		i = asl_msg_lookup(m, kq, &vm, NULL);

		/* ASL_QUERY_OP_TRUE tests if key is present in the message */
		t = op & ASL_QUERY_OP_TRUE;
		if (t == ASL_QUERY_OP_TRUE)
		{
			if (i != 0) return 0;
			continue;
		}

		/* ASL_QUERY_OP_FALSE tests if the key is NOT present in the message */
		if (t == ASL_QUERY_OP_FALSE)
		{
			if (i == 0) return 0;
			continue;
		}

		if (i != 0)
		{
			/* the message does NOT have query key - fail unless we are testing not equal */
			if (t == ASL_QUERY_OP_NOT_EQUAL) continue;
			return 0;
		}

		cmp = 1;
		if (streq(kq, ASL_KEY_TIME))
		{
			cmp = _asl_msg_test_time_expression(op, vq, vm);
		}
		else
		{
			cmp = _asl_msg_test_expression(op, vq, vm);
		}

		if (cmp == 0) return 0;
	}

	return 1;
}

/* returns 1 if a and b match, 0 otherwise */
int
asl_msg_cmp(asl_msg_t *a, asl_msg_t *b)
{

	if (a == NULL) return 0;
	if (b == NULL) return 0;

	if (a->asl_type == b->asl_type) return _asl_msg_equal(a, b);
	if (a->asl_type == ASL_TYPE_QUERY) return _asl_msg_test(a, b);
	return _asl_msg_test(b, a);
}

/*
 * Test a message against a query list.
 * Returns 1 if msg matches any query in the list, 0 otherwise.
 * Returns 1 if the query list is NULL or empty.
 */
int
asl_msg_cmp_list(asl_msg_t *msg, asl_msg_list_t *list)
{
	uint32_t i;

	if (msg == NULL) return 0;
	if (list == NULL) return 1;
	if (list->count == 0) return 1;

	for (i = 0; i < list->count; i++)
	{
		if (_asl_msg_test(list->msg[i], msg)) return 1;
	}

	return 0;
}

#pragma mark -
#pragma mark string representation

static char *
_asl_time_string(const char *infmt, const char *str, const char *nano)
{
	time_t tick, off;
	struct tm stm;
	char *ltime, *out, *p, *q;
	char ltbuf[32], nanobuf[16], fmt[32], zstr[8];
	time_t zh, zm;
	uint32_t subsec = 0;
	bool neg;

	out = NULL;
	memset(zstr, 0, sizeof(zstr));
	zh = 0;
	zm = 0;
	off = 0;
	neg = false;

	/*
	 * The longest infmt string we understand is "-hh:mm.N" (8 chars), so
	 * it is safe to ignore the input if it doesn't fit in the temp buffer.
	 * The default is local time zone format.
	 */
	if (infmt == NULL)
	{
		snprintf(fmt, sizeof(fmt), "local");
	}
	else if (strlen(infmt) >= sizeof (fmt))
	{
		snprintf(fmt, sizeof(fmt), "local");
	}
	else
	{
		snprintf(fmt, sizeof(fmt), "%s", infmt);

		p = fmt;
		q = strchr(fmt, '.');
		if (q != NULL)
		{
			*q = '\0';
			q++;
			if (q != '\0') subsec = atoi(q);
		}
	}

	nanobuf[0] = '\0';

	if (subsec > 0)
	{
		uint32_t nsec = 0;
		if (nano != NULL) nsec = atoi(nano);
		snprintf(nanobuf, sizeof(nanobuf), ".%09u", nsec);
		if (subsec > 9) subsec = 9;
		nanobuf[subsec + 1] = '\0';
	}

	tick = 0;
	if (str != NULL) tick = asl_core_parse_time(str, NULL);

	if ((!strcasecmp(fmt, "lcl")) || (!strcasecmp(fmt, "local")))
	{
		ltime = ctime_r(&tick, ltbuf);
		if (ltime == NULL) return NULL;
		ltime[19] = '\0';
		asprintf(&out, "%s%s", ltime + 4, nanobuf);
		return out;
	}

	if ((!strcasecmp(fmt, "jz")) || (!strcasecmp(fmt, "iso8601")) || (!strcasecmp(fmt, "iso8601e")))
	{
		char sep = ' ';
		if (!strncasecmp(fmt, "iso8601", 7)) sep = 'T';

		if (NULL == localtime_r(&tick, &stm)) return NULL;

		off = stm.tm_gmtoff;
		if ((neg = (off < 0))) off *= -1;
		zh = off / 3600;
		off %= 3600;
		zm = off / 60;

		if (zm == 0) snprintf(zstr, sizeof(zstr), "%c%02lld", neg ? '-' : '+', (long long) zh);
		else snprintf(zstr, sizeof(zstr), "%c%02lld:%02lld", neg ? '-' : '+', (long long) zh, (long long) zm);

		asprintf(&out, "%d-%02d-%02d%c%02d:%02d:%02d%s%s", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, sep, stm.tm_hour, stm.tm_min, stm.tm_sec, nanobuf, zstr);
		return out;
	}

	if (!strcasecmp(fmt, "iso8601b"))
	{
		if (NULL == localtime_r(&tick, &stm)) return NULL;

		off = stm.tm_gmtoff;
		if ((neg = (off < 0))) off *= -1;
		zh = off / 3600;
		off %= 3600;
		zm = off / 60;

		if (zm == 0) snprintf(zstr, sizeof(zstr), "%c%02lld", neg ? '-' : '+', (long long) zh);
		else snprintf(zstr, sizeof(zstr), "%c%02lld:%02lld", neg ? '-' : '+', (long long) zh, (long long) zm);

		asprintf(&out, "%d%02d%02dT%02d%02d%02d%s%s", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, nanobuf, zstr);
		return out;
	}

	if ((!strcasecmp(fmt, "sec")) || (!strcasecmp(fmt, "raw")))
	{
		asprintf(&out, "%llu%s", (unsigned long long) tick, nanobuf);
		return out;
	}

	if (!strcasecmp(fmt, "j"))
	{
		if (NULL == localtime_r(&tick, &stm)) return NULL;
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02d%s", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, nanobuf);
		return out;
	}

	if ((!strcasecmp(fmt, "utc")) || (!strcasecmp(fmt, "zulu")) || (!strcasecmp(fmt, "iso8601z")) || (!strcasecmp(fmt, "iso8601ez")))
	{
		char sep = ' ';
		if (!strncasecmp(fmt, "iso8601", 7)) sep = 'T';

		if (NULL == gmtime_r(&tick, &stm)) return NULL;
		asprintf(&out, "%d-%02d-%02d%c%02d:%02d:%02d%sZ", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, sep, stm.tm_hour, stm.tm_min, stm.tm_sec, nanobuf);
		return out;
	}

	if (!strcasecmp(fmt, "iso8601bz"))
	{
		if (NULL == gmtime_r(&tick, &stm)) return NULL;
		asprintf(&out, "%d%02d%02dT%02d%02d%02d%sZ", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, nanobuf);
		return out;
	}

	if ((fmt[1] == '\0') && (((fmt[0] >= 'a') && (fmt[0] <= 'z')) || ((fmt[0] >= 'A') && (fmt[0] <= 'Z'))))
	{
		char z = fmt[0];
		if (z >= 'a') z -= 32;

		if (z == 'Z') off = 0;
		else if ((z >= 'A') && (z <= 'I')) off = ((z - 'A') + 1) * SEC_PER_HOUR;
		else if ((z >= 'K') && (z <= 'M')) off = (z - 'A') * SEC_PER_HOUR;
		else if ((z >= 'N') && (z <= 'Y')) off = ('M' - z) * SEC_PER_HOUR;
		else return NULL;
	}
	else
	{
		if (fmt[0] == '-') neg = true;
		if ((*p == '-') || (*p == '+')) p++;
		if ((*p) >= '0' && (*p <= '9'))
		{
			zh = atoi(p);

			p = strchr(p, ':');
			if (p != NULL) zm = atoi(p + 1);
		}
		else
		{
			return NULL;
		}

		off = (zh * 3600) + (zm * 60);
		if (neg) off *= -1;

		if (zm == 0) snprintf(zstr, sizeof(zstr), "%c%02lld", neg ? '-' : '+', (long long) zh);
		else snprintf(zstr, sizeof(zstr), "%c%02lld:%02lld", neg ? '-' : '+', (long long) zh, (long long) zm);
	}


	tick += off;

	memset(&stm, 0, sizeof (struct tm));
	if (NULL == gmtime_r(&tick, &stm)) return NULL;

	if ((fmt[0] >= 'A') && (fmt[0] <= 'Z'))
	{
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02d%s%c", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, nanobuf, fmt[0]);
	}
	else if ((fmt[0] >= 'a') && (fmt[0] <= 'z'))
	{
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02d%s%c", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, nanobuf, fmt[0] - 32);
	}
	else
	{
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02d%s%s", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, nanobuf, zstr);
	}

	return out;
}


/* called from asl_format_message and _asl_send_message */
__private_extern__ asl_string_t *
asl_msg_to_string_raw(uint32_t encoding, asl_msg_t *msg, const char *tfmt)
{
	uint32_t i, x, count;
	const char *key, *val, *nano;
	asl_string_t *str;

	if (msg == NULL) return NULL;

	count = asl_msg_count(msg);
	if (count == 0) return NULL;

	str = asl_string_new(encoding);
	if (str == NULL) return NULL;

	key = NULL;
	val = NULL;
	i = 0;

	nano = NULL;
	asl_msg_lookup(msg, ASL_KEY_TIME_NSEC, &nano, NULL);

	for (x = asl_msg_fetch(msg, 0, &key, &val, NULL); x != IndexNull; x = asl_msg_fetch(msg, x, &key, &val, NULL))
	{
		if (key == NULL) continue;

		if (i > 0) asl_string_append_char_no_encoding(str, ' ');

		asl_string_append_char_no_encoding(str, '[');
		asl_string_append_asl_key(str, key);

		if (!strcmp(key, ASL_KEY_TIME))
		{
			char *vtime = NULL;
			asl_string_append_char_no_encoding(str, ' ');

			if (val != NULL) vtime = _asl_time_string(tfmt, val, nano);

			if (vtime != NULL)
			{
				asl_string_append_no_encoding(str, vtime);
				free(vtime);
			}
			else
			{
				asl_string_append_char_no_encoding(str, '0');
			}
		}
		else if (val != NULL)
		{
			asl_string_append_char_no_encoding(str, ' ');
			asl_string_append(str, val);
		}

		asl_string_append_char_no_encoding(str, ']');

		i++;
	}

	return str;
}

asl_string_t *
asl_string_append_asl_msg(asl_string_t *str, asl_msg_t *msg)
{
	const char *key, *val;
	uint32_t i, x;
	uint16_t op;

	if (msg == NULL) return str;

	if (msg->asl_type == ASL_TYPE_QUERY) asl_string_append(str, "Q ");

	i = 0;
	for (x = asl_msg_fetch(msg, 0, &key, &val, &op); x != IndexNull; x = asl_msg_fetch(msg, x, &key, &val, &op))
	{
		if (i != 0)	asl_string_append_char_no_encoding(str, ' ');
		i++;

		asl_string_append_char_no_encoding(str, '[');

		if (msg->asl_type == ASL_TYPE_QUERY)
		{
			asl_string_append_op(str, op);
			asl_string_append_char_no_encoding(str, ' ');
		}

		asl_string_append_asl_key(str, key);

		if (val != NULL)
		{
			asl_string_append_char_no_encoding(str, ' ');
			asl_string_append(str, val);
		}

		asl_string_append_char_no_encoding(str, ']');
	}

	return str;
}

char *
asl_msg_to_string(asl_msg_t *msg, uint32_t *len)
{
	char *out;
	asl_string_t *str = asl_string_new(ASL_ENCODE_ASL);
	if (str == NULL) return NULL;

	str = asl_string_append_asl_msg(str, msg);
	*len = asl_string_length(str);
	out = asl_string_release_return_bytes(str);
	return out;
}

static uint32_t
_asl_msg_op_from_string(char *o)
{
	uint32_t op, i;

	op = ASL_QUERY_OP_NULL;

	if (o == NULL) return op;

	for (i = 0; o[i] != '\0'; i++)
	{
		if (o[i] == '.') return ASL_QUERY_OP_NULL;
		if (o[i] == 'C') op |= ASL_QUERY_OP_CASEFOLD;
		if (o[i] == 'R') op |= ASL_QUERY_OP_REGEX;
		if (o[i] == 'N') op |= ASL_QUERY_OP_NUMERIC;
		if (o[i] == 'S') op |= ASL_QUERY_OP_SUBSTRING;
		if (o[i] == 'A') op |= ASL_QUERY_OP_PREFIX;
		if (o[i] == 'Z') op |= ASL_QUERY_OP_SUFFIX;
		if (o[i] == '<') op |= ASL_QUERY_OP_LESS;
		if (o[i] == '>') op |= ASL_QUERY_OP_GREATER;
		if (o[i] == '=') op |= ASL_QUERY_OP_EQUAL;
		if (o[i] == '!') op |= ASL_QUERY_OP_NOT_EQUAL;
		if (o[i] == 'T') op |= ASL_QUERY_OP_TRUE;
	}

	return op;
}

static char *
_asl_msg_get_next_word(char **p, uint32_t *tt, uint32_t spacedel)
{
	char *str, *out, c, oval;
	uint32_t i, len, n, outlen;

	*tt = TOKEN_NULL;

	if (p == NULL) return NULL;
	if (*p == NULL) return NULL;
	if (**p == '\0') return NULL;

	/* skip one space if it's there (word separator) */
	if (**p == ' ') (*p)++;

	/* skip leading white space */
	if (spacedel != 0)
	{
		while ((**p == ' ') || (**p == '\t')) (*p)++;
	}

	if (**p == '\0') return NULL;
	if (**p == '\n') return NULL;

	str = *p;

	/* opening [ */
	if (**p == '[')
	{
		*tt = TOKEN_OPEN;

		(*p)++;
		out = malloc(2);
		if (out == NULL) return NULL;

		out[0] = '[';
		out[1] = '\0';
		return out;
	}

	/* scan for token and calulate it's length (input and decoded output len) */
	len = 0;
	outlen = 0;

	forever
	{
		c = str[len];

		/* stop scanning when we hit a delimiter */
		if (((spacedel != 0) && (c == ' ')) || (c == ']') || (c == '\0')) break;

		if (c == '\\')
		{
			len++;
			c = str[len];
			if ((c == 'a') || (c == 'b') || (c == 't') || (c == 'n') || (c == 'v') || (c == 'f') || (c == 'r') || (c == 's') || (c == '[') || (c == '\\') || (c == ']'))
			{
			}
			else if (c == '^')
			{
				if (str[++len] == '\0') return NULL;
			}
			else if (c == 'M')
			{
				if (str[++len] == '\0') return NULL;
				if (str[++len] == '\0') return NULL;
			}
			else if ((c >= '0') && (c <= '3'))
			{
				if (str[++len] == '\0') return NULL;
				if (str[++len] == '\0') return NULL;
			}
			else
			{
				return NULL;
			}
		}

		len++;
		outlen++;
	}

	(*p) += len;

	if ((len == 0) && (**p == ']'))
	{
		*tt = TOKEN_CLOSE;
		(*p)++;
		out = malloc(2);
		if (out == NULL) return NULL;

		out[0] = ']';
		out[1] = '\0';
		return out;
	}

	*tt = TOKEN_INT;

	out = malloc(outlen + 1);
	if (out == NULL) return NULL;

	n = 0;
	for (i = 0; i < len; i++)
	{
		c = str[i];

		if (c == '\\')
		{
			*tt = TOKEN_WORD;

			i++;
			c = str[i];
			if (c == 'a')
			{
				out[n++] = '\a';
			}
			else if (c == 'b')
			{
				out[n++] = '\b';
			}
			else if (c == 't')
			{
				out[n++] = '\t';
			}
			else if (c == 'n')
			{
				out[n++] = '\n';
			}
			else if (c == 'v')
			{
				out[n++] = '\v';
			}
			else if (c == 'f')
			{
				out[n++] = '\f';
			}
			else if (c == 'r')
			{
				out[n++] = '\r';
			}
			else if (c == 's')
			{
				out[n++] = ' ';
			}
			else if (c == '[')
			{
				out[n++] = '[';
			}
			else if (c == '\\')
			{
				out[n++] = '\\';
			}
			else if (c == ']')
			{
				out[n++] = ']';
			}
			else if (c == '^')
			{
				i++;
				if (str[i] == '?') out[n++] = 127;
				else out[n++] = str[i] - 64;
			}
			else if (c == 'M')
			{
				i++;
				c = str[i];
				if (c == '^')
				{
					i++;
					if (str[i] == '?') out[n++] = 255;
					else out[n++] = str[i] + 64;
				}
				else if (c == '-')
				{
					i++;
					out[n++] = str[i] + 128;
				}
				else
				{
					*tt = TOKEN_NULL;
					free(out);
					return NULL;
				}

			}
			else if ((c >= '0') && (c <= '3'))
			{
				oval = (c - '0') * 64;

				i++;
				c = str[i];
				if ((c < '0') || (c > '7'))
				{
					*tt = TOKEN_NULL;
					free(out);
					return NULL;
				}

				oval += ((c - '0') * 8);

				i++;
				c = str[i];
				if ((c < '0') || (c > '7'))
				{
					*tt = TOKEN_NULL;
					free(out);
					return NULL;
				}

				oval += (c - '0');

				out[n++] = oval;
			}
			else
			{
				*tt = TOKEN_NULL;
				free(out);
				return NULL;
			}
		}
		else
		{

			if ((c < '0') || (c > '9')) *tt = TOKEN_WORD;
			out[n++] = c;
		}
	}

	out[n] = '\0';

	return out;
}

asl_msg_t *
asl_msg_from_string(const char *buf)
{
	uint32_t tt, type, op;
	char *k, *v, *o, *p;
	asl_msg_t *out;

	if (buf == NULL) return NULL;

	type = ASL_TYPE_MSG;
	p = (char *)buf;

	k = _asl_msg_get_next_word(&p, &tt, 1);
	if (k == NULL) return NULL;

	if (streq(k, "Q"))
	{
		type = ASL_TYPE_QUERY;
		free(k);

		k = _asl_msg_get_next_word(&p, &tt, 1);
	}
	else if (tt == TOKEN_INT)
	{
		/* Leading integer is a string length - skip it */
		free(k);
		k = _asl_msg_get_next_word(&p, &tt, 1);
		if (k == NULL) return NULL;
	}

	out = asl_msg_new(ASL_TYPE_MSG);
	if (out == NULL) return NULL;

	out->asl_type = type;

	/* OPEN WORD [WORD [WORD]] CLOSE */
	while (k != NULL)
	{
		op = ASL_QUERY_OP_NULL;

		if (tt != TOKEN_OPEN)
		{
			asl_msg_release(out);
			return NULL;
		}

		free(k);

		/* get op for query type */
		if (type == ASL_TYPE_QUERY)
		{
			o = _asl_msg_get_next_word(&p, &tt, 1);
			if ((o == NULL) || (tt != TOKEN_WORD))
			{
				if (o != NULL) free(o);
				asl_msg_release(out);
				return NULL;
			}

			op = _asl_msg_op_from_string(o);
			free(o);
		}

		k = _asl_msg_get_next_word(&p, &tt, 1);
		if (tt == TOKEN_INT) tt = TOKEN_WORD;
		if ((k == NULL) || (tt != TOKEN_WORD))
		{
			if (k != NULL) free(k);
			asl_msg_release(out);
			return NULL;
		}

		v = _asl_msg_get_next_word(&p, &tt, 0);
		if (tt == TOKEN_INT) tt = TOKEN_WORD;
		if (v == NULL)
		{
			asl_msg_set_key_val_op(out, k, NULL, op);
			free(k);
			break;
		}

		if (tt == TOKEN_CLOSE)
		{
			asl_msg_set_key_val_op(out, k, NULL, op);
		}
		else if (tt == TOKEN_WORD)
		{
			asl_msg_set_key_val_op(out, k, v, op);
		}
		else
		{
			if (k != NULL) free(k);
			if (v != NULL) free(v);
			asl_msg_release(out);
			return NULL;
		}

		if (k != NULL) free(k);
		if (v != NULL) free(v);

		if (tt != TOKEN_CLOSE)
		{
			k = _asl_msg_get_next_word(&p, &tt, 1);
			if (k == NULL) break;

			if (tt != TOKEN_CLOSE)
			{
				asl_msg_release(out);
				return NULL;
			}

			free(k);
		}

		k = _asl_msg_get_next_word(&p, &tt, 1);
		if (k == NULL) break;
	}

	return out;
}

static const char *
_asl_level_string(int level)
{
	if (level == ASL_LEVEL_EMERG) return ASL_STRING_EMERG;
	if (level == ASL_LEVEL_ALERT) return ASL_STRING_ALERT;
	if (level == ASL_LEVEL_CRIT) return ASL_STRING_CRIT;
	if (level == ASL_LEVEL_ERR) return ASL_STRING_ERR;
	if (level == ASL_LEVEL_WARNING) return ASL_STRING_WARNING;
	if (level == ASL_LEVEL_NOTICE) return ASL_STRING_NOTICE;
	if (level == ASL_LEVEL_INFO) return ASL_STRING_INFO;
	if (level == ASL_LEVEL_DEBUG) return ASL_STRING_DEBUG;
	return "unknown";
}

static const char *
_asl_level_char(int level)
{
	if (level == ASL_LEVEL_EMERG) return "P";
	if (level == ASL_LEVEL_ALERT) return "A";
	if (level == ASL_LEVEL_CRIT) return "C";
	if (level == ASL_LEVEL_ERR) return "E";
	if (level == ASL_LEVEL_WARNING) return "W";
	if (level == ASL_LEVEL_NOTICE) return "N";
	if (level == ASL_LEVEL_INFO) return "I";
	if (level == ASL_LEVEL_DEBUG) return "D";
	return "?";
}

/*
 * Find the value for a key in a message and append a formatted value to str.
 * kf may be a simple (no embedded white space) key, or one of (key) or ((key)(format)).
 * WARNING: modifies kf!
 */
static asl_string_t *
_asl_string_append_value_for_key_format(asl_string_t *str, asl_msg_t *msg, char *kf, const char *tfmt)
{
	uint32_t i, get_fmt;
	int status;
	char *key, *fmt;
	const char *mval, *nano;

	if (str == NULL) return NULL;
	if (msg == NULL) return str;
	if (kf == NULL) return str;

	key = NULL;
	fmt = NULL;
	get_fmt = 0;

	for (i = 0; kf[i] != '\0'; i++)
	{
		if (kf[i] == ')')
		{
			kf[i] = '\0';
			get_fmt = 1;
		}
		else if (kf[i] != '(')
		{
			if (key == NULL) key = kf + i;
			else if ((get_fmt == 1) && (fmt == NULL)) fmt = kf + i;
		}
	}

	if (key == NULL) return str;

	nano = NULL;
	asl_msg_lookup(msg, ASL_KEY_TIME_NSEC, &nano, NULL);

	status = asl_msg_lookup(msg, key, &mval, NULL);
	if ((status != 0) || (mval == NULL)) return str;

	if (!strcmp(key, ASL_KEY_TIME))
	{
		char *fval = NULL;

		/* format in $((Time)(fmt)) overrides tfmt */
		if (fmt == NULL)
		{
			fval = _asl_time_string(tfmt, mval, nano);
		}
		else
		{
			fval = _asl_time_string(fmt, mval, nano);
		}

		if (fval != NULL)
		{
			asl_string_append_no_encoding(str, fval);
			free(fval);
		}
		else
		{
			asl_string_append_char_no_encoding(str, '0');
		}

		return str;
	}

	/* Level: num str */
	if (!strcmp(key, ASL_KEY_LEVEL))
	{
		if (fmt == NULL)
		{
			asl_string_append_no_encoding(str, mval);
		}
		else if (!strcmp(fmt, "str"))
		{
			mval = _asl_level_string(atoi(mval));
			asl_string_append_no_encoding(str, mval);
		}
		else if (!strcmp(fmt, "char"))
		{
			mval = _asl_level_char(atoi(mval));
			asl_string_append_no_encoding(str, mval);
		}
		else
		{
			asl_string_append_no_encoding(str, mval);
		}

		return str;
	}

	return asl_string_append(str, mval);
}

/*
 * format a message for printing
 * out parameter len returns string length including trailing NUL
 */
char *
asl_format_message(asl_msg_t *msg, const char *mfmt, const char *tfmt, uint32_t text_encoding, uint32_t *len)
{
	char *out, *vtime, *k, c, skey[512], tfmt_ext[16];
	const char *vhost, *vpid, *vsender, *vmessage, *vlevel, *vrefproc, *vrefpid, *v, *key, *val, *nano;
	int i, j, l, mf, paren, oval, level;
	uint32_t x, cursor;
	asl_string_t *str;
	uint8_t *b64;

	out = NULL;
	*len = 0;

	if (msg == NULL) return NULL;

	mf = MFMT_RAW;

	if (mfmt == NULL) mf = MFMT_RAW;
	else if (!strcmp(mfmt, ASL_MSG_FMT_RAW)) mf = MFMT_RAW;
	else if (!strcmp(mfmt, ASL_MSG_FMT_STD)) mf = MFMT_STD;
	else if (!strcmp(mfmt, ASL_MSG_FMT_BSD)) mf = MFMT_BSD;
	else if (!strcmp(mfmt, ASL_MSG_FMT_XML)) mf = MFMT_XML;
	else if (!strcmp(mfmt, ASL_MSG_FMT_MSG)) mf = MFMT_MSG;
	else if ((!strncmp(mfmt, ASL_MSG_FMT_RAW, 3)) && (mfmt[3] == '.'))
	{
		mf = MFMT_RAW;
		if ((tfmt == NULL) && (mfmt[4] != '\0'))
		{
			snprintf(tfmt_ext, sizeof(tfmt_ext), "sec.%s", mfmt + 4);
			tfmt = (const char *)tfmt_ext;
		}
	}
	else if ((!strncmp(mfmt, ASL_MSG_FMT_STD, 3)) && (mfmt[3] == '.'))
	{
		mf = MFMT_STD;
		if ((tfmt == NULL) && (mfmt[4] != '\0'))
		{
			snprintf(tfmt_ext, sizeof(tfmt_ext), "lcl.%s", mfmt + 4);
			tfmt = (const char *)tfmt_ext;
		}
	}
	else if ((!strncmp(mfmt, ASL_MSG_FMT_BSD, 3)) && (mfmt[3] == '.'))
	{
		mf = MFMT_BSD;
		if ((tfmt == NULL) && (mfmt[4] != '\0'))
		{
			snprintf(tfmt_ext, sizeof(tfmt_ext), "lcl.%s", mfmt + 4);
			tfmt = (const char *)tfmt_ext;
		}
	}
	else mf = MFMT_STR;

	nano = NULL;
	asl_msg_lookup(msg, ASL_KEY_TIME_NSEC, &nano, NULL);

	if (mf == MFMT_RAW)
	{
		str = asl_msg_to_string_raw(text_encoding, msg, tfmt);
		asl_string_append_char_no_encoding(str, '\n');

		*len = asl_string_length(str);
		out = asl_string_release_return_bytes(str);
		return out;
	}

	if (mf == MFMT_MSG)
	{
		vmessage = NULL;

		if (asl_msg_lookup(msg, ASL_KEY_MSG, &vmessage, NULL) != 0) return NULL;

		str = asl_string_new(text_encoding);
		if (str == NULL) return NULL;

		asl_string_append(str, vmessage);
		asl_string_append_char_no_encoding(str, '\n');

		*len = asl_string_length(str);
		out = asl_string_release_return_bytes(str);
		return out;
	}

	if ((mf == MFMT_STD) || (mf == MFMT_BSD))
	{
		/* COMMON:  Mth dd hh:mm:ss host sender[pid] (refproc[refpid])*/
		/* BSD:  <COMMON>: message */
		/* STD:  <COMMON> <Level>: message */

		v = NULL;
		vtime = NULL;
		vhost = NULL;
		vsender = NULL;
		vpid = NULL;
		vmessage = NULL;
		vlevel = NULL;
		vrefproc = NULL;
		vrefpid = NULL;

		if (asl_msg_lookup(msg, ASL_KEY_TIME, &v, NULL) == 0)
		{
			vtime = _asl_time_string(tfmt, v, nano);
		}

		level = 7;
		if (asl_msg_lookup(msg, ASL_KEY_LEVEL, &vlevel, NULL) == 0)
		{
			if (vlevel != NULL) level = atoi(vlevel);
		}

		if (asl_msg_lookup(msg, ASL_KEY_HOST, &vhost, NULL) == 0)
		{
			if (vhost == NULL) vhost = "unknown";
		}

		if (asl_msg_lookup(msg, ASL_KEY_SENDER, &vsender, NULL) == 0)
		{
			if (vsender == NULL) vsender = "unknown";
		}

		asl_msg_lookup(msg, ASL_KEY_PID, &vpid, NULL);
		asl_msg_lookup(msg, ASL_KEY_MSG, &vmessage, NULL);
		asl_msg_lookup(msg, ASL_KEY_REF_PROC, &vrefproc, NULL);
		asl_msg_lookup(msg, ASL_KEY_REF_PID, &vrefpid, NULL);

		/* COMMON */
		str = asl_string_new(text_encoding);
		if (str == NULL) return NULL;

		if (vtime != NULL)
		{
			asl_string_append(str, vtime);
			free(vtime);
		}
		else
		{
			asl_string_append_char_no_encoding(str, '0');
		}

		asl_string_append_char_no_encoding(str, ' ');
		asl_string_append(str, vhost);
		asl_string_append_char_no_encoding(str, ' ');
		asl_string_append(str, vsender);

		if ((vpid != NULL) && (strcmp(vpid, "-1")))
		{
			asl_string_append_char_no_encoding(str, '[');
			asl_string_append(str, vpid);
			asl_string_append_char_no_encoding(str, ']');
		}

		if ((vrefproc != NULL) || (vrefpid != NULL)) asl_string_append_no_encoding(str, " (");

		if (vrefproc != NULL) asl_string_append(str, vrefproc);
		if (vrefpid != NULL)
		{
			asl_string_append_char_no_encoding(str, '[');
			asl_string_append(str, vrefpid);
			asl_string_append_char_no_encoding(str, ']');
		}

		if ((vrefproc != NULL) || (vrefpid != NULL)) asl_string_append_char_no_encoding(str, ')');

		if (mf == MFMT_STD)
		{
			asl_string_append_no_encoding(str, " <");
			asl_string_append(str, _asl_level_string(level));
			asl_string_append_char_no_encoding(str, '>');
		}

		asl_string_append_no_encoding(str, ": ");
		if (vmessage != NULL) asl_string_append(str, vmessage);
		asl_string_append_char_no_encoding(str, '\n');

		*len = asl_string_length(str);
		out = asl_string_release_return_bytes(str);
		return out;
	}

	if (mf == MFMT_XML)
	{
		str = asl_string_new(text_encoding);
		if (str == NULL) return NULL;

		asl_string_append_char_no_encoding(str, '\t');
		asl_string_append_no_encoding(str, "<dict>");
		asl_string_append_char_no_encoding(str, '\n');

		for (x = asl_msg_fetch(msg, 0, &key, &val, NULL); x != IndexNull; x = asl_msg_fetch(msg, x, &key, &val, NULL))
		{
			if (asl_is_utf8(key) == 1)
			{
				asl_string_append_xml_tag(str, "key", key);
				if (!strcmp(key, ASL_KEY_TIME))
				{
					vtime = _asl_time_string(tfmt, val, nano);
					if (vtime != NULL)
					{
						asl_string_append_xml_tag(str, "string", vtime);
						free(vtime);
					}
					else
					{
						asl_string_append_xml_tag(str, "string", "0");
					}
				}
				else
				{
					if (asl_is_utf8(val) == 1) asl_string_append_xml_tag(str, "string", val);
					else
					{
						b64 = asl_b64_encode((uint8_t *)val, strlen(val));
						asl_string_append_xml_tag(str, "data", (char *)b64);
						free(b64);
					}
				}
			}
		}

		asl_string_append_char_no_encoding(str, '\t');
		asl_string_append_no_encoding(str, "</dict>");
		asl_string_append_char_no_encoding(str, '\n');

		*len = asl_string_length(str);
		out = asl_string_release_return_bytes(str);
		return out;
	}

	/*
	 * Custom format
	 * The format string may contain arbitrary characters.
	 * Keys are identified by $Key or $(Key).  The value for
	 * that key is substituted.  If there are alterate formats
	 * for the value (for example a time may be formatted as
	 * raw seconds, in UTC, or a local timezone), then the
	 * key may be $((Key)(Format)).  "\$" prints a plain "$".
	 */

	str = asl_string_new(text_encoding);
	if (str == NULL) return NULL;

	/*
	 * We need enough space to copy any keys found in mfmt.
	 * The key obviously can't be longer than strlen(mfmt),
	 * in fact, keys must be shorter, since there's at least a '$'
	 * in front of the key, so we allocate a buffer with strlen(mfmt).
	 * If strlen(mfmt) <= sizeof(skey), we use skey to avoid a malloc.
	 */

	x = strlen(mfmt);
	if (x <= sizeof(skey))
	{
		k = skey;
	}
	else
	{
		k = malloc(x);
		if (k == NULL) return NULL;
	}

	cursor = 0;

	for (i = 0; mfmt[i] != '\0'; i++)
	{
		if (mfmt[i] == '$')
		{
			paren = 0;

			/* scan key, (key) or ((key)(format)) */
			for (j = i + 1; mfmt[j] != 0; j++)
			{
				if (mfmt[j] == '(')
				{
					paren++;
				}
				else if (mfmt[j] == ')')
				{
					if (paren > 0) paren--;
					if (paren == 0)
					{
						j++;
						break;
					}
				}
				else if (((mfmt[j] == ' ') || (mfmt[j] == '\t')) && (paren == 0)) break;
			}

			/* mfmt[i + 1] is the first char of the key or a '('. mfmt[j] is one char past the end. */
			l = j - (i + 1);
			memcpy(k, mfmt+i+1, l);
			k[l] = '\0';
			_asl_string_append_value_for_key_format(str, msg, k, tfmt);

			i = j - 1;
			continue;
		}

		if (mfmt[i] == '\\')
		{
			i++;
			if (mfmt[i] == '$') asl_string_append_char_no_encoding(str, '$');
			else if (mfmt[i] == 'e') asl_string_append_char_no_encoding(str, '\e');
			else if (mfmt[i] == 's') asl_string_append_char_no_encoding(str, ' ');
			else if (mfmt[i] == 'a') asl_string_append_char_no_encoding(str, '\a');
			else if (mfmt[i] == 'b') asl_string_append_char_no_encoding(str, '\b');
			else if (mfmt[i] == 'f') asl_string_append_char_no_encoding(str, '\f');
			else if (mfmt[i] == 'n') asl_string_append_char_no_encoding(str, '\n');
			else if (mfmt[i] == 'r') asl_string_append_char_no_encoding(str, '\r');
			else if (mfmt[i] == 't') asl_string_append_char_no_encoding(str, '\t');
			else if (mfmt[i] == 'v') asl_string_append_char_no_encoding(str, '\v');
			else if (mfmt[i] == '\'') asl_string_append_char_no_encoding(str, '\'');
			else if (mfmt[i] == '\\') asl_string_append_char_no_encoding(str, '\\');
			else if (isdigit(mfmt[i]))
			{
				oval = mfmt[i] - '0';
				if (isdigit(mfmt[i+1]))
				{
					i++;
					oval = (oval * 8) + (mfmt[i] - '0');
					if (isdigit(mfmt[i+1]))
					{
						i++;
						oval = (oval * 8) + (mfmt[i] - '0');
					}
				}
				c = oval;
				asl_string_append_char_no_encoding(str, c);
			}
			continue;
		}

		if (mfmt[i] == '\0') break;
		asl_string_append_char_no_encoding(str, mfmt[i]);
	}

	if (k != skey) free(k);

	asl_string_append_char_no_encoding(str, '\n');

	*len = asl_string_length(str);
	out = asl_string_release_return_bytes(str);
	return out;
}

#pragma mark -
#pragma mark xpc conversion

static void
_asl_msg_to_xpc(asl_msg_t *msg, xpc_object_t dict)
{
	uint32_t x, len;
	const char *key, *val, *nano;
	uint16_t kx;

	if (msg == NULL) return;
	if (dict == NULL) return;

	nano = NULL;
	asl_msg_lookup(msg, ASL_KEY_TIME_NSEC, &nano, NULL);

	for (x = asl_msg_fetch(msg, 0, &key, &val, NULL); x != IndexNull; x = asl_msg_fetch(msg, x, &key, &val, NULL))
	{
		if (key == NULL) continue;

		len = strlen(key);
		kx = _asl_msg_std_key(key, len);

		if (val == NULL)
		{
			xpc_object_t obj = xpc_null_create();
			xpc_dictionary_set_value(dict, key, obj);
			xpc_release(obj);
		}
		else if (kx == 0)
		{
			if (streq(key, ASL_KEY_SENDER_MACH_UUID))
			{
				uuid_t v;
				if (uuid_parse(val, v) == 0)
				{
					xpc_object_t obj = xpc_uuid_create(v);
					xpc_dictionary_set_value(dict, key, obj);
					xpc_release(obj);
				}
			}
			else
			{
				xpc_object_t obj = xpc_string_create(val);
				xpc_dictionary_set_value(dict, key, obj);
				xpc_release(obj);
			}
		}
		else if (kx == ASL_STD_KEY_TIME)
		{
			uint64_t t = NSEC_PER_SEC * asl_core_parse_time(val, NULL);
			if (nano != NULL) t += atoll(nano);
			xpc_object_t obj = xpc_date_create(t);
			xpc_dictionary_set_value(dict, key, obj);
			xpc_release(obj);
		}
		else if (kx == ASL_STD_KEY_NANO)
		{
			/* handled with ASL_STD_KEY_TIME */
		}
		else if ((kx == ASL_STD_KEY_PID) || (kx == ASL_STD_KEY_REF_PID))
		{
			int64_t v = atoll(val);
			xpc_object_t obj = xpc_int64_create(v);
			xpc_dictionary_set_value(dict, key, obj);
			xpc_release(obj);
		}
		else if ((kx == ASL_STD_KEY_UID) || (kx == ASL_STD_KEY_GID))
		{
			int64_t v = atoll(val);
			xpc_object_t obj = xpc_int64_create(v);
			xpc_dictionary_set_value(dict, key, obj);
			xpc_release(obj);
		}
		else if (kx == ASL_STD_KEY_LEVEL)
		{
			int64_t v = atoll(val);
			xpc_object_t obj = xpc_int64_create(v);
			xpc_dictionary_set_value(dict, key, obj);
			xpc_release(obj);
		}
		else if ((kx == ASL_STD_KEY_READ_UID) || (kx == ASL_STD_KEY_READ_GID))
		{
			int64_t v = atoll(val);
			xpc_object_t obj = xpc_int64_create(v);
			xpc_dictionary_set_value(dict, key, obj);
			xpc_release(obj);
		}
		else if (kx == ASL_STD_KEY_MSG_ID)
		{
			/* ignore */
		}
		else
		{
			xpc_object_t obj = xpc_string_create(val);
			xpc_dictionary_set_value(dict, key, obj);
			xpc_release(obj);
		}
	}
}

void
_asl_log_args_to_xpc(asl_object_t client, asl_object_t msg, xpc_object_t dict)
{
	_asl_msg_to_xpc(asl_client_kvdict((asl_client_t *)client), dict);
	_asl_msg_to_xpc((asl_msg_t *)msg, dict);
}

#pragma mark -
#pragma mark asl_object support

static asl_object_private_t *
_jump_alloc(uint32_t type)
{
	return (asl_object_private_t *)asl_msg_new(type);
}

static void
_jump_dealloc(asl_object_private_t *obj)
{
	asl_msg_t *msg = (asl_msg_t *)obj;
	while (msg != NULL)
	{
		asl_msg_t *next = msg->next;
		_asl_msg_free_page(msg);
		msg = next;
	}
}

static int
_jump_set_key_val_op(asl_object_private_t *obj, const char *key, const char *val, uint16_t op)
{
	uint32_t op32 = op;
	int status = asl_msg_set_key_val_op((asl_msg_t *)obj, key, val, op32);
	return (status == ASL_STATUS_OK) ? 0 : -1;
}

static void
_jump_unset_key(asl_object_private_t *obj, const char *key)
{
	asl_msg_unset((asl_msg_t *)obj, key);
}

static int
_jump_get_val_op_for_key(asl_object_private_t *obj, const char *key, const char **val, uint16_t *op)
{
	return asl_msg_lookup((asl_msg_t *)obj, key, val, op);
}

static int
_jump_get_key_val_op_at_index(asl_object_private_t *obj, size_t n, const char **key, const char **val, uint16_t *op)
{
	uint32_t slot = IndexNull;
	asl_msg_t *page = NULL;

	if  (0 != _asl_msg_resolve_index((asl_msg_t *)obj, n, &page, &slot)) return -1;

	if (key != NULL) *key = _asl_msg_slot_key(page, slot);
	if (val != NULL) *val = _asl_msg_slot_val(page, slot);
	if (op != NULL) *op = _get_slot_op(page, slot);

	return 0;
}

static size_t
_jump_count(asl_object_private_t *obj)
{
	size_t count = asl_msg_count((asl_msg_t *)obj);
	return count;
}

static void
_jump_append(asl_object_private_t *obj, asl_object_private_t *newobj, void *addr)
{
	int type = asl_get_type((asl_object_t)newobj);
	if ((type != ASL_TYPE_QUERY) && (type != ASL_TYPE_MSG)) return;

	asl_msg_merge((asl_msg_t *)obj, (asl_msg_t *)newobj);
}

static void
_jump_prepend(asl_object_private_t *obj, asl_object_private_t *newobj)
{
	if (obj == NULL) return;

	int type = asl_get_type((asl_object_t)newobj);
	if ((type != ASL_TYPE_QUERY) && (type != ASL_TYPE_MSG)) return;

	asl_msg_replace((asl_msg_t *)obj, (asl_msg_t *)newobj);
}

static asl_object_private_t *
_jump_search(asl_object_private_t *obj, asl_object_private_t *query)
{
	if (obj == NULL) return NULL;

	if (query == NULL)
	{
		/* NULL matches any message */
		asl_msg_list_t *out = asl_msg_list_new();
		asl_msg_list_append(out, obj);
		return (asl_object_private_t *)out;
	}

	if ((query->asl_type != ASL_TYPE_MSG) && (query->asl_type != ASL_TYPE_QUERY)) return NULL;

	if (asl_msg_cmp((asl_msg_t *)obj, (asl_msg_t *)query) == 1)
	{
		asl_msg_list_t *out = asl_msg_list_new();
		asl_msg_list_append(out, obj);
		return (asl_object_private_t *)out;
	}

	return NULL;
}

static asl_object_private_t *
_jump_match(asl_object_private_t *obj, asl_object_private_t *qlist, size_t *last, size_t start, size_t count, uint32_t duration, int32_t dir)
{
	if (obj == NULL) return NULL;

	if (qlist == NULL)
	{
		/* NULL matches any message */
		asl_msg_list_t *out = asl_msg_list_new();
		asl_msg_list_append(out, obj);
		return (asl_object_private_t *)out;
	}

	if (asl_msg_cmp_list((asl_msg_t *)obj, (asl_msg_list_t *)qlist) == 0) return NULL;

	asl_msg_list_t *out = asl_msg_list_new();
	asl_msg_list_append(out, obj);
	return (asl_object_private_t *)out;
}


__private_extern__ const asl_jump_table_t *
asl_msg_jump_table()
{
	static const asl_jump_table_t jump =
	{
		.alloc = &_jump_alloc,
		.dealloc = &_jump_dealloc,
		.set_key_val_op = &_jump_set_key_val_op,
		.unset_key = &_jump_unset_key,
		.get_val_op_for_key = &_jump_get_val_op_for_key,
		.get_key_val_op_at_index = &_jump_get_key_val_op_at_index,
		.count = &_jump_count,
		.next = NULL,
		.prev = NULL,
		.get_object_at_index = NULL,
		.set_iteration_index = NULL,
		.remove_object_at_index = NULL,
		.append = &_jump_append,
		.prepend = &_jump_prepend,
		.search = &_jump_search,
		.match = &_jump_match
	};

	return &jump;
}
