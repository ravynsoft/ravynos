/*------------------------------------------------------------*/
/* filename -       tvalidatcpp                               */
/*                                                            */
/* function(s)                                                */
/*                  TValidator member functions               */
/*                  TFilterValidator member functions         */
/*                  TRangeValidator member functions          */
/*                  TLookupValidator member functions         */
/*                  TStringLookupValidator member functions   */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_MsgBox
#define Uses_TStreamable
#define Uses_TValidator
#define Uses_TPXPictureValidator
#define Uses_TFilterValidator
#define Uses_TRangeValidator
#define Uses_TLookupValidator
#define Uses_TStringLookupValidator
#define Uses_ipstream
#define Uses_opstream
#include <tvision/tv.h>
#include <string.h>
#include <stdio.h>

static inline char uppercase(char c)
{
    return (('a' <= c) && (c <= 'z')) ? c + 'A'-'a' : c;
}

// TValidator

TValidator::TValidator() noexcept
{
  status = 0;
  options = 0;
}

#if !defined(NO_STREAMABLE)

#pragma argsused
TValidator::TValidator( StreamableInit s) noexcept
{
}

void* TValidator::read(ipstream& is)
{
  is >> options;
  status = 0;

  return this;
}

#endif

void TValidator::error()
{
}

#pragma warn -par
Boolean TValidator::isValidInput(char* s, Boolean suppressFill)
{
  return True;
}

#pragma warn -par
Boolean TValidator::isValid(const char* s)
{
  return True;
}

#if !defined(NO_STREAMABLE)

void TValidator::write(opstream& os)
{
    os << options;
}

#endif

#pragma warn -par
ushort TValidator::transfer(char* s, void* buffer, TVTransfer flag)
{
  return 0;
}

Boolean TValidator::validate(const char* s)
{
  if (!isValid(s))
  {
    error();
    return False;
  }
  return True;
}

// TPXPictureValidator

TPXPictureValidator::TPXPictureValidator(TStringView aPic, Boolean autoFill)
    : TValidator()
{
  pic = newStr(aPic);
  if ( autoFill )
       options |= voFill;

  char s[] = "";
  if (picture(s, False) != prEmpty)
    status = vsSyntax;
}

#if !defined(NO_STREAMABLE)

TPXPictureValidator::TPXPictureValidator( StreamableInit s) noexcept : TValidator(s)
{
}

void TPXPictureValidator::write(opstream& os)
{
  TValidator::write(os);
  os.writeString(pic);
}

void* TPXPictureValidator::read( ipstream& is )
{
  TValidator::read(is);
  pic = is.readString();
  index = jndex = 0;

  return this;
}

#endif

TPXPictureValidator::~TPXPictureValidator()
{
  delete[] pic;
}

void TPXPictureValidator::error()
{
  messageBox(mfError | mfOKButton, errorMsg, pic);
}

Boolean TPXPictureValidator::isValidInput(char* s, Boolean suppressFill)
{
  Boolean doFill = Boolean(((options&voFill)!=0) && !suppressFill);

  return Boolean((pic==0) || (picture( (char*)s, doFill) != prError));
}

Boolean TPXPictureValidator::isValid(const char* s)
{
  char str[256];

  strcpy(str, s);
  return Boolean((pic == 0) || (picture(str, False) == prComplete));
}

Boolean isNumber(char ch)
{
    return Boolean(('0' <= ch) && (ch <= '9'));
}

Boolean isLetter(char ch)
{
    ch &= 0xdf;
    return Boolean(('A' <= ch) && (ch <= 'Z'));
}

Boolean isSpecial(char ch, const char* special)
{
    if (memchr(special, ch, strlen(special)) != 0)
        return True;
    else
        return False;
}

/*
  This helper function will be used for a persistant TInputLine mask.
  It will be moved to TINPUTLI.CPP when needed.
*/
uchar numChar(char ch, const char* s)
{
    int count;
    uchar n;

    for (count = strlen(s), n = 0; count; count--, s++)
        if (*s == ch)
            n++;
    return n;
}

Boolean isComplete(TPicResult result)
{
  return  Boolean((result == prComplete) || (result == prAmbiguous));
}

Boolean isIncomplete(TPicResult result)
{
  return Boolean( (result == prIncomplete) || (result == prIncompNoFill) );
}


// TPXPictureValidator members

