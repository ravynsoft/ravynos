/*
 * Copyright (c) 2011-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "internal.h"

#include <libkern/OSByteOrder.h>

#if defined(__LITTLE_ENDIAN__)
#define DISPATCH_DATA_FORMAT_TYPE_UTF16_HOST DISPATCH_DATA_FORMAT_TYPE_UTF16LE
#define DISPATCH_DATA_FORMAT_TYPE_UTF16_REV DISPATCH_DATA_FORMAT_TYPE_UTF16BE
#elif defined(__BIG_ENDIAN__)
#define DISPATCH_DATA_FORMAT_TYPE_UTF16_HOST DISPATCH_DATA_FORMAT_TYPE_UTF16BE
#define DISPATCH_DATA_FORMAT_TYPE_UTF16_REV DISPATCH_DATA_FORMAT_TYPE_UTF16LE
#endif

enum {
	_DISPATCH_DATA_FORMAT_NONE = 0x1,
	_DISPATCH_DATA_FORMAT_UTF8 = 0x2,
	_DISPATCH_DATA_FORMAT_UTF16LE = 0x4,
	_DISPATCH_DATA_FORMAT_UTF16BE = 0x8,
	_DISPATCH_DATA_FORMAT_UTF_ANY = 0x10,
	_DISPATCH_DATA_FORMAT_BASE32 = 0x20,
	_DISPATCH_DATA_FORMAT_BASE32HEX = 0x40,
	_DISPATCH_DATA_FORMAT_BASE64 = 0x80,
};

#pragma mark -
#pragma mark baseXX tables

static const unsigned char base32_encode_table[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static const char base32_decode_table[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26,
	27, 28, 29, 30, 31, -1, -1, -1, -1, -1, -2, -1, -1, -1,  0,  1,  2,
	 3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25
};
static const ssize_t base32_decode_table_size = sizeof(base32_decode_table)
		/ sizeof(*base32_decode_table);

static const unsigned char base32hex_encode_table[] =
		"0123456789ABCDEFGHIJKLMNOPQRSTUV";

static const char base32hex_decode_table[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  1,  2,
	 3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -2, -1, -1, -1, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31
};
static const ssize_t base32hex_decode_table_size =
		sizeof(base32hex_encode_table) / sizeof(*base32hex_encode_table);

static const unsigned char base64_encode_table[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char base64_decode_table[] = {
	-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	-1,  62,  -1,  -1,  -1,  63,  52,  53,  54,  55,  56,  57,  58,  59,
	60,  61,  -1,  -1,  -1,  -2,  -1,  -1,  -1,   0,   1,   2,   3,   4,
	 5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
	19,  20,  21,  22,  23,  24,  25,  -1,  -1,  -1,  -1,  -1,  -1,  26,
	27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
	41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51
};

static const ssize_t base64_decode_table_size = sizeof(base64_decode_table)
		/ sizeof(*base64_decode_table);

#pragma mark -
#pragma mark dispatch_transform_buffer

typedef struct dispatch_transform_buffer_s {
	dispatch_data_t data;
	uint8_t *start;
	union {
		uint8_t *u8;
		uint16_t *u16;
	} ptr;
	size_t size;
} dispatch_transform_buffer_s;

static size_t
_dispatch_transform_sizet_mul(size_t a, size_t b)
{
	size_t rv = SIZE_MAX;
	if (a == 0 || rv/a >= b) {
		rv = a * b;
	}
	return rv;
}

#define BUFFER_MALLOC_MAX (100*1024*1024)

static bool
_dispatch_transform_buffer_new(dispatch_transform_buffer_s *buffer,
		size_t required, size_t size)
{
	size_t remaining = buffer->size - (size_t)(buffer->ptr.u8 - buffer->start);
	if (required == 0 || remaining < required) {
		if (buffer->start) {
			if (buffer->ptr.u8 > buffer->start) {
				dispatch_data_t _new = dispatch_data_create(buffer->start,
						(size_t)(buffer->ptr.u8 - buffer->start), NULL,
						DISPATCH_DATA_DESTRUCTOR_FREE);
				dispatch_data_t _concat = dispatch_data_create_concat(
						buffer->data, _new);
				dispatch_release(_new);
				dispatch_release(buffer->data);
				buffer->data = _concat;
			} else {
				free(buffer->start);
			}
		}
		buffer->size = required + size;
		buffer->start = NULL;
		if (buffer->size > 0) {
			if (buffer->size > BUFFER_MALLOC_MAX) {
				return false;
			}
			buffer->start = (uint8_t*)malloc(buffer->size);
			if (buffer->start == NULL) {
				return false;
			}
		}
		buffer->ptr.u8 = buffer->start;
	}
	return true;
}

#pragma mark -
#pragma mark dispatch_transform_helpers

static dispatch_data_t
_dispatch_data_subrange_map(dispatch_data_t data, const void **ptr,
		size_t offset, size_t size)
{
	dispatch_data_t subrange, map = NULL;

	subrange = dispatch_data_create_subrange(data, offset, size);
	if (dispatch_data_get_size(subrange) == size) {
		map = dispatch_data_create_map(subrange, ptr, NULL);
	}
	dispatch_release(subrange);
	return map;
}

static dispatch_data_format_type_t
_dispatch_transform_detect_utf(dispatch_data_t data)
{
	const void *p;
	dispatch_data_t subrange = _dispatch_data_subrange_map(data, &p, 0, 2);

	if (subrange == NULL) {
		return NULL;
	}

	const uint16_t ch = *(const uint16_t *)p;
	dispatch_data_format_type_t type = DISPATCH_DATA_FORMAT_TYPE_UTF8;

	if (ch == 0xfeff) {
		type = DISPATCH_DATA_FORMAT_TYPE_UTF16_HOST;
	} else if (ch == 0xfffe) {
		type = DISPATCH_DATA_FORMAT_TYPE_UTF16_REV;
	}

	dispatch_release(subrange);

	return type;
}

static uint16_t
_dispatch_transform_swap_to_host(uint16_t x, int32_t byteOrder)
{
	if (byteOrder == OSLittleEndian) {
		return OSSwapLittleToHostInt16(x);
	}
	return OSSwapBigToHostInt16(x);
}

static uint16_t
_dispatch_transform_swap_from_host(uint16_t x, int32_t byteOrder)
{
	if (byteOrder == OSLittleEndian) {
		return OSSwapHostToLittleInt16(x);
	}
	return OSSwapHostToBigInt16(x);
}

#pragma mark -
#pragma mark UTF-8

static uint8_t
_dispatch_transform_utf8_length(uint8_t byte)
{
	if ((byte & 0x80) == 0) {
		return 1;
	} else if ((byte & 0xe0) == 0xc0) {
		return 2;
	} else if ((byte & 0xf0) == 0xe0) {
		return 3;
	} else if ((byte & 0xf8) == 0xf0) {
		return 4;
	}
	return 0;
}

static uint32_t
_dispatch_transform_read_utf8_sequence(const uint8_t *bytes)
{
	uint32_t wch = 0;
	uint8_t seq_length = _dispatch_transform_utf8_length(*bytes);

	switch (seq_length) {
	case 4:
		wch |= (*bytes & 0x7);
		wch <<= 6;
		break;
	case 3:
		wch |= (*bytes & 0xf);
		wch <<= 6;
		break;
	case 2:
		wch |= (*bytes & 0x1f);
		wch <<= 6;
		break;
	case 1:
		wch = (*bytes & 0x7f);
		break;
	default:
		// Not a utf-8 sequence
		break;
	}

	bytes++;
	seq_length--;

	while (seq_length > 0) {
		wch |= (*bytes & 0x3f);
		bytes++;
		seq_length--;

		if (seq_length > 0) {
			wch <<= 6;
		}
	}
	return wch;
}

#pragma mark -
#pragma mark UTF-16

static dispatch_data_t
_dispatch_transform_to_utf16(dispatch_data_t data, int32_t byteOrder)
{
	__block size_t skip = 0;

	__block dispatch_transform_buffer_s buffer = {
		.data = dispatch_data_empty,
	};

	bool success = dispatch_data_apply(data, ^(
			DISPATCH_UNUSED dispatch_data_t region,
			size_t offset, const void *_buffer, size_t size) {
		const uint8_t *src = _buffer;
		size_t i;

		if (offset == 0) {
			size_t dest_size = 2 + _dispatch_transform_sizet_mul(size,
					sizeof(uint16_t));
			if (!_dispatch_transform_buffer_new(&buffer, dest_size, 0)) {
				return (bool)false;
			}
			// Insert BOM
			*(buffer.ptr.u16)++ = _dispatch_transform_swap_from_host(0xfeff,
					byteOrder);
		}

		// Skip is incremented if the previous block read-ahead into our block
		if (skip >= size) {
			skip -= size;
			return (bool)true;
		} else if (skip > 0) {
			src += skip;
			size -= skip;
			skip = 0;
		}

		for (i = 0; i < size;) {
			uint32_t wch = 0;
			uint8_t byte_size = _dispatch_transform_utf8_length(*src);

			if (byte_size == 0) {
				return (bool)false;
			} else if (byte_size + i > size) {
				// UTF-8 byte sequence spans over into the next block(s)
				const void *p;
				dispatch_data_t subrange = _dispatch_data_subrange_map(data, &p,
						offset + i, byte_size);
				if (subrange == NULL) {
					return (bool)false;
				}

				wch = _dispatch_transform_read_utf8_sequence(p);
				skip += byte_size - (size - i);
				src += byte_size;
				i = size;

				dispatch_release(subrange);
			} else {
				wch = _dispatch_transform_read_utf8_sequence(src);
				src += byte_size;
				i += byte_size;
			}

			size_t next = _dispatch_transform_sizet_mul(size - i, sizeof(uint16_t));
			if (wch >= 0xd800 && wch < 0xdfff) {
				// Illegal range (surrogate pair)
				return (bool)false;
			} else if (wch >= 0x10000) {
				// Surrogate pair
				if (!_dispatch_transform_buffer_new(&buffer, 2 *
						sizeof(uint16_t), next)) {
					return (bool)false;
				}
				wch -= 0x10000;
				*(buffer.ptr.u16)++ = _dispatch_transform_swap_from_host(
						((wch >> 10) & 0x3ff) + 0xd800, byteOrder);
				*(buffer.ptr.u16)++ = _dispatch_transform_swap_from_host(
						(wch & 0x3ff) + 0xdc00, byteOrder);
			} else {
				if (!_dispatch_transform_buffer_new(&buffer, 1 *
						sizeof(uint16_t), next)) {
					return (bool)false;
				}
				*(buffer.ptr.u16)++ = _dispatch_transform_swap_from_host(
						(wch & 0xffff), byteOrder);
			}
		}

		(void)_dispatch_transform_buffer_new(&buffer, 0, 0);

		return (bool)true;
	});

	if (!success) {
		(void)_dispatch_transform_buffer_new(&buffer, 0, 0);
		dispatch_release(buffer.data);
		return NULL;
	}

	return buffer.data;
}

static dispatch_data_t
_dispatch_transform_from_utf16(dispatch_data_t data, int32_t byteOrder)
{
	__block size_t skip = 0;

	__block dispatch_transform_buffer_s buffer = {
		.data = dispatch_data_empty,
	};

	bool success = dispatch_data_apply(data, ^(
			DISPATCH_UNUSED dispatch_data_t region, size_t offset,
			const void *_buffer, size_t size) {
		const uint16_t *src = _buffer;

		if (offset == 0) {
			// Assume first buffer will be mostly single-byte UTF-8 sequences
			size_t dest_size = _dispatch_transform_sizet_mul(size, 2) / 3;
			if (!_dispatch_transform_buffer_new(&buffer, dest_size, 0)) {
				return (bool)false;
			}
		}

		size_t i = 0, max = size / 2;

		// Skip is incremented if the previous block read-ahead into our block
		if (skip >= size) {
			skip -= size;
			return (bool)true;
		} else if (skip > 0) {
			src = (uint16_t *)(((uint16_t *)src) + skip/2);
			size -= skip;
			max = (size / 2);
			skip = 0;
		}

		// If the buffer is an odd size, allow read ahead into the next region
		if ((size % 2) != 0) {
			max += 1;
		}

		for (i = 0; i < max; i++) {
			uint32_t wch = 0;
			uint16_t ch;

			if ((i == (max - 1)) && (max > (size / 2))) {
				// Last byte of an odd sized range
				const void *p;
				dispatch_data_t range = _dispatch_data_subrange_map(data, &p,
						offset + (i * 2), 2);
				if (range == NULL) {
					return (bool)false;
				}
				ch = _dispatch_transform_swap_to_host((uint16_t)*(uint64_t*)p,
						byteOrder);
				dispatch_release(range);
				skip += 1;
			} else {
				ch =  _dispatch_transform_swap_to_host(src[i], byteOrder);
			}

			if (ch == 0xfffe && offset == 0 && i == 0) {
				// Wrong-endian BOM at beginning of data
				return (bool)false;
			} else if (ch == 0xfeff && offset == 0 && i == 0) {
				// Correct-endian BOM, skip it
				continue;
			}

			if ((ch >= 0xd800) && (ch <= 0xdbff)) {
				// Surrogate pair
				wch = ((ch - 0xd800u) << 10);
				if (++i >= max) {
					// Surrogate byte isn't in this block
					const void *p;
					dispatch_data_t range = _dispatch_data_subrange_map(data,
							&p, offset + (i * 2), 2);
					if (range == NULL) {
						return (bool)false;
					}
					ch = _dispatch_transform_swap_to_host(*(uint16_t *)p,
							byteOrder);
					dispatch_release(range);
					skip += 2;
				} else {
					ch = _dispatch_transform_swap_to_host(src[i], byteOrder);
				}
				if (!((ch >= 0xdc00) && (ch <= 0xdfff))) {
					return (bool)false;
				}
				wch = (wch | (ch & 0x3ff));
				wch += 0x10000;
			} else if ((ch >= 0xdc00) && (ch <= 0xdfff)) {
				return (bool)false;
			} else {
				wch = ch;
			}

			size_t next = _dispatch_transform_sizet_mul(max - i, 2);
			if (wch < 0x80) {
				if (!_dispatch_transform_buffer_new(&buffer, 1, next)) {
					return (bool)false;
				}
				*(buffer.ptr.u8)++ = (uint8_t)(wch & 0xff);
			} else if (wch < 0x800) {
				if (!_dispatch_transform_buffer_new(&buffer, 2, next)) {
					return (bool)false;
				}
				*(buffer.ptr.u8)++ = (uint8_t)(0xc0 | (wch >> 6));
				*(buffer.ptr.u8)++ = (uint8_t)(0x80 | (wch & 0x3f));
			} else if (wch < 0x10000) {
				if (!_dispatch_transform_buffer_new(&buffer, 3, next)) {
					return (bool)false;
				}
				*(buffer.ptr.u8)++ = (uint8_t)(0xe0 | (wch >> 12));
				*(buffer.ptr.u8)++ = (uint8_t)(0x80 | ((wch >> 6) & 0x3f));
				*(buffer.ptr.u8)++ = (uint8_t)(0x80 | (wch & 0x3f));
			} else if (wch < 0x200000) {
				if (!_dispatch_transform_buffer_new(&buffer, 4, next)) {
					return (bool)false;
				}
				*(buffer.ptr.u8)++ = (uint8_t)(0xf0 | (wch >> 18));
				*(buffer.ptr.u8)++ = (uint8_t)(0x80 | ((wch >> 12) & 0x3f));
				*(buffer.ptr.u8)++ = (uint8_t)(0x80 | ((wch >> 6) & 0x3f));
				*(buffer.ptr.u8)++ = (uint8_t)(0x80 | (wch & 0x3f));
			}
		}

		(void)_dispatch_transform_buffer_new(&buffer, 0, 0);

		return (bool)true;
	});

	if (!success) {
		(void)_dispatch_transform_buffer_new(&buffer, 0, 0);
		dispatch_release(buffer.data);
		return NULL;
	}

	return buffer.data;
}

static dispatch_data_t
_dispatch_transform_from_utf16le(dispatch_data_t data)
{
	return _dispatch_transform_from_utf16(data, OSLittleEndian);
}

static dispatch_data_t
_dispatch_transform_from_utf16be(dispatch_data_t data)
{
	return _dispatch_transform_from_utf16(data, OSBigEndian);
}

static dispatch_data_t
_dispatch_transform_to_utf16le(dispatch_data_t data)
{
	return _dispatch_transform_to_utf16(data, OSLittleEndian);
}

static dispatch_data_t
_dispatch_transform_to_utf16be(dispatch_data_t data)
{
	return _dispatch_transform_to_utf16(data, OSBigEndian);
}

#pragma mark -
#pragma mark base32

static dispatch_data_t
_dispatch_transform_from_base32_with_table(dispatch_data_t data,
		const char* table, ssize_t table_size)
{
	__block uint64_t x = 0, count = 0, pad = 0;

	__block dispatch_data_t rv = dispatch_data_empty;

	bool success = dispatch_data_apply(data, ^(
			DISPATCH_UNUSED dispatch_data_t region,
			DISPATCH_UNUSED size_t offset, const void *buffer, size_t size) {
		size_t i, dest_size = (size * 5) / 8;

		uint8_t *dest = (uint8_t*)malloc(dest_size * sizeof(uint8_t));
		uint8_t *ptr = dest;
		if (dest == NULL) {
			return (bool)false;
		}

		const uint8_t *bytes = buffer;

		for (i = 0; i < size; i++) {
			if (bytes[i] == '\n' || bytes[i] == '\t' || bytes[i] == ' ') {
				continue;
			}

			ssize_t index = bytes[i];
			if (index >= table_size || table[index] == -1) {
				free(dest);
				return (bool)false;
			}
			count++;

			char value = table[index];
			if (value == -2) {
				value = 0;
				pad++;
			}

			x <<= 5;
			x += (uint64_t)value;

			if ((count & 0x7) == 0) {
				*ptr++ = (x >> 32) & 0xff;
				*ptr++ = (x >> 24) & 0xff;
				*ptr++ = (x >> 16) & 0xff;
				*ptr++ = (x >> 8) & 0xff;
				*ptr++ = x & 0xff;
			}
		}

		size_t final = (size_t)(ptr - dest);
		switch (pad) {
		case 1:
			final -= 1;
			break;
		case 3:
			final -= 2;
			break;
		case 4:
			final -= 3;
			break;
		case 6:
			final -= 4;
			break;
		}

		dispatch_data_t val = dispatch_data_create(dest, final, NULL,
				DISPATCH_DATA_DESTRUCTOR_FREE);
		dispatch_data_t concat = dispatch_data_create_concat(rv, val);

		dispatch_release(val);
		dispatch_release(rv);
		rv = concat;

		return (bool)true;
	});

	if (!success) {
		dispatch_release(rv);
		return NULL;
	}

	return rv;
}

static dispatch_data_t
_dispatch_transform_to_base32_with_table(dispatch_data_t data, const unsigned char* table)
{
	size_t total = dispatch_data_get_size(data);
	__block size_t count = 0;

	if (total > SIZE_T_MAX-4 || ((total+4)/5 > SIZE_T_MAX/8)) {
		/* We can't hold larger than size_t in a dispatch_data_t
		 * and we want to avoid an integer overflow in the next
		 * calculation.
		 */
		return NULL;
	}

	size_t dest_size = (total + 4) / 5 * 8;
	uint8_t *dest = (uint8_t*)malloc(dest_size);
	if (dest == NULL) {
		return NULL;
	}

	__block uint8_t *ptr = dest;

	/*
					    0        1        2        3        4
	 8-bit bytes:   xxxxxxxx yyyyyyyy zzzzzzzz xxxxxxxx yyyyyyyy
	 5-bit chunks:  aaaaabbb bbcccccd ddddeeee efffffgg ggghhhhh
	 */

	bool success = dispatch_data_apply(data, ^(
			DISPATCH_UNUSED dispatch_data_t region,
			size_t offset, const void *buffer, size_t size) {
		const uint8_t *bytes = buffer;
		size_t i;

		for (i = 0; i < size; i++, count++) {
			uint8_t curr = bytes[i], last = 0;

			if ((count % 5) != 0) {
				if (i == 0) {
					const void *p;
					dispatch_data_t subrange = _dispatch_data_subrange_map(data,
							&p, offset - 1, 1);
					if (subrange == NULL) {
						return (bool)false;
					}
					last = *(uint8_t*)p;
					dispatch_release(subrange);
				} else {
					last = bytes[i - 1];
				}
			}

			switch (count % 5) {
			case 0:
				// a
				*ptr++ = table[(curr >> 3) & 0x1fu];
				break;
			case 1:
				// b + c
				*ptr++ = table[((last << 2)|(curr >> 6)) & 0x1f];
				*ptr++ = table[(curr >> 1) & 0x1f];
				break;
			case 2:
				// d
				*ptr++ = table[((last << 4)|(curr >> 4)) & 0x1f];
				break;
			case 3:
				// e + f
				*ptr++ = table[((last << 1)|(curr >> 7)) & 0x1f];
				*ptr++ = table[(curr >> 2) & 0x1f];
				break;
			case 4:
				// g + h
				*ptr++ = table[((last << 3)|(curr >> 5)) & 0x1f];
				*ptr++ = table[curr & 0x1f];
				break;
			}
		}

		// Last region, insert padding bytes, if needed
		if (offset + size == total) {
			switch (count % 5) {
			case 0:
				break;
			case 1:
				// b[4:2]
				*ptr++ = table[(bytes[size-1] << 2) & 0x1c];
				break;
			case 2:
				// d[4]
				*ptr++ = table[(bytes[size-1] << 4) & 0x10];
				break;
			case 3:
				// e[4:1]
				*ptr++ = table[(bytes[size-1] << 1) & 0x1e];
				break;
			case 4:
				// g[2:3]
				*ptr++ = table[(bytes[size-1] << 3) & 0x18];
				break;
			}
			switch (count % 5) {
			case 0:
				break;
			case 1:
				*ptr++ = '='; // c
				*ptr++ = '='; // d
			case 2:
				*ptr++ = '='; // e
			case 3:
				*ptr++ = '='; // f
				*ptr++ = '='; // g
			case 4:
				*ptr++ = '='; // h
				break;
			}
		}

		return (bool)true;
	});

	if (!success) {
		free(dest);
		return NULL;
	}
	return dispatch_data_create(dest, dest_size, NULL,
			DISPATCH_DATA_DESTRUCTOR_FREE);
}

