/* atof_ieee.c - turn a Flonum into an IEEE floating point number
   Copyright (C) 1987-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "safe-ctype.h"

/* Flonums returned here.  */
extern FLONUM_TYPE generic_floating_point_number;

/* Precision in LittleNums.  */
/* Don't count the gap in the m68k extended precision format.  */
#define MAX_PRECISION  5
#define H_PRECISION    1
#define B_PRECISION    1 /* Not strictly IEEE, but handled here anyway.  */
#define F_PRECISION    2
#define D_PRECISION    4
#define X_PRECISION    5
#ifndef X_PRECISION_PAD
#define X_PRECISION_PAD 0
#endif
#define P_PRECISION    5
#ifndef P_PRECISION_PAD
#define P_PRECISION_PAD X_PRECISION_PAD
#endif

/* Length in LittleNums of guard bits.  */
#define GUARD          2

#ifndef TC_LARGEST_EXPONENT_IS_NORMAL
#define TC_LARGEST_EXPONENT_IS_NORMAL(PRECISION) 0
#endif

static const unsigned long mask[] =
{
  0x00000000,
  0x00000001,
  0x00000003,
  0x00000007,
  0x0000000f,
  0x0000001f,
  0x0000003f,
  0x0000007f,
  0x000000ff,
  0x000001ff,
  0x000003ff,
  0x000007ff,
  0x00000fff,
  0x00001fff,
  0x00003fff,
  0x00007fff,
  0x0000ffff,
  0x0001ffff,
  0x0003ffff,
  0x0007ffff,
  0x000fffff,
  0x001fffff,
  0x003fffff,
  0x007fffff,
  0x00ffffff,
  0x01ffffff,
  0x03ffffff,
  0x07ffffff,
  0x0fffffff,
  0x1fffffff,
  0x3fffffff,
  0x7fffffff,
  0xffffffff,
};

static int bits_left_in_littlenum;
static int littlenums_left;
static LITTLENUM_TYPE *littlenum_pointer;

static int
next_bits (int number_of_bits)
{
  int return_value;

  if (!littlenums_left)
    return 0;

  if (number_of_bits >= bits_left_in_littlenum)
    {
      return_value = mask[bits_left_in_littlenum] & *littlenum_pointer;
      number_of_bits -= bits_left_in_littlenum;
      return_value <<= number_of_bits;

      if (--littlenums_left)
	{
	  bits_left_in_littlenum = LITTLENUM_NUMBER_OF_BITS - number_of_bits;
	  --littlenum_pointer;
	  return_value |=
	    (*littlenum_pointer >> bits_left_in_littlenum)
	    & mask[number_of_bits];
	}
    }
  else
    {
      bits_left_in_littlenum -= number_of_bits;
      return_value =
	mask[number_of_bits] & (*littlenum_pointer >> bits_left_in_littlenum);
    }
  return return_value;
}

/* Num had better be less than LITTLENUM_NUMBER_OF_BITS.  */

static void
unget_bits (int num)
{
  if (!littlenums_left)
    {
      ++littlenum_pointer;
      ++littlenums_left;
      bits_left_in_littlenum = num;
    }
  else if (bits_left_in_littlenum + num > LITTLENUM_NUMBER_OF_BITS)
    {
      bits_left_in_littlenum =
	num - (LITTLENUM_NUMBER_OF_BITS - bits_left_in_littlenum);
      ++littlenum_pointer;
      ++littlenums_left;
    }
  else
    bits_left_in_littlenum += num;
}

static void
make_invalid_floating_point_number (LITTLENUM_TYPE *words)
{
  as_bad (_("cannot create floating-point number"));
  /* Zero the leftmost bit.  */
  words[0] = (LITTLENUM_TYPE) ((unsigned) -1) >> 1;
  words[1] = (LITTLENUM_TYPE) -1;
  words[2] = (LITTLENUM_TYPE) -1;
  words[3] = (LITTLENUM_TYPE) -1;
  words[4] = (LITTLENUM_TYPE) -1;
  words[5] = (LITTLENUM_TYPE) -1;
}

/* Build a floating point constant at str into a IEEE floating
   point number.  This function does the same thing as atof_ieee
   however it allows more control over the exact format, i.e.
   explicitly specifying the precision and number of exponent bits
   instead of relying on this infomation being deduced from a given type.

   If generic_float_info is not NULL then it will be set to contain generic
   infomation about the parsed floating point number.

   Returns pointer past text consumed. */
