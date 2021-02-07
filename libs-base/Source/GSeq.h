/* Implementation of composite character sequence functions for GNUSTEP
   Copyright (C) 1999 Free Software Foundation, Inc.
  
   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: May 1999
   Based on code by:  Stevo Crvenkovski <stevo@btinternet.com>
   Date: March 1997
  
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful, 
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
  
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */

/*
 *	Warning - this contains hairy code - handle with care.
 *	The first part of this file contains variable and constant definitions
 *	plus inline function definitions for handling sequences of unicode
 *	characters.  It is bracketed in preprocessor conditionals so that it
 *	is only ever included once.
 *	The second part of the file contains inline function definitions that
 *	are designed to be modified depending on the defined macros at the
 *	point where they are included.  This is meant to be included multiple
 *	times so the same code can be used for NSString, and subclasses.
 */

#ifndef __GSeq_h_GNUSTEP_GSEQ_BASE_INCLUDE
#define __GSeq_h_GNUSTEP_GSEQ_BASE_INCLUDE

/*
 *	Some standard selectors for frequently used methods. Set in NSString
 *      +initialize or the GSString.m setup() function.
 */
static SEL	caiSel = NULL;
static SEL	gcrSel = NULL;
static SEL	ranSel = NULL;

/*
 *	The maximum decompostion level for composite unicode characters.
 */
#define MAXDEC 18

/*
 *	The structure definition for handling a unicode character sequence
 *	for a single character.
 */
typedef	struct {
  unichar	*chars;
  NSUInteger	count;
  NSUInteger	capacity;
  BOOL		normalized;
} GSeqStruct;
typedef	GSeqStruct	*GSeq;

/*
 *	A macro to define a GSeqStruct variable capable of holding a
 *	unicode character sequence of the specified length.
 */
#define	GSEQ_MAKE(BUF, SEQ, LEN) \
    unichar	BUF[LEN * MAXDEC + 1]; \
    GSeqStruct	SEQ = { BUF, LEN, LEN * MAXDEC, 0 }

/*
 * A function to normalize a unicode character sequence ... produces a
 * sequence containing composed characters in a well defined order and
 * with a nul terminator as well as a character count.
 */
static inline void GSeq_normalize(GSeq seq)
{
  NSUInteger	count = seq->count;

  if (count)
    {
      unichar	*source = seq->chars;
      unichar	target[count*MAXDEC+1];
      NSUInteger	base = 0;

      /*
       * Pre-scan ... anything with a code under 0x00C0 is not a decomposable
       * character, so we don't need to expand it.
       * If there are no decomposable characters or composed sequences, the
       * sequence is already normalised and we don't need to make any changes.
       */
      while (base < count)
	{
	  if (source[base] >= 0x00C0)
	    {
	      break;
	    }
	  base++;
	}
      source[count] = (unichar)(0);
      if (base < count)
	{

	  /*
	   * Now expand decomposable characters into the long format.
	   * Use the 'base' value to avoid re-checking characters which have
	   * already been expanded.
	   */
	  while (base < count)
	    {
	      unichar	*spoint = &source[base];
	      unichar	*tpoint = &target[base];
	      NSUInteger	newbase = 0;

	      do
		{
		  unichar	*dpoint = uni_is_decomp(*spoint);

		  if (!dpoint)
		    {
		      *tpoint++ = *spoint;
		    }
		  else
		    {
		      while (*dpoint)
			{
			  *tpoint++ = *dpoint++;
			}
		      if (newbase <= 0)
			{
			  newbase = (spoint - source) + 1;
			}
		    }
		}
	      while (*spoint++);

	      count = tpoint - target;
	      memcpy(&source[base], &target[base], 2*(count - base));
	      source[count] = (unichar)(0);
	      if (newbase > 0)
		{
		  base = newbase;
		}
	      else
		{
		  base = count;
		}
	    }
	  seq->count = count;

	  /*
	   * Now standardise ordering of all composed character sequences.
	   */
	  if (count > 1)
	    {
	      BOOL	notdone = YES;

	      while (notdone)
		{
		  unichar	*first = seq->chars;
		  unichar	*second = first + 1;
		  NSUInteger	i;

		  notdone = NO;
		  for (i = 1; i < count; i++)
		    {
		      if (GSPrivateUniCop(*second))
			{
			  if (GSPrivateUniCop(*first)
			    > GSPrivateUniCop(*second))
			    {
			      unichar	tmp = *first;

			      *first = *second;
			      *second = tmp;
			      notdone = YES;
			    }
			  else if (GSPrivateUniCop(*first)
			    == GSPrivateUniCop(*second))
			    {
			      if (*first > *second)
				{
				   unichar	tmp = *first;

				   *first = *second;
				   *second = tmp;
				   notdone = YES;
				}
			    }
			}
		      first++;
		      second++;
		    }
		}
	    }
	}
      seq->normalized = YES;
    }
}
 