static dispatch_data_t
_dispatch_transform_from_base32(dispatch_data_t data)
{
	return _dispatch_transform_from_base32_with_table(data, base32_decode_table,
			base32_decode_table_size);
}

static dispatch_data_t
_dispatch_transform_to_base32(dispatch_data_t data)
{
	return _dispatch_transform_to_base32_with_table(data, base32_encode_table);
}

static dispatch_data_t
_dispatch_transform_from_base32hex(dispatch_data_t data)
{
	return _dispatch_transform_from_base32_with_table(data,
			base32hex_decode_table, base32hex_decode_table_size);
}

static dispatch_data_t
_dispatch_transform_to_base32hex(dispatch_data_t data)
{
	return _dispatch_transform_to_base32_with_table(data,
			base32hex_encode_table);
}

#pragma mark -
#pragma mark base64

static dispatch_data_t
_dispatch_transform_from_base64(dispatch_data_t data)
{
	__block uint64_t x = 0, count = 0;
	__block size_t pad = 0;

	__block dispatch_data_t rv = dispatch_data_empty;

	bool success = dispatch_data_apply(data, ^(
			DISPATCH_UNUSED dispatch_data_t region,
			DISPATCH_UNUSED size_t offset, const void *buffer, size_t size) {
		size_t i, dest_size = (size * 3) / 4;

		uint8_t *dest = (uint8_t*)malloc(dest_size * sizeof(uint8_t));
		uint8_t *ptr = dest;
		if (dest == NULL) {
			return (bool)false;
		}

		const uint8_t *bytes = buffer;

		for (i = 0; i < size; i++) {
			if (bytes[i] == '\n' || bytes[i] == '\t' || bytes[i] == ' ') {
				continue;
			}

			ssize_t index = bytes[i];
			if (index >= base64_decode_table_size ||
					base64_decode_table[index] == -1) {
				free(dest);
				return (bool)false;
			}
			count++;

			char value = base64_decode_table[index];
			if (value == -2) {
				value = 0;
				pad++;
			}

			x <<= 6;
			x += (uint64_t)value;

			if ((count & 0x3) == 0) {
				*ptr++ = (x >> 16) & 0xff;
				*ptr++ = (x >> 8) & 0xff;
				*ptr++ = x & 0xff;
			}
		}

		size_t final = (size_t)(ptr - dest);
		if (pad > 0) {
			// 2 bytes of pad means only had one char in final group
			final -= pad;
		}

		dispatch_data_t val = dispatch_data_create(dest, final, NULL,
				DISPATCH_DATA_DESTRUCTOR_FREE);
		dispatch_data_t concat = dispatch_data_create_concat(rv, val);

		dispatch_release(val);
		dispatch_release(rv);
		rv = concat;

		return (bool)true;
	});

	if (!success) {
		dispatch_release(rv);
		return NULL;
	}

	return rv;
}