char *
atof_ieee_detail (char * str,
		  int precision,
		  int exponent_bits,
		  LITTLENUM_TYPE * words,
		  FLONUM_TYPE * generic_float_info)
{
  /* Extra bits for zeroed low-order bits.
     The 1st MAX_PRECISION are zeroed, the last contain flonum bits.  */
  static LITTLENUM_TYPE bits[MAX_PRECISION + MAX_PRECISION + GUARD];
  char *return_value;

  /* Number of 16-bit words in the format.  */
  FLONUM_TYPE save_gen_flonum;

  /* We have to save the generic_floating_point_number because it
     contains storage allocation about the array of LITTLENUMs where
     the value is actually stored.  We will allocate our own array of
     littlenums below, but have to restore the global one on exit.  */
  save_gen_flonum = generic_floating_point_number;

  return_value = str;
  generic_floating_point_number.low = bits + MAX_PRECISION;
  generic_floating_point_number.high = NULL;
  generic_floating_point_number.leader = NULL;
  generic_floating_point_number.exponent = 0;
  generic_floating_point_number.sign = '\0';

  /* Use more LittleNums than seems necessary: the highest flonum may
     have 15 leading 0 bits, so could be useless.  */

  memset (bits, '\0', sizeof (LITTLENUM_TYPE) * MAX_PRECISION);

  generic_floating_point_number.high
    = generic_floating_point_number.low + precision - 1 + GUARD;

  if (atof_generic (&return_value, ".", EXP_CHARS,
		    &generic_floating_point_number))
    {
      make_invalid_floating_point_number (words);
      return NULL;
    }

  if (generic_float_info)
    *generic_float_info = generic_floating_point_number;

  gen_to_words (words, precision, exponent_bits);

  /* Restore the generic_floating_point_number's storage alloc (and
     everything else).  */
  generic_floating_point_number = save_gen_flonum;

  return return_value;
}

/* Warning: This returns 16-bit LITTLENUMs.  It is up to the caller to
   figure out any alignment problems and to conspire for the
   bytes/word to be emitted in the right order.  Bigendians beware!  */

/* Note that atof-ieee always has X and P precisions enabled.  it is up
   to md_atof to filter them out if the target machine does not support
   them.  */

/* Returns pointer past text consumed.  */
char *
atof_ieee (char *str,			/* Text to convert to binary.  */
	   int what_kind,		/* 'd', 'f', 'x', 'p'.  */
	   LITTLENUM_TYPE *words)	/* Build the binary here.  */
{
  int precision;
  long exponent_bits;

  switch (what_kind)
    {
    case 'h':
    case 'H':
      precision = H_PRECISION;
      exponent_bits = 5;
      break;

    case 'b':
    case 'B':
      precision = B_PRECISION;
      exponent_bits = 8;
      break;

    case 'f':
    case 'F':
    case 's':
    case 'S':
      precision = F_PRECISION;
      exponent_bits = 8;
      break;

    case 'd':
    case 'D':
    case 'r':
    case 'R':
      precision = D_PRECISION;
      exponent_bits = 11;
      break;

    case 'x':
    case 'X':
    case 'e':
    case 'E':
      precision = X_PRECISION;
      exponent_bits = 15;
      break;

    case 'p':
    case 'P':
      precision = P_PRECISION;
      exponent_bits = -1;
      break;

    default:
      make_invalid_floating_point_number (words);
      return (NULL);
    }

  return atof_ieee_detail (str, precision, exponent_bits, words, NULL);
}

/* Turn generic_floating_point_number into a real float/double/extended.  */

