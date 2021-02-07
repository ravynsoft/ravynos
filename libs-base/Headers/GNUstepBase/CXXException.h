
struct _Unwind_Exception;
/**
 * CXXException class implements a boxed C++ exception.  This class should
 * never be created directly by the user.  
 *
 * When using libobjc2, you can catch C++ exceptions by declaring this class as
 * a catch parameter, for example:
 *
 * @catch(CXXException *e)
 *
 * The caught instance will be autoreleased and does not need to be manually
 * freed.  
 */
GS_EXPORT_CLASS
@interface CXXException : NSObject
{
  /** Exception object, as defined by the CodeSourcery exception ABI. */
  struct _Unwind_Exception *ex;
}
/**
 * Constructor called by the runtime. 
 */
+ (id) exceptionWithForeignException: (struct _Unwind_Exception*)ex;
/**
 * Returns a pointer to the thrown value.  When a value is thrown in C++, it is
 * copied into the exception structure.  The real type of the pointee is the
 * type of the object that was thrown.  
 */
- (void*) thrownValue;
/**
 * Returns a pointer to a std::type_info (C++) object defining the type of the
 * value pointed to by the return from -thrownValue.  You may compare this with
 * the result of the typeinfo() operator in Objective-C++ to determine the type
 * of the object.
 */
- (void*) cxx_type_info;
/**
 * Rethrows the exception.  Sending messages to this object after is has been
 * rethrown has undefined behaviour.
 */
- (void) rethrow;
@end
