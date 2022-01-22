/**
 * Defines the module structures.
 *
 * When defining a new ABI, the 
 */

/**
 * The symbol table for a module.  This structure references all of the
 * Objective-C symbols defined for a module, allowing the runtime to find and
 * register them.
 */
struct objc_symbol_table_abi_8
{
	/**
	 * The number of selectors referenced in this module.
	 */
	unsigned long  selector_count;
	/**
	 * An array of selectors used in this compilation unit.  SEL is a pointer
	 * type and this points to the first element in an array of selectors.
	 */
	SEL            selectors;
	/**
	 * The number of classes defined in this module.
	 */
	unsigned short class_count;
	/**
	 * The number of categories defined in this module.
	 */
	unsigned short category_count;
	/**
	 * A null-terminated array of pointers to symbols defined in this module.
	 * This contains class_count pointers to class structures, category_count
	 * pointers to category structures, and then zero or more pointers to
	 * static object instances.
	 *
	 * Current compilers only use this for constant strings.  The runtime
	 * permits other types.
	 */
	void           *definitions[];
};

/**
 * The module structure is passed to the __objc_exec_class function by a
 * constructor function when the module is loaded.  
 *
 * When defining a new ABI version, the first two fields in this structure must
 * be retained.
 */
struct objc_module_abi_8
{
	/**
	 * The version of the ABI used by this module.  This is checked against the
	 * list of ABIs that the runtime supports, and the list of incompatible
	 * ABIs.
	 */
	unsigned long                   version;
	/**
	 * The size of the module.  This is used for sanity checking, to ensure
	 * that the compiler and runtime's idea of the module size match.
	 */
	unsigned long                   size;
	/**
	 * The full path name of the source for this module.  Not currently used
	 * for anything, could be used for debugging in theory, but duplicates
	 * information available from DWARF data, so probably won't.
	 */
	const char                     *name;
	/**
	 * A pointer to the symbol table for this compilation unit.
	 */
	struct objc_symbol_table_abi_8 *symbol_table;
};

struct objc_module_abi_10
{
	/**
	 * Inherited fields from version 8 of the ABI.
	 */
	struct objc_module_abi_8 old;
	/**
	 * GC mode.  GC_Optional code can be mixed with anything, but GC_None code
	 * can't be mixed with GC_Required code.
	 */
	int gc_mode;
};

/**
 * List of static instances of a named class provided in this module.
 */
struct objc_static_instance_list
{
	/**
	 * The name of the class.  The isa pointer of all of the instances will be
	 * set to the class with this name.
	 */
	char *class_name;
	/**
	 * NULL-terminated array of statically-allocated instances.
	 */
	id    instances[];
};
