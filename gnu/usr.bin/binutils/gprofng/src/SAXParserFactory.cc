/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <ctype.h>

#include "util.h"
#include "vec.h"
#include "DefaultHandler.h"
#include "SAXParser.h"
#include "SAXParserFactory.h"
#include "StringBuilder.h"

/*
 *  Private implementation of Attributes
 */
class AttributesP : public Attributes
{
public:
  AttributesP ();
  ~AttributesP ();
  int getLength ();
  const char *getQName (int index);
  const char *getValue (int index);
  int getIndex (const char *qName);
  const char *getValue (const char *qName);
  void append (char *qName, char *value);

private:
  Vector<char*> *names;
  Vector<char*> *values;
};

AttributesP::AttributesP ()
{
  names = new Vector<char*>;
  values = new Vector<char*>;
}

AttributesP::~AttributesP ()
{
  Destroy (names);
  Destroy (values);
}

int
AttributesP::getLength ()
{
  return names->size ();
}

const char *
AttributesP::getQName (int index)
{
  if (index < 0 || index >= names->size ())
    return NULL;
  return names->fetch (index);
}

const char *
AttributesP::getValue (int index)
{
  if (index < 0 || index >= values->size ())
    return NULL;
  return values->fetch (index);
}

int
AttributesP::getIndex (const char *qName)
{
  for (int idx = 0; idx < names->size (); idx++)
    if (strcmp (names->fetch (idx), qName) == 0)
      return idx;
  return -1;
}

const char *
AttributesP::getValue (const char *qName)
{
  for (int idx = 0; idx < names->size (); idx++)
    if (strcmp (names->fetch (idx), qName) == 0)
      return values->fetch (idx);
  return NULL;
}

void
AttributesP::append (char *qName, char *value)
{
  names->append (qName);
  values->append (value);
}

/*
 *  Implementation of SAXException
 */
SAXException::SAXException ()
{
  message = strdup ("null");
}

SAXException::SAXException (const char *_message)
{
  if (_message == NULL)
    message = strdup ("null");
  else
    message = strdup (_message);
}

SAXException::~SAXException ()
{
  free (message);
}

char *
SAXException::getMessage ()
{
  return message;
}

/*
 *  SAXParseException
 */
SAXParseException::SAXParseException (char *message, int _lineNumber, int _columnNumber)
: SAXException (message == NULL ? GTXT ("XML parse error") : message)
{
  lineNumber = _lineNumber;
  columnNumber = _columnNumber;
}

/*
 *  Private implementation of SAXParser
 */
class SAXParserP : public SAXParser
{
public:
  SAXParserP ();
  ~SAXParserP ();
  void reset ();
  void parse (File*, DefaultHandler*);

  bool
  isNamespaceAware ()
  {
    return false;
  }

  bool
  isValidating ()
  {
    return false;
  }

private:

  static const int CH_EOF = -1;

  void nextch ();
  bool isWSpace ();
  void skipWSpaces ();
  void scanString (const char *str);
  char *parseName ();
  char *parseString ();
  char *decodeString (char *str);
  Attributes *parseAttributes ();
  void parseTag ();
  void parseDocument ();
  void parsePart (int idx);

  DefaultHandler *dh;
  int bufsz;
  char *buffer;
  int cntsz;
  int idx;
  int curch;
  int line;
  int column;
};

SAXParserP::SAXParserP ()
{
  dh = NULL;
  bufsz = 0x2000;
  buffer = (char*) malloc (bufsz);
  cntsz = 0;
  idx = 0;
  line = 1;
  column = 0;
}

SAXParserP::~SAXParserP ()
{
  free (buffer);
}

void
SAXParserP::reset ()
{
  dh = NULL;
  bufsz = 8192;
  buffer = (char*) realloc (buffer, bufsz);
  cntsz = 0;
  idx = 0;
  line = 1;
  column = 0;
}

void
SAXParserP::parse (File *f, DefaultHandler *_dh)
{
  if (_dh == NULL)
    return;
  dh = _dh;
  FILE *file = (FILE*) f;
  int rem = bufsz;
  cntsz = 0;
  idx = 0;
  for (;;)
    {
      int n = (int) fread (buffer + cntsz, 1, rem, file);
      if (ferror (file) || n <= 0)
	break;
      cntsz += n;
      if (feof (file))
	break;
      rem -= n;
      if (rem == 0)
	{
	  int oldbufsz = bufsz;
	  bufsz = bufsz >= 0x100000 ? bufsz + 0x100000 : bufsz * 2;
	  buffer = (char*) realloc (buffer, bufsz);
	  rem = bufsz - oldbufsz;
	}
    }
  nextch ();
  parseDocument ();
}

