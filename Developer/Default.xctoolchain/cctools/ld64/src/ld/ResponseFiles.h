//
//  ResponseFiles.hpp
//  ld
//
//  Created by Michael Trent on 3/3/20.
//  Copyright © 2020 Apple Inc. All rights reserved.
//

#ifndef ResponseFiles_hpp
#define ResponseFiles_hpp

/*
 * ExpandResponseFiles() recursively expands "@file" options as they appear in the
 * argc/argv options list.
 *
 * if "file" does not point to a valid file, the option "@file" will remain
 * in the options list.
 *
 * if "file" does point to a valid file, that file will be parsed as a series
 * of options separated by any amount of whitespace, and those options will
 * replace "@file" where it appears in the options list. options files may
 * themselves contain additional "@file" references, which will be recursively
 * expanded. to prevent infinite recursion, ExpandResponseFiles() will fail if it
 * attempts to load a "@file" path more than once.
 *
 * options are separated by any amount of whitespace. whitespace can be included
 * in an option if it is wrapped in single or double quotes. individual
 * characters can also be escaped with a single backslash character; including
 * whitespace, quotes, and other backslashes.
 *
 * NB: ExpandResponseFiles() allows options to include quotes in the middle of the
 * string; e.g., "one' 'option" will expand to "one option" rather than "one"
 * and "option". This is consistent with unix shell behavior, but not consistent
 * with some other implementations of the @file command-line option.

 * BUG: ExpandResponseFiles() will not return an error if an option contains an
 * unterminated quote character. The string "'one more time" will yield a single
 * option "'one more time".
 *
 * BUG: memory pointed to by argv_p is not freed if the pointer is replaced.
 * Also, it is not clear how the argv array and its contents should be freed
 * if the array is modified.
 */
int ExpandResponseFiles(int* argc_p, char** argv_p[]);

#endif /* ResponseFiles_hpp */