// Consume input
void TPXPictureValidator::consume(char ch, char* input)
{
      input[jndex] = ch;
      index++;
      jndex++;
}

// Skip a character or a picture group

void TPXPictureValidator::toGroupEnd(int& i, int termCh)
{
    int  brkLevel, brcLevel;

      brkLevel = 0;
      brcLevel = 0;
      do {
        if (i == termCh)
            return;
        else
          switch (pic[i])
          {
          case  '[': brkLevel++; break;
          case  ']': brkLevel--; break;
          case  '{': brcLevel++; break;
          case  '}': brcLevel--; break;
          case  ';': i++; break;
          }
        i++;
      } while (! ((brkLevel == 0) && (brcLevel == 0)));
}

// Find the a comma separator
Boolean TPXPictureValidator::skipToComma(int termCh)
{
      do {
          toGroupEnd(index, termCh); 
      } while (! ( (index == termCh) || (pic[index] == ',')));

      if (pic[index] == ',')
          index++;
      return Boolean(index < termCh);
}

// Calclate the end of a group 
int TPXPictureValidator::calcTerm(int termCh)
{
      int k = index;
      toGroupEnd(k, termCh);
      return k;
}

// The next group is repeated X times }
TPicResult TPXPictureValidator::iteration(char* input, int inTerm)
{
      int itr, k, l;
      TPicResult rslt;
      int termCh;

      itr = 0;
      rslt = prError;

      index++;  // Skip '*'

      // Retrieve number

      while (isNumber(pic[index]))
      {
        itr = itr * 10 + (pic[index] - '0');
        index++;
      }

      k = index;
      termCh = calcTerm(inTerm);

      // If Itr is 0 allow any number, otherwise enforce the number
      if (itr != 0)
      {
        for (l = 1; l <= itr; l++)
        {
          index = k;
          rslt = process(input,termCh);
          if ( ! isComplete(rslt))
      {
            // Empty means incomplete since all are required
            if (rslt == prEmpty)
            rslt = prIncomplete;

        return rslt;
      }
        }
      }
      else
      {
        do {
          index = k;
          rslt = process(input, termCh);
        } while (rslt == prComplete);

        if ((rslt == prEmpty) || (rslt == prError))
        {
          index++;
          rslt = prAmbiguous;
        }
      }
      index = termCh;

      return rslt;
}

// Process a picture group
TPicResult TPXPictureValidator::group(char* input, int inTerm)
{

      TPicResult rslt;
      int termCh;

      termCh = calcTerm(inTerm);
      index++;
      rslt = process(input, termCh - 1);

      if (! isIncomplete(rslt))  
          index = termCh;

      return rslt;
}


TPicResult TPXPictureValidator::checkComplete(TPicResult rslt, int termCh)
{
    int j = index;
    Boolean status=True;

    if (isIncomplete(rslt))
    {
        // Skip optional pieces
        while (status)
          switch (pic[j])
          {
          case '[': 
             toGroupEnd(j, termCh);
             break;
          case  '*':
             if (! isNumber(pic[j + 1]))
                 j++;
             toGroupEnd(j, termCh);
             break;

          default:
              status = False;
          }

        if (j == termCh)
          rslt = prAmbiguous;
    }

    return rslt;
}


TPicResult TPXPictureValidator::scan(char* input, int termCh)
{
    char ch;
    TPicResult rslt, rScan;

    rScan = prError;
    rslt = prEmpty;

    while ( (index != termCh) && (pic[index] != ','))
    {
        if (jndex >= (int) strlen(input))
        return checkComplete(rslt, termCh);

        ch = input[jndex];
        switch (pic[index])
    {
        case  '#': 
        if (! isNumber(ch)) 
            return prError;
        else 
            consume(ch, input);
        break;
        case  '?': 
        if (! isLetter(ch))
            return prError;
        else 
            consume(ch, input);
        break;
        case  '&': 
        if (! isLetter(ch))
            return prError;
                else 
            consume(uppercase(ch), input);
        break;
        case  '!': 
        consume(uppercase(ch), input);
        break;
        case  '@':
        consume(ch, input);
        break;
        case  '*':
    
              rslt = iteration(input,termCh);
              if (! isComplete(rslt))
                  return rslt;

              if (rslt == prError) 
              rslt = prAmbiguous;
          break;

        case '{':

              rslt = group(input, termCh);
              if (! isComplete(rslt))
              return rslt;

          break;
        case '[':
            
              rslt = group(input, termCh);
              if (isIncomplete(rslt))
              return rslt;
              if (rslt == prError)
              rslt = prAmbiguous;

          break;

        default:

          if (pic[index] == ';')
          index++;
          if (uppercase(pic[index]) != uppercase(ch))
          {
            if (ch == ' ')
             ch = pic[index];
            else 
            return rScan;
          }

          consume(pic[index], input);
    }

        if (rslt == prAmbiguous)
          rslt = prIncompNoFill;
        else
          rslt = prIncomplete;
    }

      if (rslt == prIncompNoFill)
        return prAmbiguous;
      else
        return prComplete;
}

