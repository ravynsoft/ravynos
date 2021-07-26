/**
 * This file is Copyright PathScale 2010.  Permission granted to distribute
 * according to the terms of the MIT license (see COPYING.MIT)
 */
#include <assert.h>
#include <stdint.h>

// _GNU_SOURCE must be defined for unwind.h to expose some of the functions
// that we want.  If it isn't, then we define it and undefine it to make sure
// that it doesn't impact the rest of the program.
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE 1
#	include "unwind.h"
#	undef _GNU_SOURCE
#else
#	include "unwind.h"
#endif

/**
 * Type used to store pointers to values computed by DWARF expressions.
 */
typedef unsigned char *dw_eh_ptr_t;
// Flag indicating a signed quantity
#define DW_EH_PE_signed 0x08
/// DWARF data encoding types
enum dwarf_data_encoding
{
	// Unsigned, little-endian, base 128-encoded (variable length)
	DW_EH_PE_uleb128 = 0x01,
	// uint16
	DW_EH_PE_udata2  = 0x02,
	// uint32
	DW_EH_PE_udata4  = 0x03,
	// uint64
	DW_EH_PE_udata8  = 0x04,
	// Signed versions of the above:
	DW_EH_PE_sleb128 = DW_EH_PE_uleb128 | DW_EH_PE_signed,
	DW_EH_PE_sdata2  = DW_EH_PE_udata2 | DW_EH_PE_signed,
	DW_EH_PE_sdata4  = DW_EH_PE_udata4 | DW_EH_PE_signed,
	DW_EH_PE_sdata8  = DW_EH_PE_udata8 | DW_EH_PE_signed
};

static inline enum dwarf_data_encoding get_encoding(unsigned char x)
{
	return (enum dwarf_data_encoding)(x & 0xf);
}

enum dwarf_data_relative
{
	// Value is omitted
	DW_EH_PE_omit     = 0xff,
	// Absolute pointer value
	DW_EH_PE_absptr   = 0x00,
	// Value relative to program counter
	DW_EH_PE_pcrel    = 0x10,
	// Value relative to the text segment
	DW_EH_PE_textrel  = 0x20,
	// Value relative to the data segment
	DW_EH_PE_datarel  = 0x30,
	// Value relative to the start of the function
	DW_EH_PE_funcrel  = 0x40,
	// Aligned pointer (Not supported yet - are they actually used?)
	DW_EH_PE_aligned  = 0x50,
	// Pointer points to address of real value
	DW_EH_PE_indirect = 0x80
};
static inline enum dwarf_data_relative get_base(unsigned char x)
{
	return (enum dwarf_data_relative)(x & 0x70);
}
static int is_indirect(unsigned char x)
{
	return (x & DW_EH_PE_indirect);
}

static inline int dwarf_size_of_fixed_size_field(unsigned char type)
{
	// Low three bits indicate size...
	switch (type & 7)
	{
		case DW_EH_PE_udata2: return 2;
		case DW_EH_PE_udata4: return 4;
		case DW_EH_PE_udata8: return 8;
		case DW_EH_PE_absptr: return sizeof(void*);
	}
	abort();
}

/** 
 * Read an unsigned, little-endian, base-128, DWARF value.  Updates *data to
 * point to the end of the value.
 */
static uint64_t read_leb128(unsigned char** data, int *b)
{
	uint64_t uleb = 0;
	unsigned int bit = 0;
	unsigned char digit = 0;
	// We have to read at least one octet, and keep reading until we get to one
	// with the high bit unset
	do
	{
		// This check is a bit too strict - we should also check the highest
		// bit of the digit.
		assert(bit < sizeof(uint64_t) * 8);
		// Get the base 128 digit 
		digit = (**data) & 0x7f;
		// Add it to the current value
		uleb += digit << bit;
		// Increase the shift value
		bit += 7;
		// Proceed to the next octet
		(*data)++;
		// Terminate when we reach a value that does not have the high bit set
		// (i.e. which was not modified when we mask it with 0x7f)
	} while ((*(*data - 1)) != digit);
	*b = bit;

	return uleb;
}

static int64_t read_uleb128(unsigned char** data)
{
	int b;
	return read_leb128(data, &b);
}


static int64_t read_sleb128(unsigned char** data)
{
	int bits;
	// Read as if it's signed
	uint64_t uleb = read_leb128(data, &bits);
	// If the most significant bit read is 1, then we need to sign extend it
	if (uleb >> (bits-1) == 1)
	{
		// Sign extend by setting all bits in front of it to 1
		uleb |= ((int64_t)-1) << bits;
	}
	return (int64_t)uleb;
}

static uint64_t read_value(char encoding, unsigned char **data)
{
	enum dwarf_data_encoding type = get_encoding(encoding);
	uint64_t v;
	switch ((int)type)
	{
		// Read fixed-length types
#define READ(dwarf, type) \
		case dwarf:\
			v = (uint64_t)(*(type*)(*data));\
			*data += sizeof(type);\
			break;
		READ(DW_EH_PE_udata2, uint16_t)
		READ(DW_EH_PE_udata4, uint32_t)
		READ(DW_EH_PE_udata8, uint64_t)
		READ(DW_EH_PE_sdata2, int16_t)
		READ(DW_EH_PE_sdata4, int32_t)
		READ(DW_EH_PE_sdata8, int64_t)
		case DW_EH_PE_absptr:
			v = (uint64_t)(*(intptr_t*)(*data));
			*data += sizeof(intptr_t);
			break;
		//READ(DW_EH_PE_absptr, intptr_t)
#undef READ
		case DW_EH_PE_sleb128:
			v = read_sleb128(data);
			break;
		case DW_EH_PE_uleb128:
			v = read_uleb128(data);
			break;
		default: abort();
	}

	return v;
}

