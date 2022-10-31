/* Copyright (c) 2009 Jens Ayton

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

/* NS_DECLARE_CLASS_SYMBOL()
   Helper macro to declare a name that refers to the objc_class struct for a
   given class. Example of use:
      DECLARE_CLASS_SYMBOL(NSObject);
   This declares a value _OBJC_CLASS_NSObject of type const struct objc_class.

   As an annoying complication due to linkage differences, this must be used
   in a different file than the class's @interface for Darwin, but in the
   same file on all other platforms.

   WARNING: any use of this tecnique is officially an evil hack. It is
   necessary in Foundation to implement proper CoreFoundation semantics, but
   should never be used outside the Foundation framework.

   PORTABILITY NOTE: the current non-Darwin definition may not work properly
   on platforms which don't use the convention of prefixing symbols with
   underscores, or if class symbols are for some reason handled differently.
   If using GCC or Clang, the same __asm__ approach as used for Darwin can be
   used to get the appropriate symbol.
*/
#ifdef GCC_RUNTIME_3
#define NS_CLASS_SYMBOL(className) _OBJC_CLASS_##className
#else
#define NS_CLASS_SYMBOL(className) _OBJC_CLASS_##className
#endif

#if __APPLE__ && !defined(__RAVYNOS__)
#ifdef __LP64__
#define NS_DECLARE_CLASS_SYMBOL(className) extern const struct objc_class NS_CLASS_SYMBOL(className) __asm__("_OBJC_CLASS_$_" #className)
#else
#define NS_DECLARE_CLASS_SYMBOL(className) extern const struct objc_class NS_CLASS_SYMBOL(className) __asm__(".objc_class_name_" #className)
#endif // __LP64__
#else
#if defined(__FreeBSD__) || defined(__RAVYNOS__)
#define NS_DECLARE_CLASS_SYMBOL(className) extern const struct objc_class NS_CLASS_SYMBOL(className) __asm__("._OBJC_CLASS_" #className)
#else
#define NS_DECLARE_CLASS_SYMBOL(className) extern const struct objc_class NS_CLASS_SYMBOL(className)
#endif // __FreeBSD__
#endif

/* NS_CONSTOBJ_DECL/NS_CONSTOBJ_DEF
   Appropriate linkage specifiers for constant objects.

   Constant objects must be defined in a separate linkage unit in Darwin, but
   in the same linkage unit as the class they're used by on other platforms.
*/
#if __APPLE__ && !defined(__RAVYNOS__)
#define NS_CONSTOBJ_DECL extern __attribute__((visibility("hidden")))
#define NS_CONSTOBJ_DEF
#else
#define NS_CONSTOBJ_DECL static
#define NS_CONSTOBJ_DEF static
#endif