TPicResult TPXPictureValidator::process(char* input, int termCh)
{

   TPicResult rslt, rProcess;
   Boolean incomp;
   int oldI, oldJ, incompJ=0, incompI=0;

   incomp = False;
   oldI = index;
   oldJ = jndex;
   do {
      rslt = scan(input, termCh);

      // Only accept completes if they make it farther in the input
      //   stream from the last incomplete

      if ( (rslt == prComplete) && incomp && (jndex < incompJ))
      {
        rslt = prIncomplete;
        jndex = incompJ;
      }

      if ((rslt == prError) || (rslt == prIncomplete))
      {
        rProcess = rslt;

        if (! incomp &&  (rslt == prIncomplete) )
        {
          incomp  = True;
          incompI = index;
          incompJ = jndex;
        }
        index = oldI;
        jndex = oldJ;
        if (! skipToComma(termCh))
        {
          if ( incomp )
          {
            rProcess = prIncomplete;
            index = incompI;
            jndex = incompJ;
          }
          return rProcess;
        }
        oldI = index;
      }
   } while (!((rslt != prError) && (rslt != prIncomplete)));

   if ((rslt == prComplete) && incomp)
      return prAmbiguous;
   else
      return rslt;
}

Boolean TPXPictureValidator::syntaxCheck()
{

    int i, len;
    int brkLevel, brcLevel;

    if (!pic || (strlen(pic) == 0))
        return False;

    if (pic[strlen(pic)-1] == ';')
        return False;

    i = 0;
    brkLevel = 0;
    brcLevel = 0;

    len = strlen(pic);
    while (i < len)
    {
      switch (pic[i])
      {
      case '[': brkLevel++; break;
      case ']': brkLevel--; break;
      case '{': brcLevel++; break;
      case '}': brcLevel--; break;
      case ';': i++;        break;
      }
      i++;
    }

    return Boolean( (brkLevel == 0) && (brcLevel == 0) );
}

TPicResult TPXPictureValidator::picture(char* input, Boolean autoFill)
{

  Boolean reprocess;
  TPicResult rslt;

  if (!syntaxCheck())
      return prSyntax;

  if (!input || strlen(input)==0)
       return prEmpty;

  jndex = 0;
  index = 0;

  rslt = process(input, strlen(pic));

  if ((rslt != prError) && (jndex < (int) strlen(input)))
    rslt = prError;

  if ((rslt == prIncomplete) && autoFill)
  {
    reprocess = False;

    while ((index < (int) strlen(pic)) && !isSpecial(pic[index], "#?&!@*{}[],"))
    {
      if (pic[index] == ';')
          index++;
      int end = strlen(input);
      input[end] = pic[index];
      input[end+1] = 0;
      index++;
      reprocess = True;
    }

    jndex = 0;
    index = 0;
    if (reprocess)
      rslt = process(input, strlen(pic));
  }

  if (rslt == prAmbiguous)
    return prComplete;
  else if (rslt == prIncompNoFill)
    return prIncomplete;
  else
    return rslt;
}


// TFilterValidator

TFilterValidator::TFilterValidator(TStringView aValidChars) noexcept
{
  validChars = newStr(aValidChars);
}

#if !defined(NO_STREAMABLE)

TFilterValidator::TFilterValidator( StreamableInit s) noexcept : TValidator(s)
{
}

#endif

TFilterValidator::~TFilterValidator()
{
    delete[] validChars;
}

#if !defined(NO_STREAMABLE)

void TFilterValidator::write(opstream& os)
{
  TValidator::write(os);
  os.writeString(validChars);
}

