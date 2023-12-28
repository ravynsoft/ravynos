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

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <inttypes.h>

class Experiment;
class DataView;
class DbeView;
class Histable;

class Expression
{
public:

  class Context
  {
  public:
    Context (DbeView *_dbev, Experiment *_exp = 0);
    Context (DbeView *_dbev, Experiment *_exp, DataView *_dview, long _eventId);

    ~Context () { };

    void
    put (DataView *d, long id)
    {
      dview = d;
      eventId = id;
    };

    void
    put (Experiment *_exp)
    {
      exp = _exp;
    };

    Experiment *exp;
    DataView *dview;
    DbeView *dbev;
    long eventId;
  };

  enum OpCode
  {
    OP_NONE,
    OP_QWE,
    OP_COLON,
    OP_OR,
    OP_AND,
    OP_NOT,
    OP_EQV,
    OP_NEQV,
    OP_BITOR,
    OP_BITAND,
    OP_BITXOR,
    OP_BITNOT,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_LS,
    OP_RS,
    OP_ADD,
    OP_MINUS,
    OP_MUL,
    OP_DIV,
    OP_REM,
    OP_DEG,
    OP_COMMA,
    OP_IN,
    OP_SOMEIN,
    OP_ORDRIN,
    OP_NUM,
    OP_NAME,
    OP_FUNC,
    OP_FILE,
    OP_JAVA,
    OP_HASPROP,
    OP_LIBRARY_IN,
    OP_LIBRARY_SOMEIN,
    OP_LIBRARY_ORDRIN
  };

  enum FuncCode
  {
    FUNC_FNAME,
    FUNC_DNAME
  };

  enum JavaCode
  {
    JAVA_JGROUP,
    JAVA_JPARENT
  };

  Expression (OpCode, const Expression*, const Expression* = 0);
  Expression (OpCode, uint64_t);
  Expression (const Expression &rhs);
  Expression (const Expression *rhs);
  Expression &operator= (const Expression &rhs);
  ~Expression ();

  Expression *
  copy () const
  {
    return new Expression (this);
  }
  void copy (const Expression *rhs);

  uint64_t
  eval (Context *ctx)
  {
    return bEval (ctx) ? v.val : 0;
  };

  bool
  passes (Context *ctx)
  {
    return bEval (ctx) ? v.val != 0 : true;
  };

  bool
  complete ()
  {
    return op == OP_NUM;
  };

  bool verifyObjectInExpr (Histable *obj);
  Expression *
  pEval (Context *ctx); // Partial evaluation to simplify expression

private:

  struct Value
  {

    Value (uint64_t _val = 0, Value *_next = 0) : val (_val), next (_next)
    {
      fn = 0;
    }
    uint64_t val;
    uint64_t fn;
    Value *next;
  };

  bool getVal (int propId, Context *ctx);
  bool bEval (Context *ctx);
  bool hasLoadObject ();

  OpCode op;
  Value v;
  Expression *arg0;
  Expression *arg1;
};


#endif /* _EXPRESSION_H */
