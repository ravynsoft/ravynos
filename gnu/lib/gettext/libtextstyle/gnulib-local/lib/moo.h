/* Minimal object-oriented facilities for C.
   Copyright (C) 2006 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* This file defines minimal facilities for object-oriented programming
   style in ANSI C.

   The facilities allow to define classes with single inheritance and
   "virtual" methods.

   Strict type checking is provided in combination with a C++ compiler:
   The code compiles in ANSI C with less strict type checking; when
   compiled with a C++ compiler, strict type checking is done.

   In contrast to [OOC] and [OOPC], this implementation concentrates on the
   bare essentials of an object-oriented programming style.  It does not
   provide features that are "sometimes useful", but not essential.

   Features:
     - Combination of fields and methods into a single object.      YES
     - Description of objects of same shape and same behaviour
       by a class.                                                  YES
     - Single inheritance.                                          YES
     - Multiple inheritance.                                        NO
     - Operator overloading (compile-time polymorphism).            NO
     - Virtual methods (run-time polymorphism).                     YES
     - Information hiding: private/protected/public.         private fields
     - Static fields and methods.                                   NO
     - Constructors, destructors.                                   NO
     - 'new', 'delete'.                                             NO
     - Exception handling.                                          NO
     - Garbage collection.                                          NO
     - Templates / Generic classes with parameters.                 NO
     - Namespaces.                                                  NO
     - Hidden 'this' pointer in methods.                            NO
     - Declaring or implementing several classes in the same file.  NO

   Rationale for NO:
     - Multiple inheritance is not supported because programming languages
       like Java and C# prove that they are not necessary. Modern design
       patterns use delegation more often than composition; this reduces
       the pressure to use multiple inheritance.
       Multiple inheritance of "interfaces" (classes without fields) might
       be considered, though.
     - Operator overloading is not essential: The programmer can rename
       methods so that they carry unambiguous method names. This also makes
       the code more readable.
     - Virtual methods are supported. Non-virtual methods are not: they
       constitute an assumption about the possible subclasses which is more
       often wrong than right. In other words, non-virtual methods are a
       premature optimization - "the root of all evil", according to
       Donald E. Knuth.
     - Information hiding: 'protected' is not supported because it is always
       inappropriate: it prohibits the use of the delegation design pattern.
       'private' is implemented on fields. There are no 'public' fields,
       since the use of getters/setters allows for overriding in subclasses
       and is more maintainable (ability to set breakpoints). On the other
       hand, all methods are 'public'. 'private` methods are not supported
       because methods with static linkage can be used instead.
     - Static fields and methods are not supported because normal variables
       and functions with static or extern linkage can be used instead.
     - Constructors and destructors are not supported.  The programmer can
       define 'init' and 'do_free' methods himself.
     - 'new', 'delete' are not supported because they only provide the
       grouping of two lines of code into a single line of code.
     - Exception handling is not supported because conventions with a return
       code can be used instead.
     - Garbage collection is not supported. Without it the programmer's life
       is harder, but not impossible. The programmer has to think about
       ownership of objects.
     - Templates / Generic classes with parameters are not supported because
       they are mostly used for container classes, and container classes can
       be implemented in a simpler object-oriented way that requires only a
       very limited form of class inheritance.
     - Namespaces are not implemented, because they can be simulated by a
       consistent naming convention.
     - A hidden 'this' pointer in methods is not implemented. It reduces the
       transparency of the code (because what looks like a variable access can
       be an access through 'this') and is simply not needed.
     - Declaring or implementing several classes in the same file is not
       supported, because it is anyway good practice to define each class in
       its own .oo.h / .oo.c file.

   Syntax:

   The syntax resembles C++, but deviates from C++ where the C++ syntax is
   just too braindead.

   A root class is declared in a .oo.h file:

     struct rootfoo
     {
     methods:
       int method1 (rootfoo_t x, ...); ...
     };

   and in the corresponding .oo.c file:

     struct rootfoo
     {
     fields:
       int field1; ...
     };

   A subclass is declared in a .oo.h file as well:

     struct subclass : struct rootfoo
     {
     methods:
       int method2 (subclass_t x, ...); ...
     };

   and in the corresponding .oo.c file:

     struct subclass : struct rootfoo
     {
     fields:
       int field2; ...
     };

   This defines:
     - An incomplete type 'struct any_rootfoo_representation' or
       'struct subclass_representation', respectively. It denotes the memory
       occupied by an object of the respective class. The prefix 'any_' is
       present only for a root class.
     - A type 'rootfoo_t' or 'subclass_t' that is equivalent to a pointer
       'struct any_rootfoo_representation *' or
       'struct subclass_representation *', respectively.
     - A type 'struct rootfoo_implementation' or
       'struct subclass_implementation', respectively. It contains a virtual
       function table for the corresponding type.
     - A type 'struct rootfoo_representation_header' or
       'struct subclass_representation_header', respectively, that defines
       the part of the memory representation containing the virtual function
       table pointer.
     - Functions 'rootfoo_method1 (rootfoo_t x, ...);' ...
                 'subclass_method1 (subclass_t x, ...);' ...
                 'subclass_method2 (subclass_t x, ...);' ...
       that invoke the corresponding methods. They are realized as inline
       functions if possible.
     - A declaration of 'rootfoo_typeinfo' or 'subclass_typeinfo', respectively,
       each being a typeinfo_t instance.
     - A declaration of 'ROOTFOO_SUPERCLASSES' or 'SUBCLASS_SUPERCLASSES',
       respectively, each being an initializer for an array of typeinfo_t.
     - A declaration of 'ROOTFOO_SUPERCLASSES_LENGTH' or
       'SUBCLASS_SUPERCLASSES_LENGTH', respectively, each denoting the length
       of that initializer.
     - A declaration of 'rootfoo_vtable' or 'subclass_vtable', respectively,
       being an instance of 'struct rootfoo_implementation' or
       'struct subclass_implementation', respectively.
     - A header file "rootfoo.priv.h" or "subclass.priv.h" that defines the
       private fields of 'struct rootfoo_representation' or
       'struct subclass_representation', respectively.

   A class implementation looks like this, in a .oo.c file:

     struct subclass : struct rootfoo
     {
     fields:
       int field2; ...
     };

     int subclass::method1 (subclass_t x, ...) { ... } [optional]
     int subclass::method2 (subclass_t x, ...) { ... }
     ...

   At the place of the second "struct subclass" definition, the type
   'struct subclass_representation' is expanded, and the macro 'super' is
   defined, referring to the vtable of the superclass. For root classes,
   'super' is not defined. Also, 'subclass_typeinfo' is defined.

   Each method subclass::method_i defines the implementation of a method
   for the particular class. Its C name is subclass__method_i (not to be
   confused with subclass_method_i, which is the externally visible function
   that invokes this method).

   Methods that are not defined implicitly inherited from the superclass.

   At the end of the file, 'subclass_vtable' is defined, as well as
     'subclass_method1 (subclass_t x, ...);' ...
     'subclass_method2 (subclass_t x, ...);' ...
   if they were not already defined as inline functions in the header file.

   Object representation in memory:
     - Objects have as their first field, called 'vtable', a pointer to a table
       to data and function pointers that depend only on the class, not on the
       object instance.
     - One of the first fields of the vtable is a pointer to the
       'superclasses'; this is a NULL-terminated array of pointers to
       typeinfo_t objects, starting with the class itself, then its
       superclass etc.


   [OOC] Axel-Tobias Schreiner: Object-oriented programming with ANSI-C. 1993.

   [OOPC] Laurent Deniau: Object Oriented Programming in C. 2001.

 */

