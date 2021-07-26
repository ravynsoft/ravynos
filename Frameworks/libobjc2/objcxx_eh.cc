typedef struct objc_object* id;
#include <atomic>
#include <stdlib.h>
#include <stdio.h>
#include "dwarf_eh.h"
#include "objcxx_eh.h"
#include "visibility.h"
#include "objc/runtime.h"

/**
 * Helper function that has a custom personality function.
 * This calls `cxx_throw` and has a destructor that must be run.  We intercept
 * the personality function calls and inspect the in-flight C++ exception.
 */
int eh_trampoline();

uint64_t cxx_exception_class;

extern "C" void *__cxa_allocate_exception(size_t) noexcept;

/**
 * Our own definitions of C++ ABI functions and types.  These are provided
 * because this file must not include cxxabi.h.  We need to handle subtly
 * different variations of the ABI and including one specific implementation
 * would make that very difficult.
 */
namespace __cxxabiv1
{
	/**
	 * Type info for classes.  Forward declared because the GNU ABI provides a
	 * method on all type_info objects that the dynamic the dynamic cast header
	 * needs.
	 */
	struct __class_type_info;
	/**
	 * The C++ in-flight exception object.  We will derive the offset of fields
	 * in this, so we do not ever actually see a concrete definition of it.
	 */
	struct __cxa_exception;
	/**
	 * The public ABI structure for current exception state.
	 */
	struct __cxa_eh_globals
	{
		/**
		 * The current exception that has been caught.
		 */
		__cxa_exception *caughtExceptions;
		/**
		 * The number of uncaught exceptions still in flight.
		 */
		unsigned int uncaughtExceptions;
	};
	/**
	 * Retrieve the above structure.
	 */
	extern "C" __cxa_eh_globals *__cxa_get_globals();
}

namespace std
{
	struct type_info;
}

using namespace __cxxabiv1;

// Define some C++ ABI types here, rather than including them.  This prevents
// conflicts with the libstdc++ headers, which expose only a subset of the
// type_info class (the part required for standards compliance, not the
// implementation details).

typedef void (*unexpected_handler)();
typedef void (*terminate_handler)();

namespace std
{
	/**
	 * std::type_info, containing the minimum requirements for the ABI.
	 * Public headers on some implementations also expose some implementation
	 * details.  The layout of our subclasses must respect the layout of the
	 * C++ runtime library, but also needs to be portable across multiple
	 * implementations and so should not depend on internal symbols from those
	 * libraries.
	 */
	class type_info
	{
				public:
				virtual ~type_info();
				bool operator==(const type_info &) const;
				bool operator!=(const type_info &) const;
				bool before(const type_info &) const;
				type_info();
				private:
				type_info(const type_info& rhs);
				type_info& operator= (const type_info& rhs);
				const char *__type_name;
				protected:
				type_info(const char *name): __type_name(name) { }
				public:
				const char* name() const { return __type_name; }
	};
}

namespace
{
/**
 * Helper needed by the unwind helper headers.
 */
inline _Unwind_Reason_Code continueUnwinding(struct _Unwind_Exception *ex,
                                                    struct _Unwind_Context *context)
{
#if defined(__arm__) && !defined(__ARM_DWARF_EH__)
	if (__gnu_unwind_frame(ex, context) != _URC_OK) { return _URC_FAILURE; }
#endif
	return _URC_CONTINUE_UNWIND;
}


/**
 * Flag indicating that we've already inspected a C++ exception and found all
 * of the offsets.
 */
std::atomic<bool> done_setup;
/**
 * The offset of the C++ type_info object in a thrown exception from the unwind
 * header in a `__cxa_exception`.
 */
std::atomic<ptrdiff_t> type_info_offset;
/**
 * The offset of the reference count in a
 */
std::atomic<ptrdiff_t> refcount_offset;
/**
 * The size of the `_Unwind_Exception` (including padding) in a
 * `__cxa_exception`.
 */
std::atomic<size_t> exception_struct_size;

/**
 * Helper function to find a particular value scanning backwards in a
 * structure.
 */
template<typename T>
ptrdiff_t find_backwards(void *addr, T val)
{
	T *ptr = reinterpret_cast<T*>(addr);
	for (ptrdiff_t disp = -1 ; (disp * sizeof(T) > -128) ; disp--)
	{
		if (ptr[disp] == val)
		{
			return disp * sizeof(T);
		}
	}
	fprintf(stderr, "Unable to find field in C++ exception structure\n");
	abort();
}

/**
 * Helper function to find a particular value scanning forwards in a
 * structure.
 */
template<typename T>
ptrdiff_t find_forwards(void *addr, T val)
{
	T *ptr = reinterpret_cast<T*>(addr);
	for (ptrdiff_t disp = 0 ; (disp * sizeof(T) < 256) ; disp++)
	{
		if (ptr[disp] == val)
		{
			return disp * sizeof(T);
		}
	}
	fprintf(stderr, "Unable to find field in C++ exception structure\n");
	abort();
}

template<typename T>
T *pointer_add(void *ptr, ptrdiff_t offset)
{
	return reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) + offset);
}