static uint64_t resolve_indirect_value(struct _Unwind_Context *c, unsigned char encoding, int64_t v, dw_eh_ptr_t start)
{
	switch (get_base(encoding))
	{
		case DW_EH_PE_pcrel:
			v += (uint64_t)(uintptr_t)start;
			break;
		case DW_EH_PE_textrel:
			v += (uint64_t)(uintptr_t)_Unwind_GetTextRelBase(c);
			break;
		case DW_EH_PE_datarel:
			v += (uint64_t)(uintptr_t)_Unwind_GetDataRelBase(c);
			break;
		case DW_EH_PE_funcrel:
			v += (uint64_t)(uintptr_t)_Unwind_GetRegionStart(c);
		default:
			break;
	}
	// If this is an indirect value, then it is really the address of the real
	// value
	// TODO: Check whether this should really always be a pointer - it seems to
	// be a GCC extensions, so not properly documented...
	if (is_indirect(encoding))
	{
		v = (uint64_t)(uintptr_t)*(void**)(uintptr_t)v;
	}
	return v;
}


static inline void read_value_with_encoding(struct _Unwind_Context *context,
                                            dw_eh_ptr_t *data,
                                            uint64_t *out)
{
	dw_eh_ptr_t start = *data;
	unsigned char encoding = *((*data)++);
	// If this value is omitted, skip it and don't touch the output value
	if (encoding == DW_EH_PE_omit) { return; }

	*out = read_value(encoding, data);
	*out = resolve_indirect_value(context, encoding, *out, start);
}


struct dwarf_eh_lsda
{
	dw_eh_ptr_t region_start;
	dw_eh_ptr_t landing_pads;
	dw_eh_ptr_t type_table;
	unsigned char type_table_encoding;
	dw_eh_ptr_t call_site_table;
	dw_eh_ptr_t action_table;
	unsigned char callsite_encoding;
};

static inline struct dwarf_eh_lsda parse_lsda(struct _Unwind_Context *context, unsigned char *data)
{
	struct dwarf_eh_lsda lsda;

	lsda.region_start = (dw_eh_ptr_t)(uintptr_t)_Unwind_GetRegionStart(context);

	// If the landing pads are relative to anything other than the start of
	// this region, find out where.  This is @LPStart in the spec, although the
	// encoding that GCC uses does not quite match the spec.
	uint64_t v = (uint64_t)(uintptr_t)lsda.region_start;
	read_value_with_encoding(context, &data, &v);
	lsda.landing_pads = (dw_eh_ptr_t)(uintptr_t)v;

	// If there is a type table, find out where it is.  This is @TTBase in the
	// spec.  Note: we find whether there is a type table pointer by checking
	// whether the leading byte is DW_EH_PE_omit (0xff), which is not what the
	// spec says, but does seem to be how G++ indicates this.
	lsda.type_table = 0;
	lsda.type_table_encoding = *data++;
	if (lsda.type_table_encoding != DW_EH_PE_omit)
	{
		v = read_uleb128(&data);
		dw_eh_ptr_t type_table = data;
		type_table += v;
		lsda.type_table = type_table;
		//lsda.type_table = (uintptr_t*)(data + v);
	}

#if defined(__arm__) && !defined(__ARM_DWARF_EH__)
	lsda.type_table_encoding = (DW_EH_PE_pcrel | DW_EH_PE_indirect);
#endif

	lsda.callsite_encoding = (enum dwarf_data_encoding)(*(data++));

	// Action table is immediately after the call site table
	lsda.action_table = data;
	uintptr_t callsite_size = (uintptr_t)read_uleb128(&data);
	lsda.action_table = data + callsite_size;
	// Call site table is immediately after the header
	lsda.call_site_table = (dw_eh_ptr_t)data;


	return lsda;
}

struct dwarf_eh_action
{
	dw_eh_ptr_t landing_pad;
	dw_eh_ptr_t action_record;
};

/**
 * Look up the landing pad that corresponds to the current invoke.
 */
__attribute__((unused))
static struct dwarf_eh_action 
	dwarf_eh_find_callsite(struct _Unwind_Context *context, struct dwarf_eh_lsda *lsda)
{
	struct dwarf_eh_action result = { 0, 0 };
	uint64_t ip = _Unwind_GetIP(context) - _Unwind_GetRegionStart(context);
	unsigned char *callsite_table = (unsigned char*)lsda->call_site_table;
	while (callsite_table <= lsda->action_table)
	{
		// Once again, the layout deviates from the spec.
		uint64_t call_site_start, call_site_size, landing_pad, action;
		call_site_start = read_value(lsda->callsite_encoding, &callsite_table);
		call_site_size = read_value(lsda->callsite_encoding, &callsite_table);

		// Call site entries are started
		if (call_site_start > ip) { break; }

		landing_pad = read_value(lsda->callsite_encoding, &callsite_table);
		action = read_uleb128(&callsite_table);

		if (call_site_start <= ip && ip <= call_site_start + call_site_size)
		{
			if (action)
			{
				// Action records are 1-biased so both no-record and zeroth
				// record can be stored.
				result.action_record = lsda->action_table + action - 1;
			}
			// No landing pad means keep unwinding.
			if (landing_pad)
			{
				// Landing pad is the offset from the value in the header
				result.landing_pad = lsda->landing_pads + landing_pad;
			}
			break;
		}
	}
	return result;
}

#define EXCEPTION_CLASS(a,b,c,d,e,f,g,h) ((((uint64_t)a) << 56) + (((uint64_t)b) << 48) + (((uint64_t)c) << 40) + (((uint64_t)d) << 32) + (((uint64_t)e) << 24) + (((uint64_t)f) << 16) + (((uint64_t)g) << 8) + (((uint64_t)h)))
