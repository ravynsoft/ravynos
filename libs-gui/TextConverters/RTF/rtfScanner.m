/* rtcScanner

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Stefan Boehringer (stefan.boehringer@uni-bochum.de)
   Date: Dec 1999

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rtfScanner.h"
#include "rtfGrammar.tab.h"

//	<§> scanner types and helpers

#define CArraySize(a) (sizeof(a)/sizeof((a)[0])-1)

typedef struct {
  char	*bf;
  int	length, position, chunkSize;
} DynamicString;

typedef struct {
  const char	*string;
  int		token;
} LexKeyword;

GSLexError initDynamicString (DynamicString *string)
{
  string->length = 0, string->position = 0, string->chunkSize = 128;
  string->bf = calloc(1, string->length = string->chunkSize);
  if (!string->bf) 
    {
      return LEXoutOfMemory;
    }
  return NoError;
}

GSLexError appendChar (DynamicString *string, int c)
{
  if (string->position == string->length)
    {
      if (!(string->bf = realloc(string->bf, 
				 string->length += string->chunkSize))) 
	{
	  return LEXoutOfMemory;
	}
      else 
	{
	  string->chunkSize <<= 1;
	}
    }
  
  string->bf[string->position++] = c;
  return NoError;
}

void lexInitContext (RTFscannerCtxt *lctxt, void *customContext, 
		     int (*getcharFunction)(void *))
{
  lctxt->streamLineNumber = 1;
  lctxt->streamPosition = lctxt->pushbackCount = 0;
  lctxt->lgetchar = getcharFunction;
  lctxt->customContext = customContext;
}

int lexGetchar (RTFscannerCtxt *lctxt)
{
  int	c;
  if (lctxt->pushbackCount)
    {
      lctxt->pushbackCount--;
      c = lctxt->pushbackBuffer[lctxt->pushbackCount];
    }
  else
    {
      lctxt->streamPosition++;
      c = lctxt->lgetchar(lctxt->customContext);
    }
  if (c == '\n') 
    {
      lctxt->streamLineNumber++;
    }
  return c;
}

void lexUngetchar (RTFscannerCtxt *lctxt, int c)
{
  if (c == '\n') 
    {
      lctxt->streamLineNumber--;
    }
  lctxt->pushbackBuffer[lctxt->pushbackCount++] = c;	//<!> no checking here
}

int lexStreamPosition (RTFscannerCtxt *lctxt)
{
  return lctxt->streamPosition - lctxt->pushbackCount;
}

char *my_strdup (const char *str)
{
  char *copy = str? malloc (strlen (str) + 1): 0;
  return !copy? 0: strcpy(copy, str);
}

int findStringFromKeywordArray(const char *string, const LexKeyword *array,
			       int arrayCount)
{
  int	min, max, mid, cmp;
  const LexKeyword *currentKeyword;
  
  for (min = 0, max = arrayCount; min <= max; )
    {
      mid = (min + max)>>1;
      currentKeyword = array + mid;
      if (!(cmp = strcmp (string, currentKeyword->string)))
	{
	  return currentKeyword->token;
	} 
      else if (cmp > 0) 
	{
	  min = mid + 1;
	}
      else 
	{
	  max = mid - 1;
	}
    }
  return 0; // couldn't find
}

//	end <§> scanner types and helpers

//	<§> core scanner functions

#define token(a) (a)

//	<!> must be sorted
LexKeyword RTFcommands[] = 
  {
    {"NeXTGraphic",token(RTFNeXTGraphic)},
    {"NeXTHelpLink",token(RTFNeXTHelpLink)},
    {"NeXTHelpMarker",token(RTFNeXTHelpMarker)},
    {"ansi",      token(RTFansi)},
    {"ansicpg",   token(RTFansicpg)},
    {"b",         token(RTFbold)},
    {"blue",      token(RTFblue)},
    {"bullet",    token(RTFbullet)},
    {"cb",        token(RTFcolorbg)},
    {"cell",      token(RTFcell)},
    {"cf",        token(RTFcolorfg)},
    {"colortbl",  token(RTFcolortable)},
    {"cpg",       token(RTFcpg)},
    {"dn",        token(RTFsubscript)},
    {"emdash",    token(RTFemdash)},
    {"emspace",   token(RTFemspace)},
    {"endash",    token(RTFendash)},
    {"enspace",   token(RTFenspace)},
    {"f",         token(RTFfont)},
    {"fbidi",     token(RTFfamilyBiDi)},
    {"fcharset",  token(RTFfcharset)},
    {"fdecor",    token(RTFfamilyDecor)},
    {"fi",        token(RTFfirstLineIndent)},
    {"field",     token(RTFfield)},
    {"filename",  token(RTFNeXTfilename)},
    {"fldalt",    token(RTFfldalt)},
    {"flddirty",  token(RTFflddirty)},
    {"fldedit",   token(RTFfldedit)},
    {"fldinst",   token(RTFfldinst)},
    {"fldlock",   token(RTFfldlock)},
    {"fldpriv",   token(RTFfldpriv)},
    {"fldrslt",   token(RTFfldrslt)},
    {"fmodern",   token(RTFfamilyModern)},
    {"fnil",      token(RTFfamilyNil)},
    {"fonttbl",   token(RTFfontListStart)},
    /* All footers are mapped on one entry */
    {"footer",    token(RTFfooter)},
    {"footerf",   token(RTFfooter)},
    {"footerl",   token(RTFfooter)},
    {"footerr",   token(RTFfooter)},
    {"footnote",  token(RTFfootnote)},
    {"fprq",      token(RTFfprq)},
    {"froman",    token(RTFfamilyRoman)},
    {"fs",        token(RTFfontSize)},
    {"fscript",   token(RTFfamilyScript)},
    {"fswiss",    token(RTFfamilySwiss)},
    {"ftech",     token(RTFfamilyTech)},
    {"fttruetype", token(RTFfttruetype)},
    {"green",     token(RTFgreen)},
    /* All headers are mapped on one entry */
    {"header",    token(RTFheader)},
    {"headerf",   token(RTFheader)},
    {"headerl",   token(RTFheader)},
    {"headerr",   token(RTFheader)},
    {"height",    token(RTFNeXTGraphicHeight)},
    {"i",         token(RTFitalic)},
    {"info",      token(RTFinfo)},
    {"ldblquote", token(RTFldblquote)},
    {"li",        token(RTFleftIndent)},
    {"linkFilename",token(RTFNeXTlinkFilename)},
    {"linkMarkername",token(RTFNeXTlinkMarkername)},
    {"lquote",    token(RTFlquote)},
    {"mac",       token(RTFmac)},
    {"margb",     token(RTFmarginButtom)},
    {"margl",     token(RTFmarginLeft)},
    {"margr",     token(RTFmarginRight)},
    {"margt",     token(RTFmarginTop)},
    {"markername",token(RTFNeXTmarkername)},
    {"paperh",    token(RTFpaperHeight)},
    {"paperw",    token(RTFpaperWidth)},
    {"par",       token(RTFparagraph)},
    {"pard",      token(RTFdefaultParagraph)},
    {"pc",        token(RTFpc)},
    {"pca",       token(RTFpca)},
    {"pict",      token(RTFpict)},
    {"plain",     token(RTFplain)},
    {"qc",        token(RTFalignCenter)},
    {"qj",        token(RTFalignJustified)},
    {"ql",        token(RTFalignLeft)},
    {"qr",        token(RTFalignRight)},
    {"rdblquote", token(RTFrdblquote)},
    {"red",       token(RTFred)},
    {"ri",        token(RTFrightIndent)},
    {"row",       token(RTFrow)},
    {"rquote",    token(RTFrquote)},
    {"rtf",       token(RTFstart)},
    {"s",         token(RTFstyle)},
    {"sa",        token(RTFspaceAbove)},
    {"sl",        token(RTFlineSpace)},
    {"strike",    token(RTFstrikethrough)},
    {"striked1",  token(RTFstrikethroughDouble)},
    {"stylesheet",token(RTFstylesheet)},
    {"tab",       token(RTFtabulator)},
    {"tx",        token(RTFtabstop)},
    {"u",         token(RTFunichar)},
    {"ul",        token(RTFunderline)},
    {"ulc",       token(RTFunderlinecolor)},
    {"uld",       token(RTFunderlineDot)},
    {"uldash",    token(RTFunderlineDash)},
    {"uldashd",   token(RTFunderlineDashDot)},
    {"uldashdd",  token(RTFunderlineDashDotDot)},
    {"uldb",      token(RTFunderlineDouble)},
    {"ulnone",    token(RTFunderlineStop)},
    {"ulth",      token(RTFunderlineThick)},
    {"ulthd",     token(RTFunderlineThickDot)},
    {"ulthdash",  token(RTFunderlineThickDash)},
    {"ulthdashd", token(RTFunderlineThickDashDot)},
    {"ulthdashdd",token(RTFunderlineThickDashDotDot)},
    {"ulw",       token(RTFunderlineWord)},
    {"up",        token(RTFsuperscript)},
    {"width",     token(RTFNeXTGraphicWidth)}
  };