static dispatch_data_t
_dispatch_transform_to_base64(dispatch_data_t data)
{
	// RFC 4648 states that we should not linebreak
	// http://tools.ietf.org/html/rfc4648
	size_t total = dispatch_data_get_size(data);
	__block size_t count = 0;

	if (total > SIZE_T_MAX-2 || ((total+2)/3> SIZE_T_MAX/4)) {
		/* We can't hold larger than size_t in a dispatch_data_t
		 * and we want to avoid an integer overflow in the next
		 * calculation.
		 */
		return NULL;
	}

	size_t dest_size = (total + 2) / 3 * 4;
	uint8_t *dest = (uint8_t*)malloc(dest_size);
	if (dest == NULL) {
		return NULL;
	}

	__block uint8_t *ptr = dest;

	/*
	 * 3 8-bit bytes:	xxxxxxxx yyyyyyyy zzzzzzzz
	 * 4 6-bit chunks:	aaaaaabb bbbbcccc ccdddddd
	 */

	bool success = dispatch_data_apply(data, ^(
			DISPATCH_UNUSED dispatch_data_t region,
			size_t offset, const void *buffer, size_t size) {
		const uint8_t *bytes = buffer;
		size_t i;

		for (i = 0; i < size; i++, count++) {
			uint8_t curr = bytes[i], last = 0;

			if ((count % 3) != 0) {
				if (i == 0) {
					const void *p;
					dispatch_data_t subrange = _dispatch_data_subrange_map(data,
						&p, offset - 1, 1);
					if (subrange == NULL) {
						return (bool)false;
					}
					last = *(uint8_t*)p;
					dispatch_release(subrange);
				} else {
					last = bytes[i - 1];
				}
			}

			switch (count % 3) {
			case 0:
				*ptr++ = base64_encode_table[(curr >> 2) & 0x3f];
				break;
			case 1:
				*ptr++ = base64_encode_table[((last << 4)|(curr >> 4)) & 0x3f];
				break;
			case 2:
				*ptr++ = base64_encode_table[((last << 2)|(curr >> 6)) & 0x3f];
				*ptr++ = base64_encode_table[(curr & 0x3f)];
				break;
			}
		}

		// Last region, insert padding bytes, if needed
		if (offset + size == total) {
			switch (count % 3) {
			case 0:
				break;
			case 1:
				*ptr++ = base64_encode_table[(bytes[size-1] << 4) & 0x30];
				*ptr++ = '=';
				*ptr++ = '=';
				break;
			case 2:
				*ptr++ = base64_encode_table[(bytes[size-1] << 2) & 0x3c];
				*ptr++ = '=';
				break;
			}
		}

		return (bool)true;
	});

	if (!success) {
		free(dest);
		return NULL;
	}
	return dispatch_data_create(dest, dest_size, NULL,
			DISPATCH_DATA_DESTRUCTOR_FREE);
}