int
gen_to_words (LITTLENUM_TYPE *words, int precision, long exponent_bits)
{
  int return_value = 0;

  long exponent_1;
  long exponent_2;
  long exponent_3;
  long exponent_4;
  int exponent_skippage;
  LITTLENUM_TYPE word1;
  LITTLENUM_TYPE *lp;
  LITTLENUM_TYPE *words_end;

  words_end = words + precision;
#ifdef TC_M68K
  if (precision == X_PRECISION)
    /* On the m68k the extended precision format has a gap of 16 bits
       between the exponent and the mantissa.  */
    words_end++;
#endif

  if (generic_floating_point_number.low > generic_floating_point_number.leader)
    {
      /* 0.0e0 seen.  */
      if (generic_floating_point_number.sign == '+')
	words[0] = 0x0000;
      else
	words[0] = 0x8000;
      memset (&words[1], '\0',
	      (words_end - words - 1) * sizeof (LITTLENUM_TYPE));
      return return_value;
    }

  switch (generic_floating_point_number.sign)
    {
    /* NaN:  Do the right thing.  */
    case 0:
    case 'Q': case 'q':
    case 'S': case 's':
      if (TC_LARGEST_EXPONENT_IS_NORMAL (precision))
	as_warn (_("NaNs are not supported by this target"));

      if (precision == H_PRECISION)
	{
	  if (TOUPPER (generic_floating_point_number.sign) != 'S')
	    words[0] = 0x7fff;
	  else
	    words[0] = exponent_bits == 5 ? 0x7dff : 0x7fbf;
	}
      else if (precision == F_PRECISION)
	{
	  words[0] = TOUPPER (generic_floating_point_number.sign) == 'S'
	             ? 0x7fbf : 0x7fff;
	  words[1] = 0xffff;
	}
      else if (precision == X_PRECISION)
	{
#ifdef TC_M68K
	  if (generic_floating_point_number.sign)
	    as_warn (_("NaN flavors are not supported by this target"));

	  words[0] = 0x7fff;
	  words[1] = 0;
	  words[2] = 0xffff;
	  words[3] = 0xffff;
	  words[4] = 0xffff;
	  words[5] = 0xffff;
#else /* ! TC_M68K  */
#ifdef TC_I386
	  words[0] = 0x7fff;
	  words[1] = TOUPPER (generic_floating_point_number.sign) == 'S'
		     ? 0xbfff : 0xffff;
	  words[2] = 0xffff;
	  words[3] = 0xffff;
	  words[4] = 0xffff;
#else /* ! TC_I386  */
	  abort ();
#endif /* ! TC_I386  */
#endif /* ! TC_M68K  */
	}
      else
	{
	  words[0] = TOUPPER (generic_floating_point_number.sign) == 'S'
	             ? 0x7ff7 : 0x7fff;
	  words[1] = 0xffff;
	  words[2] = 0xffff;
	  words[3] = 0xffff;
	}

      if (ISLOWER (generic_floating_point_number.sign))
	words[0] |= 0x8000;

      return return_value;

    case 'P':
    case 'N':
      if (TC_LARGEST_EXPONENT_IS_NORMAL (precision))
	as_warn (_("Infinities are not supported by this target"));

      /* +INF:  Do the right thing.  */
      if (precision == H_PRECISION /* also B_PRECISION */)
	{
	  words[0] = exponent_bits == 5 ? 0x7c00 : 0x7f80;
	}
      else if (precision == F_PRECISION)
	{
	  words[0] = 0x7f80;
	  words[1] = 0;
	}
      else if (precision == X_PRECISION)
	{
#ifdef TC_M68K
	  words[0] = 0x7fff;
	  words[1] = 0;
	  words[2] = 0;
	  words[3] = 0;
	  words[4] = 0;
	  words[5] = 0;
#else /* ! TC_M68K  */
#ifdef TC_I386
	  words[0] = 0x7fff;
	  words[1] = 0x8000;
	  words[2] = 0;
	  words[3] = 0;
	  words[4] = 0;
#else /* ! TC_I386  */
	  abort ();
#endif /* ! TC_I386  */
#endif /* ! TC_M68K  */
	}
      else
	{
	  words[0] = 0x7ff0;
	  words[1] = 0;
	  words[2] = 0;
	  words[3] = 0;
	}

      if (generic_floating_point_number.sign == 'N')
	words[0] |= 0x8000;

      return return_value;
    }

  /* The floating point formats we support have:
     Bit 15 is sign bit.
     Bits 14:n are excess-whatever exponent.
     Bits n-1:0 (if any) are most significant bits of fraction.
     Bits 15:0 of the next word(s) are the next most significant bits.

     So we need: number of bits of exponent, number of bits of
     mantissa.  */
  bits_left_in_littlenum = LITTLENUM_NUMBER_OF_BITS;
  littlenum_pointer = generic_floating_point_number.leader;
  littlenums_left = (1
		     + generic_floating_point_number.leader
		     - generic_floating_point_number.low);

  /* Seek (and forget) 1st significant bit.  */
  for (exponent_skippage = 0; !next_bits (1); ++exponent_skippage);
  exponent_1 = (generic_floating_point_number.exponent
		+ generic_floating_point_number.leader
		+ 1
		- generic_floating_point_number.low);

  /* Radix LITTLENUM_RADIX, point just higher than
     generic_floating_point_number.leader.  */
  exponent_2 = exponent_1 * LITTLENUM_NUMBER_OF_BITS;

  /* Radix 2.  */
  exponent_3 = exponent_2 - exponent_skippage;

  /* Forget leading zeros, forget 1st bit.  */
  exponent_4 = exponent_3 + ((1 << (exponent_bits - 1)) - 2);

  /* Offset exponent.  */
  lp = words;

  /* Word 1.  Sign, exponent and perhaps high bits.  */
  word1 = ((generic_floating_point_number.sign == '+')
	   ? 0
	   : (1 << (LITTLENUM_NUMBER_OF_BITS - 1)));

  /* Assume 2's complement integers.  */
  if (exponent_4 <= 0)
    {
      int prec_bits;
      int num_bits;

      unget_bits (1);
      num_bits = -exponent_4;
      prec_bits =
	LITTLENUM_NUMBER_OF_BITS * precision - (exponent_bits + 1 + num_bits);
#ifdef TC_I386
      if (precision == X_PRECISION && exponent_bits == 15)
	{
	  /* On the i386 a denormalized extended precision float is
	     shifted down by one, effectively decreasing the exponent
	     bias by one.  */
	  prec_bits -= 1;
	  num_bits += 1;
	}
#endif

      if (num_bits >= LITTLENUM_NUMBER_OF_BITS - exponent_bits)
	{
	  /* Bigger than one littlenum.  */
	  num_bits -= (LITTLENUM_NUMBER_OF_BITS - 1) - exponent_bits;
	  *lp++ = word1;
	  if (num_bits + exponent_bits + 1
	      > precision * LITTLENUM_NUMBER_OF_BITS)
	    {
	      /* Exponent overflow.  */
	      make_invalid_floating_point_number (words);
	      return return_value;
	    }
#ifdef TC_M68K
	  if (precision == X_PRECISION && exponent_bits == 15)
	    *lp++ = 0;
#endif
	  while (num_bits >= LITTLENUM_NUMBER_OF_BITS)
	    {
	      num_bits -= LITTLENUM_NUMBER_OF_BITS;
	      *lp++ = 0;
	    }
	  if (num_bits)
	    *lp++ = next_bits (LITTLENUM_NUMBER_OF_BITS - (num_bits));
	}
      else
	{
	  if (precision == X_PRECISION && exponent_bits == 15)
	    {
	      *lp++ = word1;
#ifdef TC_M68K
	      *lp++ = 0;
#endif
	      *lp++ = next_bits (LITTLENUM_NUMBER_OF_BITS - num_bits);
	    }
	  else
	    {
	      word1 |= next_bits ((LITTLENUM_NUMBER_OF_BITS - 1)
				  - (exponent_bits + num_bits));
	      *lp++ = word1;
	    }
	}
      while (lp < words_end)
	*lp++ = next_bits (LITTLENUM_NUMBER_OF_BITS);

      /* Round the mantissa up, but don't change the number.  */
      if (next_bits (1))
	{
	  --lp;
	  if (prec_bits >= LITTLENUM_NUMBER_OF_BITS)
	    {
	      int n = 0;
	      int tmp_bits;

	      n = 0;
	      tmp_bits = prec_bits;
	      while (tmp_bits > LITTLENUM_NUMBER_OF_BITS)
		{
		  if (lp[n] != (LITTLENUM_TYPE) - 1)
		    break;
		  --n;
		  tmp_bits -= LITTLENUM_NUMBER_OF_BITS;
		}
	      if (tmp_bits > LITTLENUM_NUMBER_OF_BITS
		  || (lp[n] & mask[tmp_bits]) != mask[tmp_bits]
		  || (prec_bits != (precision * LITTLENUM_NUMBER_OF_BITS
				    - exponent_bits - 1)
#ifdef TC_I386
		      /* An extended precision float with only the integer
			 bit set would be invalid.  That must be converted
			 to the smallest normalized number.  */
		      && !(precision == X_PRECISION
			   && prec_bits == (precision * LITTLENUM_NUMBER_OF_BITS
					    - exponent_bits - 2))
#endif
		      ))
		{
		  unsigned long carry;

		  for (carry = 1; carry && (lp >= words); lp--)
		    {
		      carry = *lp + carry;
		      *lp = carry;
		      carry >>= LITTLENUM_NUMBER_OF_BITS;
		    }
		}
	      else
		{
		  /* This is an overflow of the denormal numbers.  We
                     need to forget what we have produced, and instead
                     generate the smallest normalized number.  */
		  lp = words;
		  word1 = ((generic_floating_point_number.sign == '+')
			   ? 0
			   : (1 << (LITTLENUM_NUMBER_OF_BITS - 1)));
		  word1 |= (1
			    << ((LITTLENUM_NUMBER_OF_BITS - 1)
				- exponent_bits));
		  *lp++ = word1;
#ifdef TC_I386
		  /* Set the integer bit in the extended precision format.
		     This cannot happen on the m68k where the mantissa
		     just overflows into the integer bit above.  */
		  if (precision == X_PRECISION)
		    *lp++ = 1 << (LITTLENUM_NUMBER_OF_BITS - 1);
#endif
		  while (lp < words_end)
		    *lp++ = 0;
		}
	    }
	  else
	    *lp += 1;
	}

      return return_value;
    }
  else if ((unsigned long) exponent_4 > mask[exponent_bits]
	   || (! TC_LARGEST_EXPONENT_IS_NORMAL (precision)
	       && (unsigned long) exponent_4 == mask[exponent_bits]))
    {
      /* Exponent overflow.  Lose immediately.  */

      /* We leave return_value alone: admit we read the
	 number, but return a floating exception
	 because we can't encode the number.  */
      make_invalid_floating_point_number (words);
      return return_value;
    }
  else
    {
      word1 |= (exponent_4 << ((LITTLENUM_NUMBER_OF_BITS - 1) - exponent_bits))
	| next_bits ((LITTLENUM_NUMBER_OF_BITS - 1) - exponent_bits);
    }

  *lp++ = word1;

  /* X_PRECISION is special: on the 68k, it has 16 bits of zero in the
     middle.  Either way, it is then followed by a 1 bit.  */
  if (exponent_bits == 15 && precision == X_PRECISION)
    {
#ifdef TC_M68K
      *lp++ = 0;
#endif
      *lp++ = (1 << (LITTLENUM_NUMBER_OF_BITS - 1)
	       | next_bits (LITTLENUM_NUMBER_OF_BITS - 1));
    }

  /* The rest of the words are just mantissa bits.  */
  while (lp < words_end)
    *lp++ = next_bits (LITTLENUM_NUMBER_OF_BITS);

  if (next_bits (1))
    {
      unsigned long carry;
      /* Since the NEXT bit is a 1, round UP the mantissa.
	 The cunning design of these hidden-1 floats permits
	 us to let the mantissa overflow into the exponent, and
	 it 'does the right thing'. However, we lose if the
	 highest-order bit of the lowest-order word flips.
	 Is that clear?  */

      /* #if (sizeof(carry)) < ((sizeof(bits[0]) * BITS_PER_CHAR) + 2)
	 Please allow at least 1 more bit in carry than is in a LITTLENUM.
	 We need that extra bit to hold a carry during a LITTLENUM carry
	 propagation. Another extra bit (kept 0) will assure us that we
	 don't get a sticky sign bit after shifting right, and that
	 permits us to propagate the carry without any masking of bits.
	 #endif */
      for (carry = 1, lp--; carry; lp--)
	{
	  carry = *lp + carry;
	  *lp = carry;
	  carry >>= LITTLENUM_NUMBER_OF_BITS;
	  if (lp == words)
	    break;
	}
      if (precision == X_PRECISION && exponent_bits == 15)
	{
	  /* Extended precision numbers have an explicit integer bit
	     that we may have to restore.  */
	  if (lp == words)
	    {
#ifdef TC_M68K
	      /* On the m68k there is a gap of 16 bits.  We must
		 explicitly propagate the carry into the exponent.  */
	      words[0] += words[1];
	      words[1] = 0;
	      lp++;
#endif
	      /* Put back the integer bit.  */
	      lp[1] |= 1 << (LITTLENUM_NUMBER_OF_BITS - 1);
	    }
	}
      if ((word1 ^ *words) & (1 << (LITTLENUM_NUMBER_OF_BITS - 1)))
	{
	  /* We leave return_value alone: admit we read the number,
	     but return a floating exception because we can't encode
	     the number.  */
	  *words &= ~(1 << (LITTLENUM_NUMBER_OF_BITS - 1));
	}
    }
  return return_value;
}