BOOL probeCommand (RTFscannerCtxt *lctxt)
{
  int	c = lexGetchar(lctxt);
  lexUngetchar (lctxt, c);
  if (isalpha(c))
    {
      return YES;
    }
  return NO;
}

//	<N> According to spec a cmdLength of 32 is respected
#define RTFMaxCmdLength 32
#define RTFMaxArgumentLength 64
GSLexError readCommand (RTFscannerCtxt *lctxt, 
			YYSTYPE *lvalp, 
			int *token) // the '\\' is already read
{
  char cmdNameBf[RTFMaxCmdLength+1], *cmdName = cmdNameBf;
  char argumentBf[RTFMaxArgumentLength+1], *argument = argumentBf;
  int  c, foundToken;

  lvalp->cmd.name = 0;	// initialize
  while (isalpha (c = lexGetchar(lctxt)))
    {
      *cmdName++ = c;
      if (cmdName >= cmdNameBf + RTFMaxCmdLength) 
	{
	  return LEXsyntaxError;
	}
    }
  *cmdName = 0;
  if (!(foundToken = findStringFromKeywordArray(cmdNameBf, RTFcommands, 
						CArraySize(RTFcommands))))
    {
      if (!(lvalp->cmd.name = my_strdup(cmdNameBf))) 
	{
	  return LEXoutOfMemory;
	}
      *token = RTFOtherStatement;
    } 
  else 
    {
      *token = foundToken;
    }
  if (c == ' ')				// this is an empty argument
    {	
      lvalp->cmd.isEmpty = YES;
    } 
  else if (isdigit(c) || c == '-')	// we've found a numerical argument
    {
      do 
	{
	  *argument++ = c;
	  if (argument >= argumentBf + RTFMaxArgumentLength) 
	    {
	      return LEXsyntaxError;
	    }
	} while (isdigit(c = lexGetchar(lctxt)));
      *argument = 0;
      if (c != ' ') 
	{
	  lexUngetchar(lctxt, c); 	// <N> ungetc non-digit
	}
      // the consumption of the space seems necessary on NeXT but
      // is not according to spec
      lvalp->cmd.isEmpty = NO;
      lvalp->cmd.parameter = atoi(argumentBf);
    } 
  else 
    {
      lvalp->cmd.isEmpty = YES;
      lexUngetchar(lctxt, c); 		// ungetc non-whitespace delimiter
    }
  return NoError;
}

