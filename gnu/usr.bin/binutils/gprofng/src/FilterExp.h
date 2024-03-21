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

#ifndef _FILTEREXP_H
#define _FILTEREXP_H

#include "Expression.h"

class FilterExp
{
public:

  FilterExp (Expression *_expr, Expression::Context *_ctx, bool _noParFilter) :
	     expr (_expr), ctx (_ctx), noParFilter (_noParFilter) { };

  ~FilterExp ()
  {
    delete ctx;
  }

  bool
  passes ()
  {
    return expr ? expr->passes (ctx) : true;
  }

  void
  put (DataView *dview, long eventId)
  {
    ctx->put (dview, eventId);
  }

  Expression *expr;
  Expression::Context *ctx;
  bool noParFilter;
};


#endif /* _FILTEREXP_H */