#ifdef TEST
char *
print_gen (gen)
     FLONUM_TYPE *gen;
{
  FLONUM_TYPE f;
  LITTLENUM_TYPE arr[10];
  double dv;
  float fv;
  static char sbuf[40];

  if (gen)
    {
      f = generic_floating_point_number;
      generic_floating_point_number = *gen;
    }
  gen_to_words (&arr[0], 4, 11);
  memcpy (&dv, &arr[0], sizeof (double));
  sprintf (sbuf, "%x %x %x %x %.14G   ", arr[0], arr[1], arr[2], arr[3], dv);
  gen_to_words (&arr[0], 2, 8);
  memcpy (&fv, &arr[0], sizeof (float));
  sprintf (sbuf + strlen (sbuf), "%x %x %.12g\n", arr[0], arr[1], fv);

  if (gen)
    generic_floating_point_number = f;

  return (sbuf);
}
#endif

/* This is a utility function called from various tc-*.c files.  It
   is here in order to reduce code duplication.

   Turn a string at input_line_pointer into a floating point constant
   of type TYPE (a character found in the FLT_CHARS macro), and store
   it as LITTLENUMS in the bytes buffer LITP.  The number of chars
   emitted is stored in *SIZEP.  BIG_WORDIAN is TRUE if the littlenums
   should be emitted most significant littlenum first.

   An error message is returned, or a NULL pointer if everything went OK.  */