/**
 * Exception cleanup function for C++ exceptions that wrap Objective-C
 * exceptions.
 */
void exception_cleanup(_Unwind_Reason_Code reason,
                       struct _Unwind_Exception *ex)
{
	// __cxa_exception takes a pointer to the end of the __cxa_exception
	// structure, and so we find that by adding the size of the generic
	// exception structure + padding to the pointer to the generic exception
	// structure field of the enclosing structure.
	auto *cxxEx = pointer_add<__cxa_exception>(ex, exception_struct_size);
	__cxa_free_exception(cxxEx);
}

}

using namespace std;


static BOOL isKindOfClass(Class thrown, Class type)
{
	do
	{
		if (thrown == type)
		{
			return YES;
		}
		thrown = class_getSuperclass(thrown);
	} while (Nil != thrown);

	return NO;
}




namespace gnustep
{
	namespace libobjc
	{
		/**
		 * Superclass for the type info for Objective-C exceptions.
		 */
		struct __objc_type_info : std::type_info
		{
			/**
			 * Constructor that sets the name.
			 */
			__objc_type_info(const char *name) : type_info(name) {}
			/**
			 * Helper function used by libsupc++ and libcxxrt to determine if
			 * this is a pointer type.  If so, catches automatically
			 * dereference the pointer to the thrown pointer in
			 * `__cxa_begin_catch`.
			 */
			virtual bool __is_pointer_p() const { return true; }
			/**
			 * Helper function used by libsupc++ and libcxxrt to determine if
			 * this is a function pointer type.  Irrelevant for our purposes.
			 */
			virtual bool __is_function_p() const { return false; }
			/**
			 * Catch handler.  This is used in the C++ personality function.
			 * `thrown_type` is the type info of the thrown object, `this` is
			 * the type info at the catch site.  `thrown_object` is a pointer
			 * to a pointer to the thrown object and may be adjusted by this
			 * function.
			 */
			virtual bool __do_catch(const type_info *thrown_type,
			                        void **thrown_object,
			                        unsigned) const
			{
				assert(0);
				return false;
			};
			/**
			 * Function used for `dynamic_cast` between two C++ class types in
			 * libsupc++ and libcxxrt.
			 *
			 * This should never be called on Objective-C types.
			 */
			virtual bool __do_upcast(
			                const __class_type_info *target,
			                void **thrown_object) const
			{
				return false;
			};
		};
		/**
		 * Singleton type info for the `id` type.
		 */
		struct __objc_id_type_info : __objc_type_info
		{
			/**
			 * The `id` type is mangled to `@id`, which is not a valid mangling
			 * of anything else.
			 */
			__objc_id_type_info() : __objc_type_info("@id") {};
			virtual ~__objc_id_type_info();
			virtual bool __do_catch(const type_info *thrownType,
			                        void **obj,
			                        unsigned outer) const;
		};
		struct __objc_class_type_info : __objc_type_info
		{
			virtual ~__objc_class_type_info();
			virtual bool __do_catch(const type_info *thrownType,
			                        void **obj,
			                        unsigned outer) const;
		};
	}

	static inline id dereference_thrown_object_pointer(void** obj) {
		/* libc++-abi does not have  __is_pointer_p and won't do the double dereference 
		 * required to get the object pointer. We need to do it ourselves if we have
		 * caught an exception with libc++'s exception class. */
		 if (cxx_exception_class == llvm_cxx_exception_class) {
			 return **(id**)obj;
		 }
		 return *(id*)obj;
	 }
};


static bool AppleCompatibleMode = true;
extern "C" int objc_set_apple_compatible_objcxx_exceptions(int newValue)
{
	bool old = AppleCompatibleMode;
	AppleCompatibleMode = newValue;
	return old;
}

gnustep::libobjc::__objc_class_type_info::~__objc_class_type_info() {}
gnustep::libobjc::__objc_id_type_info::~__objc_id_type_info() {}
bool gnustep::libobjc::__objc_class_type_info::__do_catch(const type_info *thrownType,
                                                          void **obj,
                                                          unsigned outer) const
{
	id thrown = nullptr;
	bool found = false;
	// Id throw matches any ObjC catch.  This may be a silly idea!
	if (dynamic_cast<const __objc_id_type_info*>(thrownType)
	    || (AppleCompatibleMode && 
	        dynamic_cast<const __objc_class_type_info*>(thrownType)))
	{
		thrown = dereference_thrown_object_pointer(obj);
		// nil only matches id catch handlers in Apple-compatible mode, or when thrown as an id
		if (0 == thrown)
		{
			return false;
		}
		// Check whether the real thrown object matches the catch type.
		found = isKindOfClass(object_getClass(thrown),
		                      (Class)objc_getClass(name()));
	}
	else if (dynamic_cast<const __objc_class_type_info*>(thrownType))
	{
		thrown = dereference_thrown_object_pointer(obj);
		found = isKindOfClass((Class)objc_getClass(thrownType->name()),
		                      (Class)objc_getClass(name()));
	}
	if (found)
	{
		*obj = (void*)thrown;
	}
	return found;
};