/*
 *	A function to compare two unicode character sequences normalizing if
 *	required.
 */
static inline NSComparisonResult GSeq_compare(GSeq s0, GSeq s1)
{
  NSUInteger	i;
  NSUInteger	end;
  NSUInteger	len0;
  NSUInteger	len1;
  unichar	*c0 = s0->chars;
  unichar	*c1 = s1->chars;

  len0 = s0->count;
  len1 = s1->count;
  if (len0 == len1)
    {
      for (i = 0; i < len1; i++)
	{
	  if (c0[i] != c1[i])
	    {
	      break;
	    }
	}
      if (i == len0)
	{
	  return NSOrderedSame;
	}
    }
  if (s0->normalized == NO)
    GSeq_normalize(s0);
  if (s1->normalized == NO)
    GSeq_normalize(s1);
  len0 = s0->count;
  len1 = s1->count;
  if (len0 < len1)
    end = len0;
  else
    end = len1;
  for (i = 0; i < end; i++)
    {
      if (c0[i] < c1[i])
	return NSOrderedAscending;
      if (c0[i] > c1[i])
	return NSOrderedDescending;
    }
  if (len0 < len1)
    return NSOrderedAscending;
  if (len0 > len1)
    return NSOrderedDescending;
  return NSOrderedSame;
}

static inline void GSeq_lowercase(GSeq seq)
{
  unichar	*s = seq->chars;
  NSUInteger	len = seq->count;
  NSUInteger	i;

  for (i = 0; i < len; i++)
    s[i] = uni_tolower(s[i]);
}

static inline void GSeq_uppercase(GSeq seq)
{
  unichar	*s = seq->chars;
  NSUInteger	len = seq->count;
  NSUInteger	i;

  for (i = 0; i < len; i++)
    s[i] = uni_toupper(s[i]);
}

/*
 * Specify NSString, GSUString or GSCString
 */
#define	GSEQ_NS	0
#define	GSEQ_US	1
#define	GSEQ_CS	2

/*
 * Definitions for bitmask of search options.  These MUST match the
 * enumeration in NSString.h
 */
#define GSEQ_FCLS  3
#define GSEQ_BCLS  7
#define GSEQ_FLS  2
#define GSEQ_BLS 6
#define GSEQ_FCS  1
#define GSEQ_BCS  5
#define GSEQ_FS  0
#define GSEQ_BS  4
#define GSEQ_FCLAS  11
#define GSEQ_BCLAS  15
#define GSEQ_FLAS  10
#define GSEQ_BLAS 14
#define GSEQ_FCAS  9
#define GSEQ_BCAS  13
#define GESQ_FAS  8
#define GSEQ_BAS  12

#endif /* __GSeq_h_GNUSTEP_GSEQ_BASE_INCLUDE */

/*
 * Set up macros for dealing with 'self' on the basis of GSQ_S
 */
#if	GSEQ_S == GSEQ_US
#define	GSEQ_ST	GSStr
#define	GSEQ_SLEN	s->_count
#define	GSEQ_SGETC(I)	s->_contents.u[I]
#define	GSEQ_SGETR(B,R)	memcpy(B, &s->_contents.u[R.location], 2*(R).length)
#define	GSEQ_SRANGE(I)	(*srImp)((id)s, ranSel, I)
#else
#if	GSEQ_S == GSEQ_CS
#define	GSEQ_ST	GSStr
#define	GSEQ_SLEN	s->_count
#define	GSEQ_SGETC(I)	(unichar)s->_contents.c[I]
#define	GSEQ_SGETR(B,R)	( { \
  NSUInteger _lcount = 0; \
  while (_lcount < (R).length) \
    { \
      (B)[_lcount] = (unichar)s->_contents.c[(R).location + _lcount]; \
      _lcount++; \
    } \
} )
#define	GSEQ_SRANGE(I)	(NSRange){I,1}
#else
#define	GSEQ_ST	NSString*
#define	GSEQ_SLEN	[s length]
#define	GSEQ_SGETC(I)	(*scImp)(s, caiSel, I)
#define	GSEQ_SGETR(B,R)	(*sgImp)(s, gcrSel, B, R)
#define	GSEQ_SRANGE(I)	(*srImp)(s, ranSel, I)
#endif
#endif

