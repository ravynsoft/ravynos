//
//  args.c
//  stuff
//
//  Created by Michael Trent on 5/31/19.
//
#ifdef __linux__
#include <string.h>
#include <time.h>
#endif

#include "stuff/args.h"
#include "stuff/errors.h"
#include "stuff/port.h" /* cctools-port: reallocf() */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> /* cctools-port */
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

enum expand_result {
  EXPAND_ERROR = -1,
  EXPAND_COMPLETE = 0,
  EXPAND_CONTINUE = 1,
};

/*
 * struct string_list is a poor-man's alternative to std::vector<string>,
 * "managing" a list of strings. a zeroed structure represents a valid empty
 * string list; i.e., "struct string_list strings = {0};". if non-NULL, the
 * strs member must point to malloced, reallocable memory; each string in the
 * strs array must also be individually malloced. the strs array is expected
 * to be only large enough to hold nstr string pointers, it is not generally
 * null terminated.
 *
 * it should become its own libstuff module if it has utility in other placees.
 */
struct string_list {
  int nstr;
  char** strs;
};

static enum expand_result expand_at(struct string_list *args,
struct string_list* at_paths, int *hint_p);

static char* get_option(char** buf);

static void string_list_add(struct string_list* list, const char* str);
static void string_list_add_argv(struct string_list* list, int argc,
char** argv);
static int string_list_find(const struct string_list* list, const char* str);
static void string_list_dest(struct string_list* list);

/*
 * args_expand_at() recursively expands "@file" options as they appear in the
 * argc/argv options list.
 */
int args_expand_at(int* argc_p, char** argv_p[])
{
  int hint = 0;
  enum expand_result result = EXPAND_CONTINUE;
  struct string_list at_paths = {0};
  struct string_list args = {0};

  if (!argc_p || !argv_p) {
    errno = EINVAL;
    return -1;
  }

  // copy the arguments into a private structure
  string_list_add_argv(&args, *argc_p, *argv_p);

  // "recursively" expand at files
  while (EXPAND_CONTINUE == result) {
    result = expand_at(&args, &at_paths, &hint);
  }

  // destroy the at_paths strings
  string_list_dest(&at_paths);

  // return the modified values, adding a NULL terminator to the string list
  if (result == EXPAND_COMPLETE) {
    args.strs = reallocf(args.strs, sizeof(char*) * (args.nstr + 1));
    if (!args.strs)
      system_fatal("reallocf failed");
    args.strs[args.nstr] = NULL;
    
    *argc_p = args.nstr;
    *argv_p = args.strs;
  }

  return result == EXPAND_COMPLETE ? 0 : -1;
}

/*
 * expand_at is the worker function that expands "@file" options as they
 * appear in the argv array. it's designed to be called interatively, so that
 * we can provide recursive "@file" references without blowing out the stack
 * or imposing an arbitrary maximum.
 *
 * args is the argc/argv options list expressed in a string_list structure. the
 * contents of the struct may be modified if arguments need to be inserted into
 * the options list. expand_at requires args to be a proper string_list so it
 * can resize or clean up memory as necessary.
 *
 * expand_at will record the name of @files it encounters during the expansion
 * process so that it can return an error on infinitely-recursive input. callers
 * should providee memory to an empty struct string_list via at_paths to support
 * this feature, and then destroy the string_list contents when the expansion
 * process has completed. alternatively, callers can set at_paths to NULL to
 * disable the infinite recursion check.
 *
 * similarly, callers an provide memory for an int via hint_p across multiple
 * calls to expand_at. the initial value of *hint_p must be 0. expand_at() will
 * use this value to avoid re-examining elements in the option list that have
 * already been fully expanded. this optimization can be disabled by passing
 * NULL to hint_p.
 *
 * expand_at will return one of three states:
 *
 *   EXPAND_CONTINUE - expand_at() has modified the options list and additional
 *                     expansion appears to be necessary. callers should re-
 *                     invoke expand_at() with the same set of arguments.
 *   EXPAND_COMPLETE - expand_at() has examined the options list and no further
 *                     expansion is necessary. expand_at() may or may not have
 *                     modified the args string list. at this point, callers
 *                     are free to examine the contents of args and tear down
 *                     related data structures.
 *   EXPAND_ERROR    - an error was encountered during the expansion process.
 *                     an error message was printed to stderr, and callers can
 *                     examine errno if they like.
 *
 * usage is typically in a while loop, such as:
 *
 *   // "recursively" expand at files
 *   enum expand_result result = EXPAND_CONTINUE;
 *   while (EXPAND_CONTINUE == result) {
 *     result = expand_at(&args, &at_paths, &hint);
 *   }
 */
