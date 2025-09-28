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
constant (pTHX_ const char *name, STRLEN len, IV *iv_return) {
  /* Initially switch on the length of the name.  */
  /* When generated this function returned values for the list of names given
     in this section of perl code.  Rather than manually editing these functions
     to add or remove constants, which would result in this comment and section
     of code becoming inaccurate, we recommend that you edit this section of
     code, and use it to regenerate a new set of constant functions which you
     then use to replace the originals.

     Regenerate these constant functions by feeding this entire source file to
     perl -x

#!/spare/local/perls/5.8.6/bin/perl5.8.6 -w
use ExtUtils::Constant qw (constant_types C_constant XS_constant);

my $types = {map {($_, 1)} qw(IV)};
my @names = (qw(BZ_CONFIG_ERROR BZ_DATA_ERROR BZ_DATA_ERROR_MAGIC BZ_FINISH
	       BZ_FINISH_OK BZ_FLUSH BZ_FLUSH_OK BZ_IO_ERROR BZ_MEM_ERROR BZ_OK
	       BZ_OUTBUFF_FULL BZ_PARAM_ERROR BZ_RUN BZ_RUN_OK
	       BZ_SEQUENCE_ERROR BZ_STREAM_END BZ_UNEXPECTED_EOF));

print constant_types(); # macro defs
foreach (C_constant ("Bzip2", 'constant', 'IV', $types, undef, 3, @names) ) {
    print $_, "\n"; # C constant subs
}
print "#### XS Section:\n";
print XS_constant ("Bzip2", $types);
__END__
   */

  switch (len) {
  case 5:
    if (memEQ(name, "BZ_OK", 5)) {
#ifdef BZ_OK
      *iv_return = BZ_OK;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 6:
    if (memEQ(name, "BZ_RUN", 6)) {
#ifdef BZ_RUN
      *iv_return = BZ_RUN;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 8:
    if (memEQ(name, "BZ_FLUSH", 8)) {
#ifdef BZ_FLUSH
      *iv_return = BZ_FLUSH;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 9:
    /* Names all of length 9.  */
    /* BZ_FINISH BZ_RUN_OK */
    /* Offset 8 gives the best switch position.  */
    switch (name[8]) {
    case 'H':
      if (memEQ(name, "BZ_FINIS", 8)) {
      /*                       H     */
#ifdef BZ_FINISH
        *iv_return = BZ_FINISH;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'K':
      if (memEQ(name, "BZ_RUN_O", 8)) {
      /*                       K     */
#ifdef BZ_RUN_OK
        *iv_return = BZ_RUN_OK;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 11:
    /* Names all of length 11.  */
    /* BZ_FLUSH_OK BZ_IO_ERROR */
    /* Offset 3 gives the best switch position.  */
    switch (name[3]) {
    case 'F':
      if (memEQ(name, "BZ_FLUSH_OK", 11)) {
      /*                  ^              */
#ifdef BZ_FLUSH_OK
        *iv_return = BZ_FLUSH_OK;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'I':
      if (memEQ(name, "BZ_IO_ERROR", 11)) {
      /*                  ^              */
#ifdef BZ_IO_ERROR
        *iv_return = BZ_IO_ERROR;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 12:
    /* Names all of length 12.  */
    /* BZ_FINISH_OK BZ_MEM_ERROR */
    /* Offset 5 gives the best switch position.  */
    switch (name[5]) {
    case 'M':
      if (memEQ(name, "BZ_MEM_ERROR", 12)) {
      /*                    ^             */
#ifdef BZ_MEM_ERROR
        *iv_return = BZ_MEM_ERROR;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'N':
      if (memEQ(name, "BZ_FINISH_OK", 12)) {
      /*                    ^             */
#ifdef BZ_FINISH_OK
        *iv_return = BZ_FINISH_OK;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 13:
    /* Names all of length 13.  */
    /* BZ_DATA_ERROR BZ_STREAM_END */
    /* Offset 11 gives the best switch position.  */
    switch (name[11]) {
    case 'N':
      if (memEQ(name, "BZ_STREAM_END", 13)) {
      /*                          ^        */
#ifdef BZ_STREAM_END
        *iv_return = BZ_STREAM_END;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'O':
      if (memEQ(name, "BZ_DATA_ERROR", 13)) {
      /*                          ^        */
#ifdef BZ_DATA_ERROR
        *iv_return = BZ_DATA_ERROR;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 14:
    if (memEQ(name, "BZ_PARAM_ERROR", 14)) {
#ifdef BZ_PARAM_ERROR
      *iv_return = BZ_PARAM_ERROR;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 15:
    /* Names all of length 15.  */
    /* BZ_CONFIG_ERROR BZ_OUTBUFF_FULL */
    /* Offset 8 gives the best switch position.  */
    switch (name[8]) {
    case 'F':
      if (memEQ(name, "BZ_OUTBUFF_FULL", 15)) {
      /*                       ^             */
#ifdef BZ_OUTBUFF_FULL
        *iv_return = BZ_OUTBUFF_FULL;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'G':
      if (memEQ(name, "BZ_CONFIG_ERROR", 15)) {
      /*                       ^             */
#ifdef BZ_CONFIG_ERROR
        *iv_return = BZ_CONFIG_ERROR;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 17:
    /* Names all of length 17.  */
    /* BZ_SEQUENCE_ERROR BZ_UNEXPECTED_EOF */
    /* Offset 12 gives the best switch position.  */
    switch (name[12]) {
    case 'D':
      if (memEQ(name, "BZ_UNEXPECTED_EOF", 17)) {
      /*                           ^           */
#ifdef BZ_UNEXPECTED_EOF
        *iv_return = BZ_UNEXPECTED_EOF;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'E':
      if (memEQ(name, "BZ_SEQUENCE_ERROR", 17)) {
      /*                           ^           */
#ifdef BZ_SEQUENCE_ERROR
        *iv_return = BZ_SEQUENCE_ERROR;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 19:
    if (memEQ(name, "BZ_DATA_ERROR_MAGIC", 19)) {
#ifdef BZ_DATA_ERROR_MAGIC
      *iv_return = BZ_DATA_ERROR_MAGIC;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