bool gnustep::libobjc::__objc_id_type_info::__do_catch(const type_info *thrownType,
                                                       void **obj,
                                                       unsigned outer) const
{
	// Id catch matches any ObjC throw
	if (dynamic_cast<const __objc_class_type_info*>(thrownType))
	{
		*obj = *(id*)obj;
		return true;
	}
	if (dynamic_cast<const __objc_id_type_info*>(thrownType))
	{
		*obj = *(id*)obj;
		return true;
	}
	return false;
};

/**
 * Public interface to the Objective-C++ exception mechanism
 */
extern "C"
{
/**
 * The public symbol that the compiler uses to indicate the Objective-C id type.
 */
gnustep::libobjc::__objc_id_type_info __objc_id_type_info;

struct _Unwind_Exception *objc_init_cxx_exception(id obj)
{
	id *newEx = static_cast<id*>(__cxa_allocate_exception(sizeof(id)));
	*newEx = obj;
	_Unwind_Exception *ex = pointer_add<_Unwind_Exception>(newEx, -exception_struct_size);
	*pointer_add<std::type_info*>(ex, type_info_offset) = &__objc_id_type_info;
	ex->exception_class = cxx_exception_class;
	ex->exception_cleanup = exception_cleanup;
	__cxa_get_globals()->uncaughtExceptions++;
	return ex;
}

void* objc_object_for_cxx_exception(void *thrown_exception, int *isValid)
{
	ptrdiff_t type_offset = type_info_offset;
	if (type_offset == 0)
	{
		*isValid = 0;
		return nullptr;
	}

	const std::type_info *thrownType = 
		*pointer_add<const std::type_info*>(thrown_exception, type_offset);

	if (!dynamic_cast<const gnustep::libobjc::__objc_id_type_info*>(thrownType) && 
	    !dynamic_cast<const gnustep::libobjc::__objc_class_type_info*>(thrownType))
	{
		*isValid = 0;
		return 0;
	}
	*isValid = 1;
	return *pointer_add<id>(thrown_exception, exception_struct_size);
}

} // extern "C"


/**
 * C++ structure that is thrown through a frame with the `test_eh_personality`
 * personality function.  This contains a well-known value that we can search
 * for after the unwind header.
 */
struct
PRIVATE
MagicValueHolder
{
	/**
	 * The constant that we will search for to identify this object.
	 */
	static constexpr uint32_t magic = 0x01020304;
	/**
	 * The single field in this structure.
	 */
	uint32_t magic_value;
	/**
	 * Constructor.  Initialises the field with the magic constant.
	 */
	MagicValueHolder() { magic_value = magic; }
};


/**
 * Function that simply throws an instance of `MagicValueHolder`.
 */
PRIVATE void cxx_throw()
{
	MagicValueHolder x;
	throw x;
}

/**
 * Personality function that wraps the C++ personality and inspects the C++
 * exception structure on the way past.  This should be used only for the
 * `eh_trampoline` function.
 */
extern "C"
PRIVATE
BEGIN_PERSONALITY_FUNCTION(test_eh_personality)
	// Don't bother with a mutex here.  It doesn't matter if two threads set
	// these values at the same time.
	if (!done_setup)
	{
		uint64_t cls = __builtin_bswap64(exceptionClass);
		type_info_offset = find_backwards(exceptionObject, &typeid(MagicValueHolder));
#ifdef __LP64__
		// On 64-bit platforms, the refcount is added to the front of the
		// structure.
		ptrdiff_t refcount_backwards_offset = type_info_offset - sizeof(uintptr_t);
#else
		// On 32-bit platforms, this should be immediately before the
		// _Unwind_Exception in some spare padding, but libsupc++ puts it in
		// the same place as for 64-bit.  Try the one that's definitely in the
		// object first and then fall back to the other...
		ptrdiff_t refcount_backwards_offset = -sizeof(uint32_t);
		auto read_offset = [](void *obj, ptrdiff_t offset)
			{
				char *addr = reinterpret_cast<char*>(obj) + offset;
				uintptr_t v = *reinterpret_cast<uintptr_t*>(addr);
				return v;
			};
		if (read_offset(exceptionObject, refcount_backwards_offset) != 1)
		{
			refcount_backwards_offset = type_info_offset - sizeof(uintptr_t);
		}
		if (read_offset(exceptionObject, refcount_backwards_offset) != 1)
		{
			fprintf(stderr, "Unable to find refcount field\n");
			abort();
		}
#endif
		exception_struct_size = find_forwards(exceptionObject, MagicValueHolder::magic);
		cxx_exception_class = exceptionClass;
		done_setup = true;
	}
	return CALL_PERSONALITY_FUNCTION(__gxx_personality_v0);
}

/**
 * Probe the C++ exception handling implementation.  This throws a C++
 * exception through a function that uses `test_eh_personality` as its
 * personality function, allowing us to inspect a C++ exception that is in a
 * known state.
 */
extern "C" void test_cxx_eh_implementation()
{
	if (done_setup)
	{
		return;
	}
	bool caught = false;
	try
	{
		eh_trampoline();
	}
	catch(MagicValueHolder)
	{
		caught = true;
	}
	assert(caught);
}