enum expand_result expand_at(struct string_list *args,
struct string_list* at_paths, int *hint_p)
{
  int argc = args->nstr;
  char** argv = args->strs;
  int hint = hint_p ? *hint_p : 0;
  struct string_list newargs = {0};
  enum expand_result result = EXPAND_COMPLETE;

  for (int i = hint; i < argc; ++i) {
    if ('@' == argv[i][0]) {
      char* at_path = &(argv[i][1]);

      // error if we have seen this path before.
      if (at_paths && -1 != string_list_find(at_paths, at_path)) {
        fprintf(stderr, "error: recursively loading %s\n", at_path);
        return EXPAND_ERROR;
      }

      // open the file at this path. If the file does not exist, treat the
      // entry like a literal string and continue.
      int fd = open(at_path, O_RDONLY);
      if (-1 == fd) {
        if (ENOENT == errno) {
          // awkward. add this option if necessary.
          if (newargs.nstr) {
            string_list_add(&newargs, argv[i]);
          }
          continue;
        }
        fprintf(stderr, "error: can't open %s: %s\n", at_path, strerror(errno));
        return EXPAND_ERROR;
      }

      // remember we have opened this file previously
      if (at_paths && -1 == string_list_find(at_paths, at_path)) {
        string_list_add(at_paths, at_path);
      }

      // attempt to map the file into memory. if the file is empty, we will
      // simply treat this as an empty buffer.
      struct stat sb;
      if (fstat(fd, &sb)) {
        fprintf(stderr, "error: can't stat %s: %s\n", at_path, strerror(errno));
        close(fd);
        return EXPAND_ERROR;
      }

      char* addr = NULL;
      if (sb.st_size) {
        addr = mmap(0, sb.st_size, PROT_READ | PROT_WRITE,
                    MAP_FILE | MAP_PRIVATE, fd, 0);
        if (!addr) {
          fprintf(stderr, "error: can't mmap %s: %s\n", at_path,
                  strerror(errno));
          close(fd);
          return EXPAND_ERROR;
        }
      }

      if (close(fd)) {
        fprintf(stderr, "error: can't close %s: %s\n", at_path,
                strerror(errno));
        if (munmap(addr, sb.st_size))
          fprintf(stderr, "error: can't munmap %s: %s\n", at_path,
                  strerror(errno));
        return EXPAND_ERROR;
      }

      // build a new argument list now
      if (0 == newargs.nstr) {
        string_list_add_argv(&newargs, i, args->strs);
        *hint_p = i;
      }

      // copy the strings in from the at file. If we see another at symbol
      // set result to EXPAND_CONTINUE to request additional expansion.
      if (addr) {
        char* p = addr;
        for (char* arg = get_option(&p); arg; arg = get_option(&p)) {
          string_list_add(&newargs, arg);
          if ('@' == arg[0])
            result = EXPAND_CONTINUE;
        }
      }

      // unmap the file
      if (addr) {
        if (munmap(addr, sb.st_size)) {
          fprintf(stderr, "error: can't munmap %s: %s\n", at_path,
                  strerror(errno));
          return EXPAND_ERROR;
        }
      }
    }
    else { // if ('@' != argv[i][0])
      // add this literal option if necessary.
      if (newargs.nstr) {
        string_list_add(&newargs, argv[i]);
      }
    }
  }

  if (newargs.nstr) {
    string_list_dest(args);
    args->nstr = newargs.nstr;
    args->strs = newargs.strs;
  }

  return result;
}