#pragma mark -
#pragma mark dispatch_data_transform

dispatch_data_t
dispatch_data_create_with_transform(dispatch_data_t data,
		dispatch_data_format_type_t input, dispatch_data_format_type_t output)
{
	if (input->type == _DISPATCH_DATA_FORMAT_UTF_ANY) {
		input = _dispatch_transform_detect_utf(data);
		if (input == NULL) {
			return NULL;
		}
	}

	if ((input->type & ~output->input_mask) != 0) {
		return NULL;
	}

	if ((output->type & ~input->output_mask) != 0) {
		return NULL;
	}

	if (dispatch_data_get_size(data) == 0) {
		return data;
	}

	dispatch_data_t temp1;
	if (input->decode) {
		temp1 = input->decode(data);
	} else {
		dispatch_retain(data);
		temp1 = data;
	}

	if (!temp1) {
		return NULL;
	}

	dispatch_data_t temp2;
	if (output->encode) {
		temp2 = output->encode(temp1);
	} else {
		dispatch_retain(temp1);
		temp2 = temp1;
	}

	dispatch_release(temp1);
	return temp2;
}

const struct dispatch_data_format_type_s _dispatch_data_format_type_none = {
	.type = _DISPATCH_DATA_FORMAT_NONE,
	.input_mask = ~0u,
	.output_mask = ~0u,
	.decode = NULL,
	.encode = NULL,
};