/*
 * Set up macros for dealing with 'other' string on the basis of GSQ_O
 */
#if	GSEQ_O == GSEQ_US
#define	GSEQ_OT	GSStr
#define	GSEQ_OLEN	o->_count
#define	GSEQ_OGETC(I)	o->_contents.u[I]
#define	GSEQ_OGETR(B,R)	memcpy(B, &o->_contents.u[R.location], 2*(R).length)
#define	GSEQ_ORANGE(I)	(*orImp)((id)o, ranSel, I)
#else
#if	GSEQ_O == GSEQ_CS
#define	GSEQ_OT	GSStr
#define	GSEQ_OLEN	o->_count
#define	GSEQ_OGETC(I)	(unichar)o->_contents.c[I]
#define	GSEQ_OGETR(B,R)	( { \
  NSUInteger _lcount = 0; \
  while (_lcount < (R).length) \
    { \
      (B)[_lcount] = (unichar)o->_contents.c[(R).location + _lcount]; \
      _lcount++; \
    } \
} )
#define	GSEQ_ORANGE(I)	(NSRange){I,1}
#else
#define	GSEQ_OT	NSString*
#define	GSEQ_OLEN	[o length]
#define	GSEQ_OGETC(I)	(*ocImp)(o, caiSel, I)
#define	GSEQ_OGETR(B,R)	(*ogImp)(o, gcrSel, B, R)
#define	GSEQ_ORANGE(I)	(*orImp)(o, ranSel, I)
#endif
#endif

/*
 * If a string comparison function is required, implement it.
 */