GSLexError readText (RTFscannerCtxt *lctxt, YYSTYPE *lvalp)
{
  int c;
  DynamicString text;
  GSLexError error;
  
  if ((error = initDynamicString(&text))) 
    {
      return error;
    }
  for (;;)
    {
      c = lexGetchar(lctxt);
      
      if (c == EOF || c == '{' || c == '}' || c == '\\')
	{
	  lexUngetchar(lctxt, c);
	  break;
	}
      else 
	{
	  // <N> newline and cr are ignored if not quoted
	  if (c != '\n' && c != '\r')
	    {
	      appendChar(&text, c);
	    }
	}
    }
  appendChar(&text, 0);
  lvalp->text = text.bf; // release is up to the consumer
  return NoError;
}

// read in a character as two hex digit
static int gethex(RTFscannerCtxt *lctxt)
{
  int c = 0;
  int i;
  
  for (i = 0; i < 2; i++)
    {
      int c1 = lexGetchar(lctxt);
      
      if (!isxdigit(c1))
	{
	  lexUngetchar(lctxt, c1);
	  break;
	}
      else 
	{
	  c = c * 16;
	  if (isdigit(c1))
	    {
	      c += c1 - '0';
	    }
	  else if (isupper(c1))
	    {
	      c += c1 - 'A' + 10;
	    }
	  else
	    {
	      c += c1 - 'a' + 10;
	    }
	}
    }
  
  return c;
}