const struct dispatch_data_format_type_s _dispatch_data_format_type_base32 = {
	.type = _DISPATCH_DATA_FORMAT_BASE32,
	.input_mask = (_DISPATCH_DATA_FORMAT_NONE | _DISPATCH_DATA_FORMAT_BASE32 |
			_DISPATCH_DATA_FORMAT_BASE32HEX | _DISPATCH_DATA_FORMAT_BASE64),
	.output_mask = (_DISPATCH_DATA_FORMAT_NONE | _DISPATCH_DATA_FORMAT_BASE32 |
			_DISPATCH_DATA_FORMAT_BASE32HEX | _DISPATCH_DATA_FORMAT_BASE64),
	.decode = _dispatch_transform_from_base32,
	.encode = _dispatch_transform_to_base32,
};

const struct dispatch_data_format_type_s _dispatch_data_format_type_base32hex =
{
	.type = _DISPATCH_DATA_FORMAT_BASE32HEX,
	.input_mask = (_DISPATCH_DATA_FORMAT_NONE | _DISPATCH_DATA_FORMAT_BASE32 |
			_DISPATCH_DATA_FORMAT_BASE32HEX | _DISPATCH_DATA_FORMAT_BASE64),
	.output_mask = (_DISPATCH_DATA_FORMAT_NONE | _DISPATCH_DATA_FORMAT_BASE32 |
			_DISPATCH_DATA_FORMAT_BASE32HEX | _DISPATCH_DATA_FORMAT_BASE64),
	.decode = _dispatch_transform_from_base32hex,
	.encode = _dispatch_transform_to_base32hex,
};

