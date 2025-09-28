GNUstep Objective-C Runtime
===========================

Linux and Windows CI: [![Build Status](https://dev.azure.com/gnustep/libobjc2/_apis/build/status/gnustep.libobjc2?branchName=master)](https://dev.azure.com/gnustep/libobjc2/_build/latest?definitionId=1&branchName=master)

FreeBSD CI: [![Build Status](https://api.cirrus-ci.com/github/gnustep/libobjc2.svg)](https://cirrus-ci.com/github/gnustep/libobjc2)

The GNUstep Objective-C runtime was designed as a drop-in replacement for the
GCC runtime.  It supports three ABIs:

- The old GCC ABI, which provides support for Objective-C 1.0 features.  This
  can be selected via the `-fobjc-runtime=gcc` flag in Clang or by compiling
  with GCC.
- The initial GNUstep non-fragile ABI, which was intended to be compatible with
  the GCC ABI, but provide support for modern Objective-C features.  This can be
  selected with the `-fobjc-runtime=gnustep-1.9` flag in Clang.
- The modern (v2) ABI, which provides richer reflection metadata, smaller
  binaries and reduced memory usage.  This is selected with the
  `-fobjc-runtime=gnustep-2.0` flag in Clang 7.0 or later.

The runtime can be built without support for older ABIs by setting the
`OLDABI_COMPAT` flag to `OFF` in CMake configuration.  This will result in a
smaller binary, which will not link against code using the older ABIs.

All ABIs support the following feature above and beyond the GCC runtime:

- The modern Objective-C runtime APIs, initially introduced with OS X 10.5.
- Blocks (closures).
- Synthesised property accessors.
- Efficient support for @synchronized()
- Type-dependent dispatch, eliminating stack corruption from mismatched
  selectors.
- Support for the associated reference APIs introduced with Mac OS X 10.6.
- Support for the automatic reference counting APIs introduced with Mac OS X
  10.7

History
-------

Early work on the GNUstep runtime combined code from the GCC Objective-C
runtime, the Étoilé Objective-C runtime, Remy Demarest's blocks runtime for OS
X 10.5, and the Étoilé Objective-C 2 API compatibility framework.  All of these
aside from the GCC runtime were MIT licensed, although the GPL'd code present
in the GCC runtime meant that the combined work had to remain under the GPL.

Since then, all of the GCC code has been removed, leaving the remaining files
all MIT licensed, and allowing the entire work to be MIT licensed.  

The exception handling code uses a header file implementing the generic parts
of the Itanium EH ABI.  This file comes from PathScale's libcxxrt.  PathScale
kindly allowed it to be MIT licensed for inclusion here.

Various parts of Windows support were contributed by the WinObjC team at
Microsoft.

Type-Dependent Dispatch
-----------------------

Traditionally, Objective-C method lookup is done entirely on the name of the
method.  This is problematic when the sender and receiver of the method
disagree on the types of a method.  

For example, consider a trivial case where you have two methods with the same
name, one taking an integer, the other taking a floating point value.  Both
will pass their argument in a register on most platforms, but not the same
register.  If the sender thinks it is calling one, but is really calling the
other, then the receiver will look in the wrong register and use a nonsense
value.  The compiler will often not warn about this.

This is a relatively benign example, but if the mismatch is between methods
taking or returning a structure and those only using scalar arguments and
return then the call frame layout will be so different that the result will be
stack corruption, possibly leading to security holes.

If you compile the GNUstep runtime with type-dependent dispatch enabled, then
sending a message with a typed selector will only ever invoke a method with the
same types.  Sending a message with an untyped selector will invoke any method
with a matching name, although the slot returned from the lookup function will
contain the types, allowing the caller to check them and construct a valid call
frame, if required.

If a lookup with a typed selector matches a method with the wrong types, the
runtime will call a handler.  This handler, by default, prints a helpful
message and exits.  LanguageKit provides an alternative version which
dynamically generates a new method performing the required boxing and calling
the original.