int GSRTFlex (YYSTYPE *lvalp, //YYLTYPE *llocp, 
	      RTFscannerCtxt *lctxt)	/* provide value and position in the params */ 
{
  int c;
  int token = 0;
  char *cv;

  do
    {
      c = lexGetchar(lctxt);
    }
  while ( c == '\n' || c == '\r' );	// <A> the listed characters are to be ignored
  
  switch (c)
    {
    case EOF: 
      {
	token = 0;
	break;
      }
    case '{':
      {
	token = '{';
	break;
      }
    case '}': 
      {
	token = '}'; 
	break;
      }
    case '\\':
      if (probeCommand(lctxt) == YES)
	{
	  readCommand(lctxt, lvalp, &token);
	  switch (token)
	  {
	      case RTFtabulator: c = '\t';
		  break;
	      case RTFcell: c = '\t';
		  break;
	      case RTFemdash: c = '-';
		  break;
	      case RTFendash: c = '-';
		  break;
	      case RTFbullet: c = '*';
		  break;
	      case RTFlquote: c = '`';
		  break;
	      case RTFrquote: c = '\'';
		  break;
	      case RTFldblquote: c = '"';
		  break;
	      case RTFrdblquote: c = '"';
		  break;
	      default:
		  return token;
	  }
	}
      else
        {
	  c = lexGetchar(lctxt);
	  switch (c)
	  {
	      case EOF: token = 0;
		  return token;
	      case '\'':
		  // Convert the next two hex digits into a char
		  c = gethex(lctxt);
		  break;
	      case '*': return RTFignore;
	      case '|': 
	      case '-': 
	      case ':':
		  // Ignore these characters
		  c = lexGetchar(lctxt);
		  break;
	      case '_': c = '-';
		  break;
	      case '~': c = ' ';
		  break;
	      case '\n':
	      case '\r':
		  return RTFparagraph;
	      case '{':
	      case '}':
	      case '\\':
		  // release is up to the consumer
		  cv = calloc(1, 2);
		  cv[0] = c;
		  cv[1] = '\0';
		  lvalp->text = cv;
		  token = RTFtext;
		  return token;
	      default:
		  // fall through
		  break;
	  }
	}
      // else fall through to default: read text <A>
      // no break <A>
    default:
      lexUngetchar(lctxt, c);
      readText(lctxt, lvalp);
      token = RTFtext;
      break;
    }
  
  //*llocp = lctxt->position();
  return token;
}