const char *
ieee_md_atof (int type,
	      char *litP,
	      int *sizeP,
	      bool big_wordian)
{
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  LITTLENUM_TYPE *wordP;
  char *t;
  int prec = 0, pad = 0;

  if (strchr (FLT_CHARS, type) != NULL)
    {
      switch (type)
	{
	case 'H':
	case 'h':
	  prec = H_PRECISION;
	  break;

	case 'B':
	case 'b':
	  prec = B_PRECISION;
	  break;

	case 'f':
	case 'F':
	case 's':
	case 'S':
	  prec = F_PRECISION;
	  break;

	case 'd':
	case 'D':
	case 'r':
	case 'R':
	  prec = D_PRECISION;
	  break;

	case 't':
	case 'T':
	  prec = X_PRECISION;
	  pad = X_PRECISION_PAD;
	  type = 'x';		/* This is what atof_ieee() understands.  */
	  break;

	case 'x':
	case 'X':
	case 'p':
	case 'P':
#ifdef TC_M68K
	  /* Note: on the m68k there is a gap of 16 bits (one littlenum)
	     between the exponent and mantissa.  Hence the precision is
	     6 and not 5.  */
	  prec = P_PRECISION + 1;
#else
	  prec = P_PRECISION;
#endif
	  pad = P_PRECISION_PAD;
	  break;

	default:
	  break;
	}
    }
  /* The 'f' and 'd' types are always recognised, even if the target has
     not put them into the FLT_CHARS macro.  This is because the 'f' type
     can come from the .dc.s, .dcb.s, .float or .single pseudo-ops and the
     'd' type from the .dc.d, .dbc.d or .double pseudo-ops.

     The 'x' type is not implicitly recognised however, even though it can
     be generated by the .dc.x and .dbc.x pseudo-ops because not all targets
     can support floating point values that big.  ie the target has to
     explicitly allow them by putting them into FLT_CHARS.  */
  else if (type == 'f')
    prec = F_PRECISION;
  else if (type == 'd')
    prec = D_PRECISION;

  if (prec == 0)
    {
      *sizeP = 0;
      return _("Unrecognized or unsupported floating point constant");
    }

  gas_assert (prec <= MAX_LITTLENUMS);

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizeP = (prec + pad) * sizeof (LITTLENUM_TYPE);

  if (big_wordian)
    {
      for (wordP = words; prec --;)
	{
	  md_number_to_chars (litP, (valueT) (* wordP ++), sizeof (LITTLENUM_TYPE));
	  litP += sizeof (LITTLENUM_TYPE);
	}
    }
  else
    {
      for (wordP = words + prec; prec --;)
	{
	  md_number_to_chars (litP, (valueT) (* -- wordP), sizeof (LITTLENUM_TYPE));
	  litP += sizeof (LITTLENUM_TYPE);
	}
    }

  memset (litP, 0, pad * sizeof (LITTLENUM_TYPE));
  litP += pad * sizeof (LITTLENUM_TYPE);

  return NULL;
}