#ifdef	GSEQ_STRCOMP
static NSComparisonResult
GSEQ_STRCOMP(NSString *ss, NSString *os, NSUInteger mask, NSRange aRange)
{
  GSEQ_ST	s = (GSEQ_ST)ss;
  GSEQ_OT	o = (GSEQ_OT)os;
  NSUInteger	oLength;			/* Length of other.	*/

#if	0
  /* Range should be checked in calling code */
  if (aRange.location > GSEQ_SLEN)
    [NSException raise: NSRangeException format: @"Invalid location."];
  if (aRange.length > (GSEQ_SLEN - aRange.location))
    [NSException raise: NSRangeException format: @"Invalid location+length."];
#endif

  oLength = GSEQ_OLEN;
  if (aRange.length == 0)
    {
      if (oLength == 0)
	{
	  return NSOrderedSame;
	}
      return NSOrderedAscending;
    }
  else if (oLength == 0)
    {
      return NSOrderedDescending;
    }

  if (mask & NSLiteralSearch)
    {
      NSUInteger	i;
      NSUInteger	sLen = aRange.length;
      NSUInteger	oLen = oLength;
      NSUInteger	end;
#if	GSEQ_S == GSEQ_NS
      void	(*sgImp)(NSString*, SEL, unichar*, NSRange);
      unichar	sBuf[sLen];
#else
#if	GSEQ_S == GSEQ_US
      unichar	*sBuf;
#else
      unsigned char	*sBuf;
#endif
#endif
#if	GSEQ_O == GSEQ_NS
      void	(*ogImp)(NSString*, SEL, unichar*, NSRange);
      unichar	oBuf[oLen];
#else
#if	GSEQ_O == GSEQ_US
      unichar	*oBuf;
#else
      unsigned char	*oBuf;
#endif
#endif

#if	GSEQ_S == GSEQ_NS
      sgImp = (void (*)(NSString*,SEL,unichar*,NSRange))
	[(id)s methodForSelector: gcrSel];
      GSEQ_SGETR(sBuf, aRange);
#else
#if	GSEQ_S == GSEQ_CS
      sBuf = &s->_contents.c[aRange.location];
#else
      sBuf = &s->_contents.u[aRange.location];
#endif
#endif
#if	GSEQ_O == GSEQ_NS
      ogImp = (void (*)(NSString*,SEL,unichar*,NSRange))
	[(id)o methodForSelector: gcrSel];
      GSEQ_OGETR(oBuf, NSMakeRange(0, oLen));
#else
#if	GSEQ_O == GSEQ_CS
      oBuf = o->_contents.c;
#else
      oBuf = o->_contents.u;
#endif
#endif

      if (oLen < sLen)
	end = oLen;
      else
	end = sLen;

      if (mask & NSCaseInsensitiveSearch)
	{
#if	GSEQ_O == GSEQ_CS || GSEQ_S == GSEQ_CS
          if (GSPrivateDefaultCStringEncoding() == NSISOLatin1StringEncoding)
            {
              /* Using latin1 internally, rather than native encoding,
               * so we can't use native tolower() function.
               */
              for (i = 0; i < end; i++)
                {
                  unichar	c1 = uni_tolower((unichar)sBuf[i]);
                  unichar	c2 = uni_tolower((unichar)oBuf[i]);

                  if (c1 < c2)
                    return NSOrderedAscending;
                  if (c1 > c2)
                    return NSOrderedDescending;
                }
            }
          else
            {
              /* We are not using latin1 encoding internally, so we trust
               * that the internal encoding matches the native encoding
               * and the native tolower() function will work.
               */
              for (i = 0; i < end; i++)
                {
#if	GSEQ_S == GSEQ_CS
                  unichar	c1 = tolower(sBuf[i]);
#else
                  unichar	c1 = uni_tolower((unichar)sBuf[i]);
#endif
#if	GSEQ_O == GSEQ_CS
                  unichar	c2 = tolower(oBuf[i]);
#else
                  unichar       c2 = uni_tolower((unichar)oBuf[i]);
#endif

                  if (c1 < c2)
                    return NSOrderedAscending;
                  if (c1 > c2)
                    return NSOrderedDescending;
                }
            }
#else
	  for (i = 0; i < end; i++)
	    {
	      unichar	c1 = uni_tolower((unichar)sBuf[i]);
	      unichar	c2 = uni_tolower((unichar)oBuf[i]);

	      if (c1 < c2)
		return NSOrderedAscending;
	      if (c1 > c2)
		return NSOrderedDescending;
	    }
#endif
	}
      else
	{
	  for (i = 0; i < end; i++)
	    {
#if	GSEQ_O == GSEQ_CS && GSEQ_S == GSEQ_CS
	      if (sBuf[i] < oBuf[i])
		return NSOrderedAscending;
	      if (sBuf[i] > oBuf[i])
		return NSOrderedDescending;
#else
	      if ((unichar)sBuf[i] < (unichar)oBuf[i])
		return NSOrderedAscending;
	      if ((unichar)sBuf[i] > (unichar)oBuf[i])
		return NSOrderedDescending;
#endif
	    }
	}
      if (sLen > oLen)
	return NSOrderedDescending;
      else if (sLen < oLen)
	return NSOrderedAscending;
      else
	return NSOrderedSame;
    }
  else
    {
      NSUInteger		start = aRange.location;
      NSUInteger		end = start + aRange.length;
      NSUInteger		sLength = GSEQ_SLEN;
      NSUInteger		sCount = start;
      NSUInteger		oCount = 0;
      NSComparisonResult result;
#if	GSEQ_S == GSEQ_NS || GSEQ_S == GSEQ_US
      NSRange		(*srImp)(NSString*, SEL, NSUInteger);
#endif
#if	GSEQ_O == GSEQ_NS || GSEQ_O == GSEQ_US
      NSRange		(*orImp)(NSString*, SEL, NSUInteger);
#endif
#if	GSEQ_S == GSEQ_NS
      void		(*sgImp)(NSString*, SEL, unichar*, NSRange);
#endif
#if	GSEQ_O == GSEQ_NS
      void		(*ogImp)(NSString*, SEL, unichar*, NSRange);
#endif

#if	GSEQ_S == GSEQ_NS || GSEQ_S == GSEQ_US
      srImp = (NSRange (*)(NSString*, SEL, NSUInteger))
	[(id)s methodForSelector: ranSel];
#endif
#if	GSEQ_O == GSEQ_NS || GSEQ_O == GSEQ_US
      orImp = (NSRange (*)(NSString*, SEL, NSUInteger))
	[(id)o methodForSelector: ranSel];
#endif
#if	GSEQ_S == GSEQ_NS
      sgImp = (void (*)(NSString*, SEL, unichar*, NSRange))
	[(id)s methodForSelector: gcrSel];
#endif
#if	GSEQ_O == GSEQ_NS
      ogImp = (void (*)(NSString*, SEL, unichar*, NSRange))
	[(id)o methodForSelector: gcrSel];
#endif

      while (sCount < end)
	{
	  if (oCount >= oLength)
	    {
	      return NSOrderedDescending;
	    }
	  else if (sCount >= sLength)
	    {
	      return NSOrderedAscending;
	    }
	  else
	    {
	      NSRange	sRange = GSEQ_SRANGE(sCount);
	      NSRange	oRange = GSEQ_ORANGE(oCount);
	      GSEQ_MAKE(sBuf, sSeq, sRange.length);
	      GSEQ_MAKE(oBuf, oSeq, oRange.length);

	      GSEQ_SGETR(sBuf, sRange);
	      GSEQ_OGETR(oBuf, oRange);

	      result = GSeq_compare(&sSeq, &oSeq);

	      if (result != NSOrderedSame)
		{
		  if (mask & NSCaseInsensitiveSearch)
		    {
		      GSeq_lowercase(&oSeq);
		      GSeq_lowercase(&sSeq);
		      result = GSeq_compare(&sSeq, &oSeq);
		      if (result != NSOrderedSame)
			{
			  return result;
			}
		    }
		  else
		    {
		      return result;
		    }
		}

	      sCount += sRange.length;
	      oCount += oRange.length;
	    }
	}
      if (oCount < oLength)
	return NSOrderedAscending;
      return NSOrderedSame;
   }
}
#undef	GSEQ_STRCOMP
#endif