static int
hex (char c)
{
  if (c >= '0' && c <= '9')
    return (c - '0');
  else if (c >= 'a' && c <= 'f')
      return 10 + (c - 'a');
  return -1;
}

void
SAXParserP::nextch ()
{
  curch = idx >= cntsz ? CH_EOF : buffer[idx++];
  if (curch == '\n')
    {
      line += 1;
      column = 0;
    }
  else
    column += 1;
}

bool
SAXParserP::isWSpace ()
{
  return curch == ' ' || curch == '\t' || curch == '\n' || curch == '\r';
}

void
SAXParserP::skipWSpaces ()
{
  while (isWSpace ())
    nextch ();
}

void
SAXParserP::scanString (const char *str)
{
  if (str == NULL || *str == '\0')
    return;
  for (;;)
    {
      if (curch == CH_EOF)
	break;
      else if (curch == *str)
	{
	  const char *p = str;
	  for (;;)
	    {
	      p += 1;
	      nextch ();
	      if (*p == '\0')
		return;
	      if (curch != *p)
		break;
	    }
	}
      nextch ();
    }
}

char *
SAXParserP::parseName ()
{
  StringBuilder *name = new StringBuilder ();

  if ((curch >= 'A' && curch <= 'Z') || (curch >= 'a' && curch <= 'z'))
    {
      name->append ((char) curch);
      nextch ();
      while (isalnum (curch) != 0 || curch == '_')
	{
	  name->append ((char) curch);
	  nextch ();
	}
    }

  char *res = name->toString ();
  delete name;
  return res;
}

/**
 * Replaces encoded XML characters with original characters
 * Attention: this method reuses the same string that is passed as the argument
 * @param str
 * @return str
 */
char *
SAXParserP::decodeString (char * str)
{
  // Check if string has %22% and replace it with double quotes
  // Also replace all other special combinations.
  char *from = str;
  char *to = str;
  if (strstr (from, "%") || strstr (from, "&"))
    {
      int len = strlen (from);
      for (int i = 0; i < len; i++)
	{
	  int nch = from[i];
	  // Process &...; combinations
	  if (nch == '&' && i + 3 < len)
	    {
	      if (from[i + 2] == 't' && from[i + 3] == ';')
		{
		  // check &lt; &gt;
		  if (from[i + 1] == 'l')
		    {
		      nch = '<';
		      i += 3;
		    }
		  else if (from[i + 1] == 'g')
		    {
		      nch = '>';
		      i += 3;
		    }
		}
	      else if (i + 4 < len && from[i + 4] == ';')
		{
		  // check &amp;
		  if (from[i + 1] == 'a' && from[i + 2] == 'm' && from[i + 3] == 'p')
		    {
		      nch = '&';
		      i += 4;
		    }
		}
	      else if ((i + 5 < len) && (from[i + 5] == ';'))
		{
		  // check &apos; &quot;
		  if (from[i + 1] == 'a' && from[i + 2] == 'p'
		      && from[i + 3] == 'o' && from[i + 4] == 's')
		    {
		      nch = '\'';
		      i += 5;
		    }
		  if (from[i + 1] == 'q' && from[i + 2] == 'u' && from[i + 3] == 'o' && from[i + 4] == 't')
		    {
		      nch = '"';
		      i += 5;
		    }
		}
	    }
	  // Process %XX% combinations
	  if (nch == '%' && i + 3 < len && from[i + 3] == '%')
	    {
	      int ch = hex (from[i + 1]);
	      if (ch >= 0)
		{
		  int ch2 = hex (from[i + 2]);
		  if (ch2 >= 0)
		    {
		      ch = ch * 16 + ch2;
		      nch = ch;
		      i += 3;
		    }
		}
	    }
	  *to++ = (char) nch;
	}
      *to = '\0';
    }
  return str;
}

char *
SAXParserP::parseString ()
{
  StringBuilder *str = new StringBuilder ();
  int quote = '>';
  if (curch == '"')
    {
      quote = curch;
      nextch ();
    }
  for (;;)
    {
      if (curch == CH_EOF)
	break;
      if (curch == quote)
	{
	  nextch ();
	  break;
	}
      str->append ((char) curch);
      nextch ();
    }

  char *res = str->toString ();
  // Decode XML characters
  res = decodeString (res);
  delete str;
  return res;
}

