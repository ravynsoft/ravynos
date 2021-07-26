#include <assert.h>

/**
 * Metadata structure for an instance variable.
 *
 */
// begin: objc_ivar
struct objc_ivar
{
	/**
	 * Name of this instance variable.
	 */
	const char *name;
	/**
	 * Type encoding for this instance variable.
	 */
	const char *type;
	/**
	 * The offset from the start of the object.  When using the non-fragile
	 * ABI, this is initialized by the compiler to the offset from the start of
	 * the ivars declared by this class.  It is then set by the runtime to the
	 * offset from the object pointer.
	 */
	int        *offset;
	/**
	 * The size of this ivar.  Note that the current ABI limits ivars to 4GB.
	 */
	uint32_t    size;
	/**
	 * Flags for this instance variable.
	 */
	uint32_t    flags;
};
// end: objc_ivar

/**
 * Instance variable ownership.
 */
// begin: objc_ivar_ownership
typedef enum {
	/**
	 * Invalid.  Indicates that this is not an instance variable with ownership
	 * semantics.
	 */
	ownership_invalid = 0,
	/**
	 * Strong ownership.  Assignments to this instance variable should retain
	 * the assigned value.
	 */
	ownership_strong  = 1,
	/**
	 * Weak ownership.  This ivar is a zeroing weak reference to an object.
	 */
	ownership_weak    = 2,
	/**
	 * Object that has `__unsafe_unretained` semantics.
	 */
	ownership_unsafe  = 3
} objc_ivar_ownership;
// end: objc_ivar_ownership

/**
 * Shift for instance variable alignment. */
static const int ivar_align_shift = 3;

typedef enum {
	/**
	 * Mask applied to the flags field to indicate ownership.
	 */
	ivar_ownership_mask = (1<<0) | (1<<1),
	/**
	 * Flag indicating that the ivar contains an extended type encoding.
	 */
	ivar_extended_type_encoding = (1<<2),
	/**
	 * Mask for describing the alignment.  We need 6 bits to represent any
	 * power of two aligmnent from 0 to 63-bit alignment.  There is probably no
	 * point supporting more than 32-bit aligment, because various bits of
	 * offset assume objects are less than 4GB, but there's definitely no point
	 * in supporting 64-bit alignment because we currently don't support any
	 * architectures where an address space could contain more than one 2^64
	 * byte aligned value.
	 */
	ivar_align_mask = (((1<<6)-1) << ivar_align_shift)
} objc_ivar_flags;


static inline size_t ivarGetAlign(Ivar ivar)
{
	return 1<<((ivar->flags & ivar_align_mask) >> ivar_align_shift);
}

static inline void ivarSetAlign(Ivar ivar, size_t align)
{
	if (align != 0)
	{
		if (sizeof(size_t) == 4)
		{
			align = 4 * 8 - __builtin_clz(align) - 1;
		}
		else if (sizeof(size_t) == 8)
		{
			align = 8 * 8 - __builtin_clzll(align) - 1;
		}
		_Static_assert((sizeof(size_t) == 4) || (sizeof(size_t) == 8), "Unexpected type for size_t");
	}
	align  <<= ivar_align_shift;
	ivar->flags = (ivar->flags & ~ivar_align_mask) | align;
}

static inline void ivarSetOwnership(Ivar ivar, objc_ivar_ownership o)
{
	ivar->flags = (ivar->flags & ~ivar_ownership_mask) | o;
}


/**
 * Look up the ownership for a given instance variable.
 */
static inline objc_ivar_ownership ivarGetOwnership(Ivar ivar)
{
	return (objc_ivar_ownership)(ivar->flags & ivar_ownership_mask);
}

/**
 * Legacy ivar structure, inherited from the GCC ABI.
 */
struct objc_ivar_gcc
{
	/**
	 * Name of this instance variable.
	 */
	const char *name;
	/**
	 * Type encoding for this instance variable.
	 */
	const char *type;
	/**
	 * The offset from the start of the object.  When using the non-fragile
	 * ABI, this is initialized by the compiler to the offset from the start of
	 * the ivars declared by this class.  It is then set by the runtime to the
	 * offset from the object pointer.
	 */
	int         offset;
};


/**
 * A list of instance variables declared on this class.  Unlike the method
 * list, this is a single array and size.  Categories are not allowed to add
 * instance variables, because that would require existing objects to be
 * reallocated, which is only possible with accurate GC (i.e. not in C).
 */
// begin: objc_ivar_list
struct objc_ivar_list
{
	/**
	 * The number of instance variables in this list.
	 */
	int              count;
	/**
	 * The size of a `struct objc_ivar`.  This allows the runtime to load
	 * versions of this that come from a newer compiler, if we ever need to do
	 * so.
	 */
	size_t           size;
	/**
	 * An array of instance variable metadata structures.  Note that this array
	 * has count elements.
	 */
	struct objc_ivar ivar_list[];
};
// end: objc_ivar_list

/**
 * Returns a pointer to the ivar inside the `objc_ivar_list` structure.  This
 * structure is designed to allow the compiler to add other fields without
 * breaking the ABI, so although the `ivar_list` field appears to be an array
 * of `objc_ivar` structures, it may be an array of some future version of
 * `objc_ivar` structs, which have fields appended that this version of the
 * runtime does not know about.
 */
static inline struct objc_ivar *ivar_at_index(struct objc_ivar_list *l, int i)
{
	assert(l->size >= sizeof(struct objc_ivar));
	return (struct objc_ivar*)(((char*)l->ivar_list) + (i * l->size));
}

/**
 * Legacy version of the ivar list
 */
struct objc_ivar_list_gcc
{
	/**
	 * The number of instance variables in this list.
	 */
	int              count;
	/**
	 * An array of instance variable metadata structures.  Note that this array
	 * has count elements.
	 */
	struct objc_ivar_gcc ivar_list[];
};