/*
 * If a string search function is required, implement it.
 */
#ifdef	GSEQ_STRRANGE
static NSRange
GSEQ_STRRANGE(NSString *ss, NSString *os, NSUInteger mask, NSRange aRange)
{
  GSEQ_ST	s = (GSEQ_ST)ss;
  GSEQ_OT	o = (GSEQ_OT)os;
  NSUInteger    rangeEnd = NSMaxRange(aRange);
  NSUInteger	myIndex;
  NSUInteger	myEndIndex;
  NSUInteger	strLength;
#if	GSEQ_S == GSEQ_NS
  unichar	(*scImp)(NSString*, SEL, NSUInteger);
  void		(*sgImp)(NSString*, SEL, unichar*, NSRange);
#endif
#if	GSEQ_O == GSEQ_NS
  unichar	(*ocImp)(NSString*, SEL, NSUInteger);
  void		(*ogImp)(NSString*, SEL, unichar*, NSRange);
#endif
#if	GSEQ_S == GSEQ_NS || GSEQ_S == GSEQ_US
  NSRange	(*srImp)(NSString*, SEL, NSUInteger);
#endif
#if	GSEQ_O == GSEQ_NS || GSEQ_O == GSEQ_US
  NSRange	(*orImp)(NSString*, SEL, NSUInteger);
#endif
  
#if	0
  /* Check that the search range is reasonable */
  NSUInteger	myLength = GSEQ_SLEN;

  /* Range should be checked in calling code */
  if (aRange.location > myLength)
    [NSException raise: NSRangeException format: @"Invalid location."];
  if (aRange.length > (myLength - aRange.location))
    [NSException raise: NSRangeException format: @"Invalid location+length."];
#endif

  /* Ensure the string can be found */
  strLength = GSEQ_OLEN;
  if (strLength > aRange.length || strLength == 0)
    return (NSRange){NSNotFound, 0};

  /*
   * Cache method implementations for getting characters and ranges
   */
#if	GSEQ_S == GSEQ_NS
  scImp = (unichar (*)(NSString*,SEL,NSUInteger))
    [(id)s methodForSelector: caiSel];
  sgImp = (void (*)(NSString*,SEL,unichar*,NSRange))
    [(id)s methodForSelector: gcrSel];
#endif
#if	GSEQ_O == GSEQ_NS
  ocImp = (unichar (*)(NSString*,SEL,NSUInteger))
    [(id)o methodForSelector: caiSel];
  ogImp = (void (*)(NSString*,SEL,unichar*,NSRange))
    [(id)o methodForSelector: gcrSel];
#endif
#if	GSEQ_S == GSEQ_NS || GSEQ_S == GSEQ_US
  srImp = (NSRange (*)(NSString*,SEL,NSUInteger))
    [(id)s methodForSelector: ranSel];
#endif
#if	GSEQ_O == GSEQ_NS || GSEQ_O == GSEQ_US
  orImp = (NSRange (*)(NSString*,SEL,NSUInteger))
    [(id)o methodForSelector: ranSel];
#endif

  switch (mask)
    {
      case GSEQ_FCLS: 
      case GSEQ_FCLAS: 
	{
	  unichar	strFirstCharacter = GSEQ_OGETC(0);

	  myIndex = aRange.location;
	  myEndIndex = rangeEnd - strLength;

	  if (mask & NSAnchoredSearch)
	    myEndIndex = myIndex;

	  for (;;)
	    {
	      NSUInteger	i = 1;
	      unichar	myCharacter = GSEQ_SGETC(myIndex);
	      unichar	strCharacter = strFirstCharacter;

              for (;;)
		{
		  if ((myCharacter != strCharacter) &&
		      ((uni_tolower(myCharacter) != uni_tolower(strCharacter))))
		    break;
		  if (i == strLength)
		    return (NSRange){myIndex, strLength};
                  if (myIndex + i >= rangeEnd)
                    break;
		  myCharacter = GSEQ_SGETC(myIndex + i);
		  strCharacter = GSEQ_OGETC(i);
		  i++;
		}
	      if (myIndex == myEndIndex)
		break;
	      myIndex++;
	    }
	  return (NSRange){NSNotFound, 0};
	}

      case GSEQ_BCLS: 
      case GSEQ_BCLAS: 
	{
	  unichar	strFirstCharacter = GSEQ_OGETC(0);

	  myIndex = rangeEnd - strLength;
	  myEndIndex = aRange.location;

	  if (mask & NSAnchoredSearch)
	    myEndIndex = myIndex;

	  for (;;)
	    {
	      NSUInteger	i = 1;
	      unichar	myCharacter = GSEQ_SGETC(myIndex);
	      unichar	strCharacter = strFirstCharacter;

	      for (;;)
		{
		  if ((myCharacter != strCharacter) &&
		      ((uni_tolower(myCharacter) != uni_tolower(strCharacter))))
		    break;
		  if (i == strLength)
		    return (NSRange){myIndex, strLength};
                  if (myIndex + i >= rangeEnd)
                    break;
		  myCharacter = GSEQ_SGETC(myIndex + i);
		  strCharacter = GSEQ_OGETC(i);
		  i++;
		}
	      if (myIndex-- == myEndIndex)
		break;
	    }
	  return (NSRange){NSNotFound, 0};
	}

      case GSEQ_FLS: 
      case GSEQ_FLAS: 
	{
	  unichar	strFirstCharacter = GSEQ_OGETC(0);

	  myIndex = aRange.location;
	  myEndIndex = rangeEnd - strLength;

	  if (mask & NSAnchoredSearch)
	    myEndIndex = myIndex;

	  for (;;)
	    {
	      NSUInteger	i = 1;
	      unichar	myCharacter = GSEQ_SGETC(myIndex);
	      unichar	strCharacter = strFirstCharacter;

              for (;;)
		{
		  if (myCharacter != strCharacter)
		    break;
		  if (i == strLength)
		    return (NSRange){myIndex, strLength};
                  if (myIndex + i >= rangeEnd)
                    break;
		  myCharacter = GSEQ_SGETC(myIndex + i);
		  strCharacter = GSEQ_OGETC(i);
		  i++;
		}
	      if (myIndex++ == myEndIndex)
		break;
	    }
	  return (NSRange){NSNotFound, 0};
	}

      case GSEQ_BLS: 
      case GSEQ_BLAS: 
	{
	  unichar	strFirstCharacter = GSEQ_OGETC(0);

	  myIndex = rangeEnd - strLength;
	  myEndIndex = aRange.location;

	  if (mask & NSAnchoredSearch)
	    myEndIndex = myIndex;

	  for (;;)
	    {
	      NSUInteger	i = 1;
	      unichar	myCharacter = GSEQ_SGETC(myIndex);
	      unichar	strCharacter = strFirstCharacter;

	      for (;;)
		{
		  if (myCharacter != strCharacter)
		    break;
		  if (i == strLength)
		    return (NSRange){myIndex, strLength};
                  if (myIndex + i >= rangeEnd)
                    break;
		  myCharacter = GSEQ_SGETC(myIndex + i);
		  strCharacter = GSEQ_OGETC(i);
		  i++;
		}
	      if (myIndex-- == myEndIndex)
		break;
	    }
	  return (NSRange){NSNotFound, 0};
	}

      case GSEQ_FCS: 
      case GSEQ_FCAS: 
	{
	  NSUInteger	strBaseLength = [(NSString*)o _baseLength];
	  NSRange	iRange;

	  myIndex = aRange.location;
	  myEndIndex = rangeEnd - strBaseLength;

	  if (mask & NSAnchoredSearch)
	    myEndIndex = myIndex;

	  iRange = GSEQ_ORANGE(0);
	  if (iRange.length)
	    {
	      GSEQ_MAKE(iBuf, iSeq, iRange.length);

	      GSEQ_OGETR(iBuf, iRange);
	      GSeq_lowercase(&iSeq);

	      for (;;)
		{
		  NSRange	sRange = GSEQ_SRANGE(myIndex);
		  GSEQ_MAKE(sBuf, sSeq, sRange.length);

		  GSEQ_SGETR(sBuf, sRange);
		  GSeq_lowercase(&sSeq);

		  if (GSeq_compare(&iSeq, &sSeq) == NSOrderedSame)
		    {
		      NSUInteger	myCount = sRange.length;
		      NSUInteger	strCount = iRange.length;

		      if (strCount >= strLength)
			{
			  return (NSRange){myIndex, myCount};
			}
                      while (myIndex + myCount < rangeEnd)
			{
			  NSRange	r0 = GSEQ_SRANGE(myIndex + myCount);
			  GSEQ_MAKE(b0, s0, r0.length);
			  NSRange	r1 = GSEQ_ORANGE(strCount);
			  GSEQ_MAKE(b1, s1, r1.length);

			  GSEQ_SGETR(b0, r0);
			  GSEQ_OGETR(b1, r1);

			  if (GSeq_compare(&s0, &s1) != NSOrderedSame)
			    {
			      GSeq_lowercase(&s0);
			      GSeq_lowercase(&s1);
			      if (GSeq_compare(&s0, &s1) != NSOrderedSame)
				{
				  break;
				}
			    }
			  myCount += r0.length;
			  strCount += r1.length;
			  if (strCount >= strLength)
			    {
			      return (NSRange){myIndex, myCount};
			    }
			}
		    }
		  myIndex += sRange.length;
		  if (myIndex > myEndIndex)
		    break;
		}
	    }
	  return (NSRange){NSNotFound, 0};
	}

      case GSEQ_BCS: 
      case GSEQ_BCAS: 
	{
	  NSUInteger	strBaseLength = [(NSString*)o _baseLength];
	  NSRange	iRange;

	  myIndex = rangeEnd - strBaseLength;
	  myEndIndex = aRange.location;

	  if (mask & NSAnchoredSearch)
	    myEndIndex = myIndex;

	  iRange = GSEQ_ORANGE(0);
	  if (iRange.length)
	    {
	      GSEQ_MAKE(iBuf, iSeq, iRange.length);

	      GSEQ_OGETR(iBuf, iRange);
	      GSeq_lowercase(&iSeq);

	      for (;;)
		{
		  NSRange	sRange = GSEQ_SRANGE(myIndex);
		  GSEQ_MAKE(sBuf, sSeq, sRange.length);

		  GSEQ_SGETR(sBuf, sRange);
		  GSeq_lowercase(&sSeq);

		  if (GSeq_compare(&iSeq, &sSeq) == NSOrderedSame)
		    {
		      NSUInteger	myCount = sRange.length;
		      NSUInteger	strCount = iRange.length;

		      if (strCount >= strLength)
			{
			  return (NSRange){myIndex, myCount};
			}
		      while (myIndex + myCount < rangeEnd)
			{
			  NSRange	r0 = GSEQ_SRANGE(myIndex + myCount);
			  GSEQ_MAKE(b0, s0, r0.length);
			  NSRange	r1 = GSEQ_ORANGE(strCount);
			  GSEQ_MAKE(b1, s1, r1.length);

			  GSEQ_SGETR(b0, r0);
			  GSEQ_OGETR(b1, r1);

			  if (GSeq_compare(&s0, &s1) != NSOrderedSame)
			    {
			      GSeq_lowercase(&s0);
			      GSeq_lowercase(&s1);
			      if (GSeq_compare(&s0, &s1) != NSOrderedSame)
				{
				  break;
				}
			    }
			  myCount += r0.length;
			  strCount += r1.length;
			  if (strCount >= strLength)
			    {
			      return (NSRange){myIndex, myCount};
			    }
			}
		    }
		  if (myIndex-- <= myEndIndex)
		    break;
		  while (uni_isnonsp(GSEQ_SGETC(myIndex))
		    && (myIndex > 0))
		    myIndex--;
		}
	    }
	  return (NSRange){NSNotFound, 0};
	}

      case GSEQ_BS: 
      case GSEQ_BAS: 
	{
	  NSUInteger	strBaseLength = [(NSString*)o _baseLength];
	  NSRange	iRange;

	  myIndex = rangeEnd - strBaseLength;
	  myEndIndex = aRange.location;

	  if (mask & NSAnchoredSearch)
	    myEndIndex = myIndex;

	  iRange = GSEQ_ORANGE(0);
	  if (iRange.length)
	    {
	      GSEQ_MAKE(iBuf, iSeq, iRange.length);

	      GSEQ_OGETR(iBuf, iRange);

	      for (;;)
		{
		  NSRange	sRange = GSEQ_SRANGE(myIndex);
		  GSEQ_MAKE(sBuf, sSeq, sRange.length);

		  GSEQ_SGETR(sBuf, sRange);

		  if (GSeq_compare(&iSeq, &sSeq) == NSOrderedSame)
		    {
		      NSUInteger	myCount = sRange.length;
		      NSUInteger	strCount = iRange.length;

		      if (strCount >= strLength)
			{
			  return (NSRange){myIndex, myCount};
			}
		      while (myIndex + myCount < rangeEnd)
			{
			  NSRange	r0 = GSEQ_SRANGE(myIndex + myCount);
			  GSEQ_MAKE(b0, s0, r0.length);
			  NSRange	r1 = GSEQ_ORANGE(strCount);
			  GSEQ_MAKE(b1, s1, r1.length);

			  GSEQ_SGETR(b0, r0);
			  GSEQ_OGETR(b1, r1);

			  if (GSeq_compare(&s0, &s1) != NSOrderedSame)
			    {
			      break;
			    }
			  myCount += r0.length;
			  strCount += r1.length;
			  if (strCount >= strLength)
			    {
			      return (NSRange){myIndex, myCount};
			    }
			}
		    }
		  if (myIndex-- <= myEndIndex)
		    break;
		  while (uni_isnonsp(GSEQ_SGETC(myIndex))
		    && (myIndex > 0))
                    {
                      myIndex--;
                    }
		}
	    }
	  return (NSRange){NSNotFound, 0};
	}

      case GSEQ_FS: 
      case GESQ_FAS: 
      default: 
	{
	  NSUInteger	strBaseLength = [(NSString*)o _baseLength];
	  NSRange	iRange;

	  myIndex = aRange.location;
	  myEndIndex = rangeEnd - strBaseLength;

	  if (mask & NSAnchoredSearch)
	    myEndIndex = myIndex;

	  iRange = GSEQ_ORANGE(0);
	  if (iRange.length)
	    {
	      GSEQ_MAKE(iBuf, iSeq, iRange.length);

	      GSEQ_OGETR(iBuf, iRange);

              for (;;)
		{
		  NSRange	sRange = GSEQ_SRANGE(myIndex);
		  GSEQ_MAKE(sBuf, sSeq, sRange.length);

		  GSEQ_SGETR(sBuf, sRange);

		  if (GSeq_compare(&iSeq, &sSeq) == NSOrderedSame)
		    {
		      NSUInteger	myCount = sRange.length;
		      NSUInteger	strCount = iRange.length;

		      if (strCount >= strLength)
			{
			  return (NSRange){myIndex, myCount};
			}
		      while (myIndex + myCount < rangeEnd)
			{
			  NSRange	r0 = GSEQ_SRANGE(myIndex + myCount);
			  GSEQ_MAKE(b0, s0, r0.length);
			  NSRange	r1 = GSEQ_ORANGE(strCount);
			  GSEQ_MAKE(b1, s1, r1.length);

			  GSEQ_SGETR(b0, r0);
			  GSEQ_OGETR(b1, r1);

			  if (GSeq_compare(&s0, &s1) != NSOrderedSame)
			    {
			      break;
			    }
			  myCount += r0.length;
			  strCount += r1.length;
			  if (strCount >= strLength)
			    {
			      return (NSRange){myIndex, myCount};
			    }
			}
		    }
		  myIndex += sRange.length;
		  if (myIndex > myEndIndex)
		    break;
		}
	    }
	  return (NSRange){NSNotFound, 0};
	}
    }
}
#undef	GSEQ_STRRANGE
#endif

/*
 * Clean up macro namespace
 */
#ifdef	GSEQ_S
#undef	GSEQ_SLEN
#undef	GSEQ_SGETC
#undef	GSEQ_SGETR
#undef	GSEQ_SRANGE
#undef	GSEQ_ST
#undef	GSEQ_S
#endif

#ifdef	GSEQ_O
#undef	GSEQ_OLEN
#undef	GSEQ_OGETC
#undef	GSEQ_OGETR
#undef	GSEQ_ORANGE
#undef	GSEQ_OT
#undef	GSEQ_O
#endif

