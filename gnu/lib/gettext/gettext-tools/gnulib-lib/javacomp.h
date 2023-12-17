/* Compile a Java program.
   Copyright (C) 2001-2002, 2006, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _JAVACOMP_H
#define _JAVACOMP_H

/* Compile a Java source file to bytecode.
   java_sources is an array of source file names.
   classpaths is a list of pathnames to be prepended to the CLASSPATH.

   source_version can be:    support for
             1.6             assert keyword (1.4), generic classes and methods (1.5)
             1.7             switch(string)
             1.8             lambdas
             9               private interface methods
            10               type inference for local variables
            11               'var' in parameters of lambda expressions
            ...
   If source-version 1.3 or 1.4 or 1.5 is requested, it gets mapped to 1.6, for
   backward compatibility. (Currently the minimum Java and javac version we need
   to support is Java 1.6, since that's the default Java version on Solaris 10.)

   target_version can be:  classfile version:
             1.6                 50.0
             1.7                 51.0
             1.8                 52.0
             9                   53.0
            10                   54.0
            11                   55.0
            ...                  ...
   If a target-version below 1.6 is requested, it gets mapped to 1.6, for
   backward compatibility. (Currently the minimum Java and javac version we need
   to support is Java 1.6, since that's the default Java version on Solaris 10.)
   target_version can also be given as NULL. In this case, the required
   target_version is determined from the found JVM (see javaversion.h).
   Specifying target_version is useful when building a library (.jar) that is
   useful outside the given package. Passing target_version = NULL is useful
   when building an application.
   It is unreasonable to ask for a target-version < source-version, such as
     - target_version < 1.4 with source_version >= 1.4, or
     - target_version < 1.5 with source_version >= 1.5, or
     - target_version < 1.6 with source_version >= 1.6, or
     - target_version < 1.7 with source_version >= 1.7, or
     - target_version < 1.8 with source_version >= 1.8, or
     - target_version < 9 with source_version >= 9, or
     - target_version < 10 with source_version >= 10, or
     - target_version < 11 with source_version >= 11, or
     - ...
   because even Sun's/Oracle's javac doesn't support these combinations.
   It is redundant to ask for a target_version > source_version, since the
   smaller target_version = source_version will also always work and newer JVMs
   support the older target_versions too.

   directory is the target directory. The .class file for class X.Y.Z is
   written at directory/X/Y/Z.class. If directory is NULL, the .class
   file is written in the source's directory.
   use_minimal_classpath = true means to ignore the user's CLASSPATH and
   use a minimal one. This is likely to reduce possible problems if the
   user's CLASSPATH contains garbage or a classes.zip file of the wrong
   Java version.
   If verbose, the command to be executed will be printed.
   Return false if OK, true on error.  */
extern bool compile_java_class (const char * const *java_sources,
                                unsigned int java_sources_count,
                                const char * const *classpaths,
                                unsigned int classpaths_count,
                                const char *source_version,
                                const char *target_version,
                                const char *directory,
                                bool optimize, bool debug,
                                bool use_minimal_classpath,
                                bool verbose);

#endif /* _JAVACOMP_H */
