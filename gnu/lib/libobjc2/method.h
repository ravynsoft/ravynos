#include <assert.h>

/**
 * Metadata structure describing a method.  
 */
// begin: objc_method
struct objc_method
{
	/**
	 * A pointer to the function implementing this method.
	 */
	IMP         imp;
	/**
	 * Selector used to send messages to this method.
	 */
	SEL         selector;
	/**
	 * The extended type encoding for this method.
	 */
	const char *types;
};
// end: objc_method

struct objc_method_gcc
{
	/**
	 * Selector used to send messages to this method.  The type encoding of
	 * this method should match the types field.
	 */
	SEL         selector;
	/**
	 * The type encoding for this selector.  Used only for introspection, and
	 * only required because of the stupid selector handling in the old GNU
	 * runtime.  In future, this field may be reused for something else.
	 */
	const char *types;
	/**
	 * A pointer to the function implementing this method.
	 */
	IMP         imp;
};

/**
 * Method list.  Each class or category defines a new one of these and they are
 * all chained together in a linked list, with new ones inserted at the head.
 * When constructing the dispatch table, methods in the start of the list are
 * used in preference to ones at the end.
 */
// begin: objc_method_list
struct objc_method_list
{
	/**
	 * The next group of methods in the list.
	 */
	struct objc_method_list  *next;
	/**
	 * The number of methods in this list.
	 */
	int                       count;
	/**
	 * Sze of `struct objc_method`.  This allows runtimes downgrading newer
	 * versions of this structure.
	 */
	size_t                    size;
	/**
	 * An array of methods.  Note that the actual size of this is count.
	 */
	struct objc_method        methods[];
};
// end: objc_method_list

/**
 * Returns a pointer to the method inside the `objc_method` structure.  This
 * structure is designed to allow the compiler to add other fields without
 * breaking the ABI, so although the `methods` field appears to be an array
 * of `objc_method` structures, it may be an array of some future version of
 * `objc_method` structs, which have fields appended that this version of the
 * runtime does not know about.
 */
static inline struct objc_method *method_at_index(struct objc_method_list *l, int i)
{
	assert(l->size >= sizeof(struct objc_method));
	return (struct objc_method*)(((char*)l->methods) + (i * l->size));
}

/**
 * Legacy version of the method list.
 */
struct objc_method_list_gcc
{
	/**
	 * The next group of methods in the list.
	 */
	struct objc_method_list_gcc *next;
	/**
	 * The number of methods in this list.
	 */
	int                       count;
	/**
	 * An array of methods.  Note that the actual size of this is count.
	 */
	struct objc_method_gcc methods[];
};