const struct dispatch_data_format_type_s _dispatch_data_format_type_base64 = {
	.type = _DISPATCH_DATA_FORMAT_BASE64,
	.input_mask = (_DISPATCH_DATA_FORMAT_NONE | _DISPATCH_DATA_FORMAT_BASE32 |
			_DISPATCH_DATA_FORMAT_BASE32HEX | _DISPATCH_DATA_FORMAT_BASE64),
	.output_mask = (_DISPATCH_DATA_FORMAT_NONE | _DISPATCH_DATA_FORMAT_BASE32 |
			_DISPATCH_DATA_FORMAT_BASE32HEX | _DISPATCH_DATA_FORMAT_BASE64),
	.decode = _dispatch_transform_from_base64,
	.encode = _dispatch_transform_to_base64,
};

const struct dispatch_data_format_type_s _dispatch_data_format_type_utf16le = {
	.type = _DISPATCH_DATA_FORMAT_UTF16LE,
	.input_mask = (_DISPATCH_DATA_FORMAT_UTF8 | _DISPATCH_DATA_FORMAT_UTF16BE |
			_DISPATCH_DATA_FORMAT_UTF16LE),
	.output_mask = (_DISPATCH_DATA_FORMAT_UTF8 | _DISPATCH_DATA_FORMAT_UTF16BE |
			_DISPATCH_DATA_FORMAT_UTF16LE),
	.decode = _dispatch_transform_from_utf16le,
	.encode = _dispatch_transform_to_utf16le,
};