/*
 * get_option() tokenizes a string of command-line options separated by
 * whitespace. given a pointer to a string, get_option() will return a pointer
 * to the first word in that string and adjust the pointer to point to the
 * remainder of the string. this promotes usage in a simple loop:
 *
 *   if (string) {
 *     char* p = string;
 *     for (char* arg = get_option(&p); arg; arg = get_option(&p)) {
 *       // do something
 *     }
 *   }
 *
 * the string, buf, provides all of the storage necessary for tokenization;
 * both the contents of buf as well as the value of *buf will be modified by
 * get_option().
 *
 * get_option() honors characters escaped by \ or wrapped in single or double
 * quotes. using these features callers can force options to contain whitespace,
 * other backslashes, or quote characters.
 *
 * BUG: get_option() will not return an error if an option contains an
 * unterminated quote character. The string "'one more time" will yield a single
 * option "'one more time". callers will need to deal with this explicitly, if
 * they care.
 *
 * NB: get_option() will allow callers to incldude quotes in the middle of
 * an option; e.g., "one'    'two" will expand to "one    two" rather than
 * "one" and "two". This is consistent with unix shell behavior, but not
 * consistent with some implementations of the @file command line option.
 */
static char* get_option(char** buf)
{
  char* p = NULL; // beginning of option
  char* q = NULL; // end of option

  while (buf && *buf && *(*buf)) {
    char c = *(*buf);

    // whitespace
    //   ignore the space. if in an option, end option parsing. the option
    //   string (q) will be terminated later.
    if (' ' == c || '\t' == c || '\n' == c || '\r' == c) {
      (*buf)++;
      if (p)
        break;
    }

    // backslash
    //   ignore the backslash, but treat the next character as a literal
    //   character. start an option if not yet in an option.
    else if ('\\' == c) {
      // ignore the backslash (don't advance q)
      (*buf)++;
      // start a new option if necessary
      if (!p)
        p = q = *buf;
      // if the string continues, include that next character in the option.
      if (*(*buf)) {
        *q++ = *(*buf);
        (*buf)++;
      }
    }

    // single or double quote
    //   ignore the quote character, but treat all characters (except backslash
    //   escaped cahracters) until a closing character as literal characters.
    //
    //   BUG: unterminated quotes are indistinguishable from terminated ones.
    else if ('\'' == c || '"' == c) {
      // ignore the quote (don't advance q)
      (*buf)++;
      // start a new option if necessary
      if (!p)
        p = q = *buf;
      // consume remaining characters
      while (*(*buf) && c != *(*buf)) {
        if ('\\' == *(*buf)) {
          // ignore the backslash (don't advance q)
          (*buf)++;
          // if the string continues, include that next character in the option.
          if (*(*buf)) {
            *q++ = *(*buf);
            (*buf)++;
          }
        }
        else {
          // include this character in the option.
          *q++ = *(*buf);
          (*buf)++;
        }
      }
      // ignore the closing quote if we found one (don't advance q)
      if (*(*buf))
        (*buf)++;
    }

    // default (all other characters)
    //   start an option if necessary, and consume the character
    else {
      if (!p)
        p = q = *buf;
      *q++ = *(*buf);
      (*buf)++;
    }
  }

  // terminate the option string
  if (q)
    *q = '\0';

  return p;
}

/*
 * string_list_add() adds a string to the list.
 */
static void string_list_add(struct string_list* list, const char* str)
{
  list->strs = reallocf(list->strs, sizeof(char*) * (list->nstr + 1));
  if (!list->strs) {
    system_fatal("reallocf failed");
  }
  list->strs[list->nstr++] = strdup(str);
}

/*
 * string_list_add_argv() adds an array of strings to the string list.
 */
static void string_list_add_argv(struct string_list* list, int argc,
char* argv[])
{
  list->strs = reallocf(list->strs, sizeof(char*) * (list->nstr + argc));
  if (!list->strs) {
    system_fatal("reallocf failed");
  }
  for (int i = 0; i < argc; ++i) {
    list->strs[list->nstr++] = strdup(argv[i]);
  }
}

/*
 * string_list_find() returns the index of str in the string list, or -1 if
 * the string is not found.
 */
static int string_list_find(const struct string_list* list, const char* str)
{
  for (int i = 0; i < list->nstr; ++i) {
    if (0 == strcmp(str, list->strs[i]))
      return i;
  }
  return -1;
}

/*
 * string_list_dest() frees the individual strings being held in the strs
 * array, as well as the strs array itself. it does not free the struct
 * strings_list pointer; instead it zeroes out the struct members.
 *
 * BUG: this function is not called string_list_free() because that might
 * imply it also frees the struct string_list, which it does not.
 */
static void string_list_dest(struct string_list* list)
{
  for (int i = 0; i < list->nstr; ++i) {
    free(list->strs[i]);
  }
  free(list->strs);
  list->strs = NULL;
  list->nstr = 0;
}
