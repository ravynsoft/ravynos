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

#ifndef _QLPARSER_H
#define _QLPARSER_H

#include <sstream>
#include <istream>
#include <iostream>
#include "Expression.h"

/* This class contains parser inputs (a string, if non-NULL: if NULL, use cin),
   and outputs (obtained via operator(), which resets the output
   expression).  The destructor deletes the returned expression to allow
   exception throws on syntax error to clean up properly.  */

namespace QL
{
  struct Result
  {
    std::stringstream streamify;
  public:
    std::istream in;
    Expression *out;

    Result () : in (std::cin.rdbuf ()), out (NULL) { }
    Result (const char *instr) : streamify (std::string (instr)),
	in (streamify.rdbuf ()), out (NULL) { }

    Expression *operator() ()
    {
      Expression *o = out;
      out = NULL;
      return o;
    }

    ~Result ()
    {
      delete out;
    }
  };
};

#endif /* _QLPARSER_H */