Attributes *
SAXParserP::parseAttributes ()
{
  AttributesP *attrs = new AttributesP ();

  for (;;)
    {
      skipWSpaces ();
      char *name = parseName ();
      if (name == NULL || *name == '\0')
	{
	  free (name);
	  break;
	}
      skipWSpaces ();
      if (curch != '=')
	{
	  SAXParseException *e = new SAXParseException (NULL, line, column);
	  dh->error (e);
	  scanString (">");
	  free (name);
	  return attrs;
	}
      nextch ();
      skipWSpaces ();
      char *value = parseString ();
      attrs->append (name, value);
    }
  return attrs;
}

void
SAXParserP::parseTag ()
{
  skipWSpaces ();
  bool empty = false;
  char *name = parseName ();
  if (name == NULL || *name == '\0')
    {
      SAXParseException *e = new SAXParseException (NULL, line, column);
      dh->error (e);
      scanString (">");
      free (name);
      return;
    }

  Attributes *attrs = parseAttributes ();
  if (curch == '/')
    {
      nextch ();
      empty = true;
    }
  if (curch == '>')
    nextch ();
  else
    {
      empty = false;
      SAXParseException *e = new SAXParseException (NULL, line, column);
      dh->error (e);
      scanString (">");
    }
  if (curch == CH_EOF)
    {
      free (name);
      delete attrs;
      return;
    }
  dh->startElement (NULL, NULL, name, attrs);
  if (empty)
    {
      dh->endElement (NULL, NULL, name);
      free (name);
      delete attrs;
      return;
    }

  StringBuilder *chars = new StringBuilder ();
  bool wspaces = true;
  for (;;)
    {
      if (curch == CH_EOF)
	break;
      else if (curch == '<')
	{
	  if (chars->length () > 0)
	    {
	      char *str = chars->toString ();
	      // Decode XML characters
	      str = decodeString (str);
	      if (wspaces)
		dh->ignorableWhitespace (str, 0, chars->length ());
	      else
		dh->characters (str, 0, chars->length ());
	      free (str);
	      chars->setLength (0);
	      wspaces = true;
	    }
	  nextch ();
	  if (curch == '/')
	    {
	      nextch ();
	      char *ename = parseName ();
	      if (ename && *ename != '\0')
		{
		  if (strcmp (name, ename) == 0)
		    {
		      skipWSpaces ();
		      if (curch == '>')
			{
			  nextch ();
			  dh->endElement (NULL, NULL, name);
			  free (ename);
			  break;
			}
		      SAXParseException *e = new SAXParseException (NULL, line, column);
		      dh->error (e);
		    }
		  else
		    {
		      SAXParseException *e = new SAXParseException (NULL, line, column);
		      dh->error (e);
		    }
		  scanString (">");
		}
	      free (ename);
	    }
	  else
	    parseTag ();
	}
      else
	{
	  if (!isWSpace ())
	    wspaces = false;
	  chars->append ((char) curch);
	  nextch ();
	}
    }

  free (name);
  delete attrs;
  delete chars;
  return;
}

void
SAXParserP::parseDocument ()
{
  dh->startDocument ();
  for (;;)
    {
      if (curch == CH_EOF)
	break;
      if (curch == '<')
	{
	  nextch ();
	  if (curch == '?')
	    scanString ("?>");
	  else if (curch == '!')
	    scanString (">");
	  else
	    parseTag ();
	}
      else
	nextch ();
    }
  dh->endDocument ();
}

/*
 *  Private implementation of SAXParserFactory
 */
class SAXParserFactoryP : public SAXParserFactory
{
public:
  SAXParserFactoryP () { }
  ~SAXParserFactoryP () { }
  SAXParser *newSAXParser ();

  void
  setFeature (const char *, bool) { }

  bool
  getFeature (const char *)
  {
    return false;
  }
};

SAXParser *
SAXParserFactoryP::newSAXParser ()
{
  return new SAXParserP ();
}

/*
 *  SAXParserFactory
 */
const char *SAXParserFactory::DEFAULT_PROPERTY_NAME = "javax.xml.parsers.SAXParserFactory";

SAXParserFactory *
SAXParserFactory::newInstance ()
{
  return new SAXParserFactoryP ();
}

void
DefaultHandler::dump_startElement (const char *qName, Attributes *attrs)
{
  fprintf (stderr, NTXT ("DefaultHandler::startElement qName='%s'\n"), STR (qName));
  for (int i = 0, sz = attrs ? attrs->getLength () : 0; i < sz; i++)
    {
      const char *qn = attrs->getQName (i);
      const char *vl = attrs->getValue (i);
      fprintf (stderr, NTXT ("  %d  '%s' = '%s'\n"), i, STR (qn), STR (vl));
    }
}