const struct dispatch_data_format_type_s _dispatch_data_format_type_utf16be = {
	.type = _DISPATCH_DATA_FORMAT_UTF16BE,
	.input_mask = (_DISPATCH_DATA_FORMAT_UTF8 | _DISPATCH_DATA_FORMAT_UTF16BE |
			_DISPATCH_DATA_FORMAT_UTF16LE),
	.output_mask = (_DISPATCH_DATA_FORMAT_UTF8 | _DISPATCH_DATA_FORMAT_UTF16BE |
			_DISPATCH_DATA_FORMAT_UTF16LE),
	.decode = _dispatch_transform_from_utf16be,
	.encode = _dispatch_transform_to_utf16be,
};

const struct dispatch_data_format_type_s _dispatch_data_format_type_utf8 = {
	.type = _DISPATCH_DATA_FORMAT_UTF8,
	.input_mask = (_DISPATCH_DATA_FORMAT_UTF8 | _DISPATCH_DATA_FORMAT_UTF16BE |
			_DISPATCH_DATA_FORMAT_UTF16LE),
	.output_mask = (_DISPATCH_DATA_FORMAT_UTF8 | _DISPATCH_DATA_FORMAT_UTF16BE |
			_DISPATCH_DATA_FORMAT_UTF16LE),
	.decode = NULL,
	.encode = NULL,
};

const struct dispatch_data_format_type_s _dispatch_data_format_type_utf_any = {
	.type = _DISPATCH_DATA_FORMAT_UTF_ANY,
	.input_mask = 0,
	.output_mask = 0,
	.decode = NULL,
	.encode = NULL,
};