#ifndef _MOO_H
#define _MOO_H

/* Get size_t, abort().  */
#include <stdlib.h>

/* An object of this type is defined for each class.  */
typedef struct
{
  const char *classname;
} typeinfo_t;

/* IS_INSTANCE (OBJ, ROOTCLASSNAME, CLASSNAME)
   tests whether an object is instance of a given class, given as lower case
   class name.  */
#define IS_INSTANCE(obj,rootclassname,classname) \
  (((const struct rootclassname##_representation_header *)(const struct any_##rootclassname##_representation *)(obj))->vtable->superclasses_length \
   >= classname##_SUPERCLASSES_LENGTH \
   && ((const struct rootclassname##_representation_header *)(const struct any_##rootclassname##_representation *)(obj))->vtable->superclasses \
      [((const struct rootclassname##_representation_header *)(const struct any_##rootclassname##_representation *)(obj))->vtable->superclasses_length \
       - classname##_SUPERCLASSES_LENGTH] \
      == & classname##_typeinfo)
/* This instance test consists of two comparisons.  One could even optimize
   this to a single comparison, by limiting the inheritance depth to a fixed
   limit, for example, say, depth <= 10.  The superclasses list would then
   need to be stored in reverse order, from the root down to the class itself,
   and be filled up with NULLs so that the array has length 10.  The instance
   test would look like this:
     #define IS_INSTANCE(obj,rootclassname,classname) \
       (((const struct rootclassname##_representation_header *)(const struct any_##rootclassname##_representation *)(obj))->vtable->superclasses \
        [classname##_SUPERCLASSES_LENGTH - 1] \
        == & classname##_typeinfo)
   but the classname##_superclasses_length would no longer be available as a
   simple sizeof expression.  */

#endif /* _MOO_H */
