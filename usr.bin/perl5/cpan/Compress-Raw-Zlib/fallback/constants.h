#define PERL_constant_NOTFOUND	1
#define PERL_constant_NOTDEF	2
#define PERL_constant_ISIV	3
#define PERL_constant_ISNO	4
#define PERL_constant_ISNV	5
#define PERL_constant_ISPV	6
#define PERL_constant_ISPVN	7
#define PERL_constant_ISSV	8
#define PERL_constant_ISUNDEF	9
#define PERL_constant_ISUV	10
#define PERL_constant_ISYES	11

#ifndef NVTYPE
typedef double NV; /* 5.6 and later define NVTYPE, and typedef NV to it.  */
#endif
#ifndef aTHX_
#define aTHX_ /* 5.6 or later define this for threading support.  */
#endif
#ifndef pTHX_
#define pTHX_ /* 5.6 or later define this for threading support.  */
#endif

static int
constant_7 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     OS_CODE Z_ASCII Z_BLOCK Z_ERRNO Z_FIXED Z_TREES */
  /* Offset 6 gives the best switch position.  */
  switch (name[6]) {
  case 'D':
    if (memEQ(name, "Z_FIXE", 6)) {
    /*                     D     */
#ifdef Z_FIXED
      *iv_return = Z_FIXED;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'E':
    if (memEQ(name, "OS_COD", 6)) {
    /*                     E     */
#ifdef OS_CODE
      *iv_return = OS_CODE;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'I':
    if (memEQ(name, "Z_ASCI", 6)) {
    /*                     I     */
#ifdef Z_ASCII
      *iv_return = Z_ASCII;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'K':
    if (memEQ(name, "Z_BLOC", 6)) {
    /*                     K     */
#ifdef Z_BLOCK
      *iv_return = Z_BLOCK;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'O':
    if (memEQ(name, "Z_ERRN", 6)) {
    /*                     O     */
#ifdef Z_ERRNO
      *iv_return = Z_ERRNO;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'S':
    if (memEQ(name, "Z_TREE", 6)) {
    /*                     S     */
#if ZLIB_VERNUM >= 0x1240
      *iv_return = Z_TREES;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_9 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     DEF_WBITS MAX_WBITS Z_UNKNOWN */
  /* Offset 2 gives the best switch position.  */
  switch (name[2]) {
  case 'F':
    if (memEQ(name, "DEF_WBITS", 9)) {
    /*                 ^            */
#ifdef DEF_WBITS
      *iv_return = DEF_WBITS;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'U':
    if (memEQ(name, "Z_UNKNOWN", 9)) {
    /*                 ^            */
#ifdef Z_UNKNOWN
      *iv_return = Z_UNKNOWN;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'X':
    if (memEQ(name, "MAX_WBITS", 9)) {
    /*                 ^            */
#ifdef MAX_WBITS
      *iv_return = MAX_WBITS;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_10 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     Z_DEFLATED Z_FILTERED Z_NO_FLUSH */
  /* Offset 7 gives the best switch position.  */
  switch (name[7]) {
  case 'R':
    if (memEQ(name, "Z_FILTERED", 10)) {
    /*                      ^         */
#ifdef Z_FILTERED
      *iv_return = Z_FILTERED;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'T':
    if (memEQ(name, "Z_DEFLATED", 10)) {
    /*                      ^         */
#ifdef Z_DEFLATED
      *iv_return = Z_DEFLATED;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'U':
    if (memEQ(name, "Z_NO_FLUSH", 10)) {
    /*                      ^         */
#ifdef Z_NO_FLUSH
      *iv_return = Z_NO_FLUSH;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_11 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     Z_BUF_ERROR Z_MEM_ERROR Z_NEED_DICT */
  /* Offset 4 gives the best switch position.  */
  switch (name[4]) {
  case 'E':
    if (memEQ(name, "Z_NEED_DICT", 11)) {
    /*                   ^             */
#ifdef Z_NEED_DICT
      *iv_return = Z_NEED_DICT;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'F':
    if (memEQ(name, "Z_BUF_ERROR", 11)) {
    /*                   ^             */
#ifdef Z_BUF_ERROR
      *iv_return = Z_BUF_ERROR;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'M':
    if (memEQ(name, "Z_MEM_ERROR", 11)) {
    /*                   ^             */
#ifdef Z_MEM_ERROR
      *iv_return = Z_MEM_ERROR;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_12 (pTHX_ const char *name, IV *iv_return, const char **pv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     ZLIB_VERSION Z_BEST_SPEED Z_DATA_ERROR Z_FULL_FLUSH Z_STREAM_END
     Z_SYNC_FLUSH */
  /* Offset 4 gives the best switch position.  */
  switch (name[4]) {
  case 'L':
    if (memEQ(name, "Z_FULL_FLUSH", 12)) {
    /*                   ^              */
#ifdef Z_FULL_FLUSH
      *iv_return = Z_FULL_FLUSH;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'N':
    if (memEQ(name, "Z_SYNC_FLUSH", 12)) {
    /*                   ^              */
#ifdef Z_SYNC_FLUSH
      *iv_return = Z_SYNC_FLUSH;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'R':
    if (memEQ(name, "Z_STREAM_END", 12)) {
    /*                   ^              */
#ifdef Z_STREAM_END
      *iv_return = Z_STREAM_END;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'S':
    if (memEQ(name, "Z_BEST_SPEED", 12)) {
    /*                   ^              */
#ifdef Z_BEST_SPEED
      *iv_return = Z_BEST_SPEED;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'T':
    if (memEQ(name, "Z_DATA_ERROR", 12)) {
    /*                   ^              */
#ifdef Z_DATA_ERROR
      *iv_return = Z_DATA_ERROR;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case '_':
    if (memEQ(name, "ZLIB_VERSION", 12)) {
    /*                   ^              */
#ifdef ZLIB_VERSION
      *pv_return = ZLIB_VERSION;
      return PERL_constant_ISPV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant (pTHX_ const char *name, STRLEN len, IV *iv_return, const char **pv_return) {
  /* Initially switch on the length of the name.  */
  /* When generated this function returned values for the list of names given
     in this section of perl code.  Rather than manually editing these functions
     to add or remove constants, which would result in this comment and section
     of code becoming inaccurate, we recommend that you edit this section of
     code, and use it to regenerate a new set of constant functions which you
     then use to replace the originals.

     Regenerate these constant functions by feeding this entire source file to
     perl -x

#!/linux-shared/base/perl/install/bin/perl -w
use ExtUtils::Constant qw (constant_types C_constant XS_constant);

my $types = {map {($_, 1)} qw(IV PV)};
my @names = (qw(DEF_WBITS MAX_MEM_LEVEL MAX_WBITS OS_CODE Z_ASCII
	       Z_BEST_COMPRESSION Z_BEST_SPEED Z_BINARY Z_BLOCK Z_BUF_ERROR
	       Z_DATA_ERROR Z_DEFAULT_COMPRESSION Z_DEFAULT_STRATEGY Z_DEFLATED
	       Z_ERRNO Z_FILTERED Z_FINISH Z_FIXED Z_FULL_FLUSH Z_HUFFMAN_ONLY
	       Z_MEM_ERROR Z_NEED_DICT Z_NO_COMPRESSION Z_NO_FLUSH Z_NULL Z_OK
	       Z_PARTIAL_FLUSH Z_RLE Z_STREAM_END Z_STREAM_ERROR Z_SYNC_FLUSH
	       Z_UNKNOWN Z_VERSION_ERROR),
            {name=>"ZLIB_VERSION", type=>"PV"},
            {name=>"Z_TREES", type=>"IV", macro=>["#if ZLIB_VERNUM >= 0x1240\n", "#endif\n"]});

print constant_types(), "\n"; # macro defs
foreach (C_constant ("Zlib", 'constant', 'IV', $types, undef, 3, @names) ) {
    print $_, "\n"; # C constant subs
}
print "\n#### XS Section:\n";
print XS_constant ("Zlib", $types);
__END__
   */

  switch (len) {
  case 4:
    if (memEQ(name, "Z_OK", 4)) {
#ifdef Z_OK
      *iv_return = Z_OK;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 5:
    if (memEQ(name, "Z_RLE", 5)) {
#ifdef Z_RLE
      *iv_return = Z_RLE;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 6:
    if (memEQ(name, "Z_NULL", 6)) {
#ifdef Z_NULL
      *iv_return = Z_NULL;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 7:
    return constant_7 (aTHX_ name, iv_return);
    break;
  case 8:
    /* Names all of length 8.  */
    /* Z_BINARY Z_FINISH */
    /* Offset 6 gives the best switch position.  */
    switch (name[6]) {
    case 'R':
      if (memEQ(name, "Z_BINARY", 8)) {
      /*                     ^       */
#ifdef Z_BINARY
        *iv_return = Z_BINARY;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'S':
      if (memEQ(name, "Z_FINISH", 8)) {
      /*                     ^       */
#ifdef Z_FINISH
        *iv_return = Z_FINISH;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 9:
    return constant_9 (aTHX_ name, iv_return);
    break;
  case 10:
    return constant_10 (aTHX_ name, iv_return);
    break;
  case 11:
    return constant_11 (aTHX_ name, iv_return);
    break;
  case 12:
    return constant_12 (aTHX_ name, iv_return, pv_return);
    break;
  case 13:
    if (memEQ(name, "MAX_MEM_LEVEL", 13)) {
#ifdef MAX_MEM_LEVEL
      *iv_return = MAX_MEM_LEVEL;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 14:
    /* Names all of length 14.  */
    /* Z_HUFFMAN_ONLY Z_STREAM_ERROR */
    /* Offset 3 gives the best switch position.  */
    switch (name[3]) {
    case 'T':
      if (memEQ(name, "Z_STREAM_ERROR", 14)) {
      /*                  ^                 */
#ifdef Z_STREAM_ERROR
        *iv_return = Z_STREAM_ERROR;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'U':
      if (memEQ(name, "Z_HUFFMAN_ONLY", 14)) {
      /*                  ^                 */
#ifdef Z_HUFFMAN_ONLY
        *iv_return = Z_HUFFMAN_ONLY;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 15:
    /* Names all of length 15.  */
    /* Z_PARTIAL_FLUSH Z_VERSION_ERROR */
    /* Offset 5 gives the best switch position.  */
    switch (name[5]) {
    case 'S':
      if (memEQ(name, "Z_VERSION_ERROR", 15)) {
      /*                    ^                */
#ifdef Z_VERSION_ERROR
        *iv_return = Z_VERSION_ERROR;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'T':
      if (memEQ(name, "Z_PARTIAL_FLUSH", 15)) {
      /*                    ^                */
#ifdef Z_PARTIAL_FLUSH
        *iv_return = Z_PARTIAL_FLUSH;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 16:
    if (memEQ(name, "Z_NO_COMPRESSION", 16)) {
#ifdef Z_NO_COMPRESSION
      *iv_return = Z_NO_COMPRESSION;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 18:
    /* Names all of length 18.  */
    /* Z_BEST_COMPRESSION Z_DEFAULT_STRATEGY */
    /* Offset 14 gives the best switch position.  */
    switch (name[14]) {
    case 'S':
      if (memEQ(name, "Z_BEST_COMPRESSION", 18)) {
      /*                             ^          */
#ifdef Z_BEST_COMPRESSION
        *iv_return = Z_BEST_COMPRESSION;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'T':
      if (memEQ(name, "Z_DEFAULT_STRATEGY", 18)) {
      /*                             ^          */
#ifdef Z_DEFAULT_STRATEGY
        *iv_return = Z_DEFAULT_STRATEGY;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 21:
    if (memEQ(name, "Z_DEFAULT_COMPRESSION", 21)) {
#ifdef Z_DEFAULT_COMPRESSION
      *iv_return = Z_DEFAULT_COMPRESSION;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

