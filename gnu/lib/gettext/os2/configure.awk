#!/usr/bin/gawk -f
# A script for emulating configure on OS/2 without having even a Unix-like
# shell. Designed specifically for compiling gettext with gcc+emx.

BEGIN{
  print "/* config.h.  Generated automatically by configure.awk.  */"

  cfg["HAVE_ALLOCA"] = 1;
  cfg["HAVE_ALLOCA_H"] = 1;
  cfg["HAVE_LONG_FILE_NAMES"] = 1;
  cfg["STDC_HEADERS"] = 1;
  cfg["HAVE_GETCWD"] = 1;
  cfg["HAVE_GETEGID"] = 1;
  cfg["HAVE_GETEUID"] = 1;
  cfg["HAVE_GETGID"] = 1;
  cfg["HAVE_GETPAGESIZE"] = 1;
  cfg["HAVE_GETUID"] = 1;
  cfg["HAVE_ISASCII"] = 1;
  cfg["HAVE_MBLEN"] = 1;
  cfg["HAVE_MEMCPY"] = 1;
  cfg["HAVE_MEMMOVE"] = 1;
  cfg["HAVE_MEMSET"] = 1;
  cfg["HAVE_PUTENV"] = 1;
  cfg["HAVE_STRCHR"] = 1;
  cfg["HAVE_STRCSPN"] = 1;
  cfg["HAVE_STRDUP"] = 1;
  cfg["HAVE_STRERROR"] = 1;
  cfg["HAVE_STRSTR"] = 1;
  cfg["HAVE_STRTOUL"] = 1;
  cfg["HAVE_UNAME"] = 1;
  cfg["HAVE_LIMITS_H"] = 1;
  cfg["HAVE_LOCALE_H"] = 1;
  cfg["HAVE_MALLOC_H"] = 1;
  cfg["HAVE_STDDEF_H"] = 1;
  cfg["HAVE_STDLIB_H"] = 1;
  cfg["HAVE_STRING_H"] = 1;
  cfg["HAVE_SYS_PARAM_H"] = 1;
  cfg["HAVE_UNISTD_H"] = 1;
  cfg["HAVE_GETTIMEOFDAY"] = 1;
  cfg["HAVE_PATHCONF"] = 1;
  cfg["HAVE_RAISE"] = 1;
  cfg["HAVE_SELECT"] = 1;
  cfg["HAVE_STRPBRK"] = 1;
  cfg["HAVE_UTIME"] = 1;
  cfg["HAVE_UTIMES"] = 1;
  cfg["HAVE_WAITPID"] = 1;
  cfg["HAVE_ARPA_INET_H"] = 1;
  cfg["HAVE_DIRENT_H"] = 1;
  cfg["HAVE_FCNTL_H"] = 1;
  cfg["HAVE_SYS_TIME_H"] = 1;
  cfg["HAVE_TIME_H"] = 1;
  cfg["HAVE_POSIX_SIGNALBLOCKING"] = 1;
  cfg["HAVE_ERRNO_DECL"] = 1;
  cfg["HAVE_ICONV"] = 1;
  cfg["ICONV_CONST"] = "const";
  cfg["_GNU_SOURCE"] = 1;
  cfg["HAVE_UNSIGNED_LONG_LONG"] = 1;
  cfg["HAVE_PTRDIFF_T"] = 1;
  cfg["vfork"] = "fork";
  cfg["uintmax_t"] = "unsigned long long";
  cfg["HAVE_DECL_WCWIDTH"] = 0;
  cfg["mbstate_t"] = "int";
  cfg["SETLOCALE_CONST"] = "const";
  cfg["ENABLE_NLS"] = 1;

  cfg["PACKAGE"] = "\""PACKAGE"\"";
  cfg["VERSION"] = "\""VERSION"\"";
}

/^#undef/ {
  if (cfg[$2] != "")
    print "#define "$2" "cfg[$2];
  else
    print "/* #undef "$2" */";
  next
}

{
  print $0
}
