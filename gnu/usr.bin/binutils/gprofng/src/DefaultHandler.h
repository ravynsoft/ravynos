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

/*
 *  org/xml/sax/helpers/DefaultHandler.java
 *  Based on JavaTM 2 Platform Standard Ed. 5.0
 */

#ifndef _DefaultHandler_h
#define _DefaultHandler_h

/*
 *	org/xml/sax/Attributes.java
 */
class Attributes
{
public:
  virtual ~Attributes () { };

  virtual int getLength () = 0;
  virtual const char *getQName (int index) = 0;
  virtual const char *getValue (int index) = 0;
  virtual int getIndex (const char *qName) = 0;
  virtual const char *getValue (const char *qName) = 0;
};

/*
 *	org/xml/sax/SAXException.java
 */
class SAXException
{
public:
  SAXException ();
  SAXException (const char *message);
  virtual ~SAXException ();
  char *getMessage ();

private:
  char *message;
};

class SAXParseException : public SAXException
{
public:
  SAXParseException (char *message, int lineNumber, int columnNumber);

  int
  getLineNumber ()
  {
    return lineNumber;
  }

  int
  getColumnNumber ()
  {
    return columnNumber;
  }

private:
  int lineNumber;
  int columnNumber;
};

class DefaultHandler
{
public:
  virtual ~DefaultHandler () { };

  virtual void startDocument () = 0;
  virtual void endDocument () = 0;
  virtual void startElement (char *uri, char *localName, char *qName,
			     Attributes *attributes) = 0;
  virtual void endElement (char *uri, char *localName, char *qName) = 0;
  virtual void characters (char *ch, int start, int length) = 0;
  virtual void ignorableWhitespace (char *ch, int start, int length) = 0;

  virtual void
  warning (SAXParseException *e)
  {
    delete e;
  }

  virtual void
  error (SAXParseException *e)
  {
    delete e;
  }

  virtual void
  fatalError (SAXParseException *e)
  {
    throw ( e);
  }
  void dump_startElement (const char *qName, Attributes *attributes);
};

#endif /* _DefaultHandler_h */