void* TFilterValidator::read(ipstream& is)
{
  TValidator::read(is);
  validChars = is.readString();
  return this;
}

#endif

Boolean TFilterValidator::isValid(const char* s)
{
  return Boolean(strspn(s, validChars) == strlen(s));
}

Boolean TFilterValidator::isValidInput(char* s, Boolean suppressFill)
{
  return Boolean(strspn(s, validChars) == strlen(s));
}

void TFilterValidator::error()
{
  messageBox(mfError | mfOKButton, errorMsg );
}

// TRangeValidator

TRangeValidator::TRangeValidator( int32_t aMin, int32_t aMax ) noexcept :
    TFilterValidator( 0 ),
    min(aMin),
    max(aMax)
{
    if (aMin >= 0)
        validChars = newStr( TRangeValidator::validUnsignedChars );
    else
        validChars = newStr( TRangeValidator::validSignedChars );
}

#if !defined(NO_STREAMABLE)

TRangeValidator::TRangeValidator( StreamableInit s) noexcept : TFilterValidator(s)
{
}

void TRangeValidator::write(opstream& os)
{
  TFilterValidator::write( os );
  os << min << max;
}

void* TRangeValidator::read( ipstream& is )
{
  TFilterValidator::read( is );
  is >> min >> max;
  return this;
}

#endif

void TRangeValidator::error()
{
  messageBox( mfError | mfOKButton, errorMsg, min,max);
}


Boolean TRangeValidator::isValid(const char* s)
{
  long value;

  if (TFilterValidator::isValid(s))
    if (sscanf(s, "%ld", &value) == 1)
        if ((value >= min) && (value <= max))
            return True;

  return False;
}

ushort TRangeValidator::transfer(char* s, void* buffer, TVTransfer flag)
{
  long value;

  if ((options & voTransfer) != 0)
  {
    switch ( flag )
    {
    case vtDataSize:
      break;
    case vtGetData:
      sscanf(s,"%ld",&value);
      *(long*)buffer = value;
      break;
    case vtSetData:
      sprintf(s, "%ld", *(long*)buffer);
      break;
    }
    return sizeof(long);
  }
  else
    return 0;
}

// TLookupValidator

#if !defined(NO_STREAMABLE)

TLookupValidator::TLookupValidator( StreamableInit s) noexcept : TValidator(s)
{
}

#endif

Boolean TLookupValidator::isValid(const char* s)
{
  return lookup(s);
}

Boolean TLookupValidator::lookup(const char* s)
{
  return True;
}

// TStringLookupValidator

TStringLookupValidator::TStringLookupValidator(TStringCollection* aStrings) noexcept
{
  strings = aStrings;
}

#if !defined(NO_STREAMABLE)

void TStringLookupValidator::write( opstream& os )
{
  TLookupValidator::write( os );
  os << strings;
}

void* TStringLookupValidator::read( ipstream& is )
{
  TLookupValidator::read(is);
  is >> strings;

  return this;
}

TStringLookupValidator::TStringLookupValidator( StreamableInit s) noexcept :
        TLookupValidator(s)
{
}

#endif

TStringLookupValidator::~TStringLookupValidator()
{
  newStringList(0);
}

void TStringLookupValidator::error()
{
  messageBox(mfError | mfOKButton, errorMsg);
}

static Boolean stringMatch(void* a1, void* a2)
{
    return Boolean(strcmp((const char*)a1, (const char*)a2) == 0);
}


Boolean TStringLookupValidator::lookup(const char* s)
{
    return Boolean(strings->firstThat(stringMatch,(void*)s) != 0);
}

void TStringLookupValidator::newStringList(TStringCollection* aStrings)
{
  if (strings)
      destroy(strings);

  strings = aStrings;
}


#if !defined(NO_STREAMABLE)

TStreamable *TValidator::build()
{
    return new TValidator( streamableInit );
}

TStreamable *TRangeValidator::build()
{
    return new TRangeValidator( streamableInit );
}

TStreamable *TFilterValidator::build()
{
    return new TFilterValidator( streamableInit );
}

TStreamable *TPXPictureValidator::build()
{
    return new TPXPictureValidator( streamableInit );
}

TStreamable *TLookupValidator::build()
{
    return new TLookupValidator( streamableInit );
}

TStreamable *TStringLookupValidator::build()
{
    return new TStringLookupValidator( streamableInit );
}

#endif
