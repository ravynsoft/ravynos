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
 *	javax/xml/parsers/SAXParserFactory.java
 *
 *	Based on JavaTM 2 Platform Standard Ed. 5.0
 */

#ifndef _SAXParserFactory_h
#define _SAXParserFactory_h

class SAXParser;

class SAXParserFactory
{
public:
  static SAXParserFactory *newInstance ();

  virtual ~SAXParserFactory () { }
  virtual SAXParser *newSAXParser () = 0;
  virtual void setFeature (const char *name, bool value) = 0;
  virtual bool getFeature (const char *name) = 0;

  void
  setNamespaceAware (bool awareness)
  {
    namespaceAware = awareness;
  }

  void
  setValidating (bool _validating)
  {
    validating = _validating;
  }

  bool
  isNamespaceAware ()
  {
    return namespaceAware;
  }

  bool
  isValidating ()
  {
    return validating;
  }

protected:
  SAXParserFactory () { }

private:
  static const char *DEFAULT_PROPERTY_NAME;
  bool validating;
  bool namespaceAware;
};

#endif /* _SAXParserFactory_h */
