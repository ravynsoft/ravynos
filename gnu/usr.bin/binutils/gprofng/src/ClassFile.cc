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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"
#include "DbeSession.h"
#include "ClassFile.h"
#include "Function.h"
#include "StringBuilder.h"
#include "DbeFile.h"

class ByteCodeInfo
{
public:

  ByteCodeInfo (JMethod *_func, int _bci, int _lno)
  {
    func = _func;
    bci = _bci;
    lno = _lno;
  };

  JMethod *func;
  int bci;
  int lno;
};

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;

// Class File Constants
#define JAVA_MAGIC        0xcafebabe

enum {
  // First argument in access_flags_to_str()
  ClassAccess = 1,
  FieldAccess,
  MethodAccess,
  NestedClassAccess,

  // jdk/src/share/classes/sun/tools/java/RuntimeConstants.java
  // Type codes
  T_CLASS             = 0x00000002,
  T_BOOLEAN           = 0x00000004,
  T_CHAR              = 0x00000005,
  T_FLOAT             = 0x00000006,
  T_DOUBLE            = 0x00000007,
  T_BYTE              = 0x00000008,
  T_SHORT             = 0x00000009,
  T_INT               = 0x0000000a,
  T_LONG              = 0x0000000b,

// Access and modifier flags
  ACC_PUBLIC          = 0x00000001,
  ACC_PRIVATE         = 0x00000002,
  ACC_PROTECTED       = 0x00000004,
  ACC_STATIC          = 0x00000008,
  ACC_FINAL           = 0x00000010,
  ACC_SYNCHRONIZED    = 0x00000020,
  ACC_VOLATILE        = 0x00000040,
  ACC_TRANSIENT       = 0x00000080,
  ACC_NATIVE          = 0x00000100,
  ACC_INTERFACE       = 0x00000200,
  ACC_ABSTRACT        = 0x00000400,
  ACC_STRICT          = 0x00000800,
  ACC_SYNTHETIC       = 0x00001000,
  ACC_ANNOTATION      = 0x00002000,
  ACC_ENUM            = 0x00004000,

  ACC_SUPER           = 0x00000020,
  ACC_BRIDGE          = 0x00000040,
  ACC_VARARGS         = 0x00000080,

// Opcodes
  opc_try             = -3,
  opc_dead            = -2,
  opc_label           = -1,
  opc_nop             = 0,
  opc_aconst_null     = 1,
  opc_iconst_m1       = 2,
  opc_iconst_0        = 3,
  opc_iconst_1        = 4,
  opc_iconst_2        = 5,
  opc_iconst_3        = 6,
  opc_iconst_4        = 7,
  opc_iconst_5        = 8,
  opc_lconst_0        = 9,
  opc_lconst_1        = 10,
  opc_fconst_0        = 11,
  opc_fconst_1        = 12,
  opc_fconst_2        = 13,
  opc_dconst_0        = 14,
  opc_dconst_1        = 15,
  opc_bipush          = 16,
  opc_sipush          = 17,
  opc_ldc             = 18,
  opc_ldc_w           = 19,
  opc_ldc2_w          = 20,
  opc_iload           = 21,
  opc_lload           = 22,
  opc_fload           = 23,
  opc_dload           = 24,
  opc_aload           = 25,
  opc_iload_0         = 26,
  opc_iload_1         = 27,
  opc_iload_2         = 28,
  opc_iload_3         = 29,
  opc_lload_0         = 30,
  opc_lload_1         = 31,
  opc_lload_2         = 32,
  opc_lload_3         = 33,
  opc_fload_0         = 34,
  opc_fload_1         = 35,
  opc_fload_2         = 36,
  opc_fload_3         = 37,
  opc_dload_0         = 38,
  opc_dload_1         = 39,
  opc_dload_2         = 40,
  opc_dload_3         = 41,
  opc_aload_0         = 42,
  opc_aload_1         = 43,
  opc_aload_2         = 44,
  opc_aload_3         = 45,
  opc_iaload          = 46,
  opc_laload          = 47,
  opc_faload          = 48,
  opc_daload          = 49,
  opc_aaload          = 50,
  opc_baload          = 51,
  opc_caload          = 52,
  opc_saload          = 53,
  opc_istore          = 54,
  opc_lstore          = 55,
  opc_fstore          = 56,
  opc_dstore          = 57,
  opc_astore          = 58,
  opc_istore_0        = 59,
  opc_istore_1        = 60,
  opc_istore_2        = 61,
  opc_istore_3        = 62,
  opc_lstore_0        = 63,
  opc_lstore_1        = 64,
  opc_lstore_2        = 65,
  opc_lstore_3        = 66,
  opc_fstore_0        = 67,
  opc_fstore_1        = 68,
  opc_fstore_2        = 69,
  opc_fstore_3        = 70,
  opc_dstore_0        = 71,
  opc_dstore_1        = 72,
  opc_dstore_2        = 73,
  opc_dstore_3        = 74,
  opc_astore_0        = 75,
  opc_astore_1        = 76,
  opc_astore_2        = 77,
  opc_astore_3        = 78,
  opc_iastore         = 79,
  opc_lastore         = 80,
  opc_fastore         = 81,
  opc_dastore         = 82,
  opc_aastore         = 83,
  opc_bastore         = 84,
  opc_castore         = 85,
  opc_sastore         = 86,
  opc_pop             = 87,
  opc_pop2            = 88,
  opc_dup             = 89,
  opc_dup_x1          = 90,
  opc_dup_x2          = 91,
  opc_dup2            = 92,
  opc_dup2_x1         = 93,
  opc_dup2_x2         = 94,
  opc_swap            = 95,
  opc_iadd            = 96,
  opc_ladd            = 97,
  opc_fadd            = 98,
  opc_dadd            = 99,
  opc_isub            = 100,
  opc_lsub            = 101,
  opc_fsub            = 102,
  opc_dsub            = 103,
  opc_imul            = 104,
  opc_lmul            = 105,
  opc_fmul            = 106,
  opc_dmul            = 107,
  opc_idiv            = 108,
  opc_ldiv            = 109,
  opc_fdiv            = 110,
  opc_ddiv            = 111,
  opc_irem            = 112,
  opc_lrem            = 113,
  opc_frem            = 114,
  opc_drem            = 115,
  opc_ineg            = 116,
  opc_lneg            = 117,
  opc_fneg            = 118,
  opc_dneg            = 119,
  opc_ishl            = 120,
  opc_lshl            = 121,
  opc_ishr            = 122,
  opc_lshr            = 123,
  opc_iushr           = 124,
  opc_lushr           = 125,
  opc_iand            = 126,
  opc_land            = 127,
  opc_ior             = 128,
  opc_lor             = 129,
  opc_ixor            = 130,
  opc_lxor            = 131,
  opc_iinc            = 132,
  opc_i2l             = 133,
  opc_i2f             = 134,
  opc_i2d             = 135,
  opc_l2i             = 136,
  opc_l2f             = 137,
  opc_l2d             = 138,
  opc_f2i             = 139,
  opc_f2l             = 140,
  opc_f2d             = 141,
  opc_d2i             = 142,
  opc_d2l             = 143,
  opc_d2f             = 144,
  opc_i2b             = 145,
  opc_i2c             = 146,
  opc_i2s             = 147,
  opc_lcmp            = 148,
  opc_fcmpl           = 149,
  opc_fcmpg           = 150,
  opc_dcmpl           = 151,
  opc_dcmpg           = 152,
  opc_ifeq            = 153,
  opc_ifne            = 154,
  opc_iflt            = 155,
  opc_ifge            = 156,
  opc_ifgt            = 157,
  opc_ifle            = 158,
  opc_if_icmpeq       = 159,
  opc_if_icmpne       = 160,
  opc_if_icmplt       = 161,
  opc_if_icmpge       = 162,
  opc_if_icmpgt       = 163,
  opc_if_icmple       = 164,
  opc_if_acmpeq       = 165,
  opc_if_acmpne       = 166,
  opc_goto            = 167,
  opc_jsr             = 168,
  opc_ret             = 169,
  opc_tableswitch     = 170,
  opc_lookupswitch    = 171,
  opc_ireturn         = 172,
  opc_lreturn         = 173,
  opc_freturn         = 174,
  opc_dreturn         = 175,
  opc_areturn         = 176,
  opc_return          = 177,
  opc_getstatic       = 178,
  opc_putstatic       = 179,
  opc_getfield        = 180,
  opc_putfield        = 181,
  opc_invokevirtual   = 182,
  opc_invokespecial   = 183,
  opc_invokestatic    = 184,
  opc_invokeinterface = 185,
  opc_invokedynamic   = 186,
  opc_new             = 187,
  opc_newarray        = 188,
  opc_anewarray       = 189,
  opc_arraylength     = 190,
  opc_athrow          = 191,
  opc_checkcast       = 192,
  opc_instanceof      = 193,
  opc_monitorenter    = 194,
  opc_monitorexit     = 195,
  opc_wide            = 196,
  opc_multianewarray  = 197,
  opc_ifnull          = 198,
  opc_ifnonnull       = 199,
  opc_goto_w          = 200,
  opc_jsr_w           = 201,
  opc_breakpoint      = 202,

// Constant table
  CONSTANT_UTF8       = 1,
  CONSTANT_UNICODE    = 2,
  CONSTANT_INTEGER    = 3,
  CONSTANT_FLOAT      = 4,
  CONSTANT_LONG       = 5,
  CONSTANT_DOUBLE     = 6,
  CONSTANT_CLASS      = 7,
  CONSTANT_STRING     = 8,
  CONSTANT_FIELD      = 9,
  CONSTANT_METHOD     = 10,
  CONSTANT_INTERFACEMETHOD = 11,
  CONSTANT_NAMEANDTYPE    = 12,
  CONSTANT_METHODHANDLE   = 15,
  CONSTANT_METHODTYPE     = 16,
  CONSTANT_INVOKEDYNAMIC  = 18
};

static char *opcNames[] = {
  NTXT ("nop"),
  NTXT ("aconst_null"),
  NTXT ("iconst_m1"),
  NTXT ("iconst_0"),
  NTXT ("iconst_1"),
  NTXT ("iconst_2"),
  NTXT ("iconst_3"),
  NTXT ("iconst_4"),
  NTXT ("iconst_5"),
  NTXT ("lconst_0"),
  NTXT ("lconst_1"),
  NTXT ("fconst_0"),
  NTXT ("fconst_1"),
  NTXT ("fconst_2"),
  NTXT ("dconst_0"),
  NTXT ("dconst_1"),
  NTXT ("bipush"),
  NTXT ("sipush"),
  NTXT ("ldc"),
  NTXT ("ldc_w"),
  NTXT ("ldc2_w"),
  NTXT ("iload"),
  NTXT ("lload"),
  NTXT ("fload"),
  NTXT ("dload"),
  NTXT ("aload"),
  NTXT ("iload_0"),
  NTXT ("iload_1"),
  NTXT ("iload_2"),
  NTXT ("iload_3"),
  NTXT ("lload_0"),
  NTXT ("lload_1"),
  NTXT ("lload_2"),
  NTXT ("lload_3"),
  NTXT ("fload_0"),
  NTXT ("fload_1"),
  NTXT ("fload_2"),
  NTXT ("fload_3"),
  NTXT ("dload_0"),
  NTXT ("dload_1"),
  NTXT ("dload_2"),
  NTXT ("dload_3"),
  NTXT ("aload_0"),
  NTXT ("aload_1"),
  NTXT ("aload_2"),
  NTXT ("aload_3"),
  NTXT ("iaload"),
  NTXT ("laload"),
  NTXT ("faload"),
  NTXT ("daload"),
  NTXT ("aaload"),
  NTXT ("baload"),
  NTXT ("caload"),
  NTXT ("saload"),
  NTXT ("istore"),
  NTXT ("lstore"),
  NTXT ("fstore"),
  NTXT ("dstore"),
  NTXT ("astore"),
  NTXT ("istore_0"),
  NTXT ("istore_1"),
  NTXT ("istore_2"),
  NTXT ("istore_3"),
  NTXT ("lstore_0"),
  NTXT ("lstore_1"),
  NTXT ("lstore_2"),
  NTXT ("lstore_3"),
  NTXT ("fstore_0"),
  NTXT ("fstore_1"),
  NTXT ("fstore_2"),
  NTXT ("fstore_3"),
  NTXT ("dstore_0"),
  NTXT ("dstore_1"),
  NTXT ("dstore_2"),
  NTXT ("dstore_3"),
  NTXT ("astore_0"),
  NTXT ("astore_1"),
  NTXT ("astore_2"),
  NTXT ("astore_3"),
  NTXT ("iastore"),
  NTXT ("lastore"),
  NTXT ("fastore"),
  NTXT ("dastore"),
  NTXT ("aastore"),
  NTXT ("bastore"),
  NTXT ("castore"),
  NTXT ("sastore"),
  NTXT ("pop"),
  NTXT ("pop2"),
  NTXT ("dup"),
  NTXT ("dup_x1"),
  NTXT ("dup_x2"),
  NTXT ("dup2"),
  NTXT ("dup2_x1"),
  NTXT ("dup2_x2"),
  NTXT ("swap"),
  NTXT ("iadd"),
  NTXT ("ladd"),
  NTXT ("fadd"),
  NTXT ("dadd"),
  NTXT ("isub"),
  NTXT ("lsub"),
  NTXT ("fsub"),
  NTXT ("dsub"),
  NTXT ("imul"),
  NTXT ("lmul"),
  NTXT ("fmul"),
  NTXT ("dmul"),
  NTXT ("idiv"),
  NTXT ("ldiv"),
  NTXT ("fdiv"),
  NTXT ("ddiv"),
  NTXT ("irem"),
  NTXT ("lrem"),
  NTXT ("frem"),
  NTXT ("drem"),
  NTXT ("ineg"),
  NTXT ("lneg"),
  NTXT ("fneg"),
  NTXT ("dneg"),
  NTXT ("ishl"),
  NTXT ("lshl"),
  NTXT ("ishr"),
  NTXT ("lshr"),
  NTXT ("iushr"),
  NTXT ("lushr"),
  NTXT ("iand"),
  NTXT ("land"),
  NTXT ("ior"),
  NTXT ("lor"),
  NTXT ("ixor"),
  NTXT ("lxor"),
  NTXT ("iinc"),
  NTXT ("i2l"),
  NTXT ("i2f"),
  NTXT ("i2d"),
  NTXT ("l2i"),
  NTXT ("l2f"),
  NTXT ("l2d"),
  NTXT ("f2i"),
  NTXT ("f2l"),
  NTXT ("f2d"),
  NTXT ("d2i"),
  NTXT ("d2l"),
  NTXT ("d2f"),
  NTXT ("i2b"),
  NTXT ("i2c"),
  NTXT ("i2s"),
  NTXT ("lcmp"),
  NTXT ("fcmpl"),
  NTXT ("fcmpg"),
  NTXT ("dcmpl"),
  NTXT ("dcmpg"),
  NTXT ("ifeq"),
  NTXT ("ifne"),
  NTXT ("iflt"),
  NTXT ("ifge"),
  NTXT ("ifgt"),
  NTXT ("ifle"),
  NTXT ("if_icmpeq"),
  NTXT ("if_icmpne"),
  NTXT ("if_icmplt"),
  NTXT ("if_icmpge"),
  NTXT ("if_icmpgt"),
  NTXT ("if_icmple"),
  NTXT ("if_acmpeq"),
  NTXT ("if_acmpne"),
  NTXT ("goto"),
  NTXT ("jsr"),
  NTXT ("ret"),
  NTXT ("tableswitch"),
  NTXT ("lookupswitch"),
  NTXT ("ireturn"),
  NTXT ("lreturn"),
  NTXT ("freturn"),
  NTXT ("dreturn"),
  NTXT ("areturn"),
  NTXT ("return"),
  NTXT ("getstatic"),
  NTXT ("putstatic"),
  NTXT ("getfield"),
  NTXT ("putfield"),
  NTXT ("invokevirtual"),
  NTXT ("invokespecial"),
  NTXT ("invokestatic"),
  NTXT ("invokeinterface"),
  NTXT ("invokedynamic"),
  NTXT ("new"),
  NTXT ("newarray"),
  NTXT ("anewarray"),
  NTXT ("arraylength"),
  NTXT ("athrow"),
  NTXT ("checkcast"),
  NTXT ("instanceof"),
  NTXT ("monitorenter"),
  NTXT ("monitorexit"),
  NTXT ("wide"),
  NTXT ("multianewarray"),
  NTXT ("ifnull"),
  NTXT ("ifnonnull"),
  NTXT ("goto_w"),
  NTXT ("jsr_w"),
  NTXT ("breakpoint")
};


#define APPEND_FLAG(len, buf, flag, x) \
    if (((x) & (flag)) != 0) \
      { \
	flag &= ~(x); \
	AppendString(len, buf, NTXT("%s%s"), delimiter, #x); \
	delimiter = NTXT("|"); \
      }

static char *
access_flags_to_str (int kind, int flag)
{
  static char buf[256];
  size_t len = 0;
  buf[0] = 0;
  if (flag == 0)
    {
      AppendString (len, buf, NTXT ("0x%x"), (unsigned int) flag);
      return buf;
    }
  const char *delimiter = "";
  if (kind == ClassAccess)
    {
      APPEND_FLAG (len, buf, flag, ACC_FINAL);
      APPEND_FLAG (len, buf, flag, ACC_SUPER);
      APPEND_FLAG (len, buf, flag, ACC_INTERFACE);
      APPEND_FLAG (len, buf, flag, ACC_ABSTRACT);
      APPEND_FLAG (len, buf, flag, ACC_SYNTHETIC);
      APPEND_FLAG (len, buf, flag, ACC_ANNOTATION);
      APPEND_FLAG (len, buf, flag, ACC_ENUM);
      if (flag)
	AppendString (len, buf, "%s0x%x", delimiter, (unsigned int) (flag));
    }
  else if (kind == FieldAccess)
    {
      APPEND_FLAG (len, buf, flag, ACC_PUBLIC);
      APPEND_FLAG (len, buf, flag, ACC_PRIVATE);
      APPEND_FLAG (len, buf, flag, ACC_PROTECTED);
      APPEND_FLAG (len, buf, flag, ACC_STATIC);
      APPEND_FLAG (len, buf, flag, ACC_FINAL);
      APPEND_FLAG (len, buf, flag, ACC_VOLATILE);
      APPEND_FLAG (len, buf, flag, ACC_TRANSIENT);
      APPEND_FLAG (len, buf, flag, ACC_SYNTHETIC);
      APPEND_FLAG (len, buf, flag, ACC_ENUM);
      if (flag)
	AppendString (len, buf, "%s0x%x", delimiter, (unsigned int) (flag));
    }
  else if (kind == MethodAccess)
    {
      APPEND_FLAG (len, buf, flag, ACC_PUBLIC);
      APPEND_FLAG (len, buf, flag, ACC_PRIVATE);
      APPEND_FLAG (len, buf, flag, ACC_PROTECTED);
      APPEND_FLAG (len, buf, flag, ACC_STATIC);
      APPEND_FLAG (len, buf, flag, ACC_FINAL);
      APPEND_FLAG (len, buf, flag, ACC_SYNCHRONIZED);
      APPEND_FLAG (len, buf, flag, ACC_BRIDGE);
      APPEND_FLAG (len, buf, flag, ACC_VARARGS);
      APPEND_FLAG (len, buf, flag, ACC_NATIVE);
      APPEND_FLAG (len, buf, flag, ACC_ABSTRACT);
      APPEND_FLAG (len, buf, flag, ACC_STRICT);
      APPEND_FLAG (len, buf, flag, ACC_SYNTHETIC);
      if (flag)
	AppendString (len, buf, "%s0x%x", delimiter, (unsigned int) (flag));
    }
  else if (kind == NestedClassAccess)
    {
      APPEND_FLAG (len, buf, flag, ACC_PUBLIC);
      APPEND_FLAG (len, buf, flag, ACC_PRIVATE);
      APPEND_FLAG (len, buf, flag, ACC_PROTECTED);
      APPEND_FLAG (len, buf, flag, ACC_STATIC);
      APPEND_FLAG (len, buf, flag, ACC_FINAL);
      APPEND_FLAG (len, buf, flag, ACC_INTERFACE);
      APPEND_FLAG (len, buf, flag, ACC_ABSTRACT);
      APPEND_FLAG (len, buf, flag, ACC_SYNTHETIC);
      APPEND_FLAG (len, buf, flag, ACC_ANNOTATION);
      APPEND_FLAG (len, buf, flag, ACC_ENUM);
      if (flag)
	AppendString (len, buf, "%s0x%x", delimiter, (unsigned int) (flag));
    }
  return buf;
}

class DataReadException
{
public:

  DataReadException (char *s)
  {
    str_err = s;
  }

  ~DataReadException ()
  {
    free (str_err);
  }

  char *
  toString ()
  {
    return str_err;
  }

private:
  char *str_err;
};

class DataInputStream
{
public:

  DataInputStream (const unsigned char *bytes, int64_t sz)
  {
    bp = bp_orig = bytes;
    bp_last = bp_orig + sz;
  }

  DataInputStream (DataInputStream *in)
  {
    bp = bp_orig = in->bp_orig;
    bp_last = in->bp_last;
  }

  u1
  readByte ()
  {
    check (1);
    u1 val = *bp;
    bp++;
    return val;
  }

  u2
  readUnsignedShort ()
  {
    check (2);
    u2 val = (bp[0] << 8) | bp[1];
    bp += 2;
    return val;
  }

  u4
  readUnsigned ()
  {
    check (4);
    u4 val = (bp[0] << 24) | (bp[1] << 16) | (bp[2] << 8) | bp[3];
    bp += 4;
    return val;
  }

  const u1 *
  getptr ()
  {
    return bp;
  }

  const size_t
  get_offset ()
  {
    return bp - bp_orig;
  }

  void
  skip (int n)
  {
    check (n);
    bp += n;
  }

  void
  reset ()
  {
    bp = bp_orig;
  }

  void
  copy_bytes (char *buf, int64_t len)
  {
    check (len);
    memcpy (buf, bp, len);
    buf[len] = '\0';
  }

private:

  void
  check (int64_t sz)
  {
    if (sz < 0 || bp + sz > bp_last)
      {
	DataReadException *e1 = new DataReadException (
	       dbe_sprintf (GTXT ("(Cannot read %lld byte(s) offset=0x%llx)\n"),
			    (long long) sz, (long long) get_offset ()));
	throw (e1);
      }
  };

  const unsigned char *bp_last;
  const unsigned char *bp_orig;
  const unsigned char *bp;
};

class BinaryConstantPool
{
public:
  BinaryConstantPool (DataInputStream &in);
  ~BinaryConstantPool ();

  u1
  getType (int n)
  {
    return (n < nconst && n > 0) ? types[n] : 0;
  };
  char *getString (int index);

private:
  static char *getTypeName (int ty);
  static char *type_name_to_str (int ty);
  static char *offset_to_str (long long offset);
  int nconst;
  u1 *types;
  int64_t *offsets;
  char **strings;
  DataInputStream *input;
};

char *
BinaryConstantPool::type_name_to_str (int ty)
{
  static char buf[128];
  char *tyName = getTypeName (ty);
  snprintf (buf, sizeof (buf), NTXT ("%s(%d)"), tyName, ty);
  return buf;
}

char *
BinaryConstantPool::offset_to_str (long long offset)
{
  static char buf[128];
  snprintf (buf, sizeof (buf), NTXT ("offset=0x%06llx (%llu)"), offset, offset);
  return buf;
}

BinaryConstantPool::BinaryConstantPool (DataInputStream &in)
{
  nconst = 0;
  types = NULL;
  offsets = NULL;
  strings = NULL;
  input = new DataInputStream (in);
  int cntConst = in.readUnsignedShort ();
  if (cntConst > 0)
    {
      types = new u1[cntConst];
      types[0] = 0;
      offsets = new int64_t [cntConst];
      strings = new char * [cntConst];
      strings[0] = NULL;
    }
  Dprintf (DUMP_JAVA_CLASS, NTXT ("# BinaryConstantPool: %d\n"), (int) nconst);
  for (int i = 1; i < cntConst; i++)
    {
      nconst = i + 1;
      strings[i] = NULL;
      types[i] = in.readByte ();
      offsets[i] = in.get_offset ();
      Dprintf (DUMP_JAVA_CLASS, NTXT (" %3d %-25s %-25s"), i, offset_to_str (offsets[i]), type_name_to_str (types[i]));
      switch (types[i])
	{
	case CONSTANT_UTF8:
	  {
	    u2 length = in.readUnsignedShort ();
	    in.skip (length);
	    Dprintf (DUMP_JAVA_CLASS, " length=%u\n", (unsigned int) length);
	    break;
	  }
	case CONSTANT_INTEGER:
	  {
	    u4 bytes = in.readUnsigned ();
	    Dprintf (DUMP_JAVA_CLASS, " bytes=0x%08x\n", (unsigned int) bytes);
	    break;
	  }
	case CONSTANT_FLOAT:
	  {
	    u4 bytes = in.readUnsigned ();
	    Dprintf (DUMP_JAVA_CLASS, " bytes=0x%08x\n", (unsigned int) bytes);
	    break;
	  }
	case CONSTANT_LONG:
	case CONSTANT_DOUBLE:
	  {
	    // JVM 4.4.5: all 8-byte constants take up
	    // two entries in the constant_pool table.
	    i++;
	    nconst++;
	    offsets[i] = 0;
	    strings[i] = NULL;
	    u4 high_bytes = in.readUnsigned ();
	    u4 low_bytes = in.readUnsigned ();
	    Dprintf (DUMP_JAVA_CLASS, NTXT (" high_bytes=0x%08x  low_bytes=0x%08x\n"),
		     (unsigned int) high_bytes, (unsigned int) low_bytes);
	    break;
	  }
	case CONSTANT_CLASS:
	  {
	    u2 name_index = in.readUnsignedShort ();
	    Dprintf (DUMP_JAVA_CLASS, NTXT (" name_index=%6u\n"), (unsigned int) name_index);
	    break;
	  }
	case CONSTANT_STRING:
	  {
	    u2 string_index = in.readUnsignedShort ();
	    Dprintf (DUMP_JAVA_CLASS, NTXT (" string_index=%4u\n"), (unsigned int) string_index);
	    break;
	  }
	case CONSTANT_FIELD:
	case CONSTANT_METHOD:
	case CONSTANT_INTERFACEMETHOD:
	  {
	    u2 class_index = in.readUnsignedShort ();
	    u2 name_and_type_index = in.readUnsignedShort ();
	    Dprintf (DUMP_JAVA_CLASS, NTXT (" class_index=%5u  name_and_type_index=%u\n"),
		     (unsigned int) class_index, (unsigned int) name_and_type_index);
	    break;
	  }
	case CONSTANT_NAMEANDTYPE:
	  {
	    u2 name_index = in.readUnsignedShort ();
	    u2 descriptor_index = in.readUnsignedShort ();
	    Dprintf (DUMP_JAVA_CLASS, " name_index=%6u  descriptor_index=%u\n",
		    (unsigned int) name_index, (unsigned int) descriptor_index);
	    break;
	  }
	case CONSTANT_METHODHANDLE:
	  {
	    u1 reference_kind = in.readByte ();
	    u2 reference_index = in.readUnsignedShort ();
	    Dprintf (DUMP_JAVA_CLASS, " reference_kind=%u reference_index=%u\n",
		 (unsigned int) reference_kind, (unsigned int) reference_index);
	    break;
	  }
	case CONSTANT_METHODTYPE:
	  {
	    u2 descriptor_index = in.readUnsignedShort ();
	    Dprintf (DUMP_JAVA_CLASS, NTXT (" descriptor_index=%u\n"),
		     (unsigned int) descriptor_index);
	    break;
	  }
	case CONSTANT_INVOKEDYNAMIC:
	  {
	    u2 bootstrap_method_attr_index = in.readUnsignedShort ();
	    u2 name_and_type_index = in.readUnsignedShort ();
	    Dprintf (DUMP_JAVA_CLASS, NTXT (" bootstrap_method_attr_index=%5u  name_and_type_index=%u\n"),
		     (unsigned int) bootstrap_method_attr_index,
		     (unsigned int) name_and_type_index);
	    break;
	  }
	default:
	  Dprintf (DUMP_JAVA_CLASS, NTXT ("\n"));
	  DataReadException *e1 = new DataReadException (
		  dbe_sprintf (GTXT ("BinaryConstantPool[%d]: bad tag %d %s\n"),
			       i, types[i], offset_to_str (offsets[i])));
	  throw (e1);
	}
    }
}

BinaryConstantPool::~BinaryConstantPool ()
{
  delete[] types;
  delete[] offsets;
  delete input;
  if (strings)
    {
      for (int i = 0; i < nconst; i++)
	free (strings[i]);
      delete[] strings;
    }
}

#define CASE_S(x)   case x: return (char *) #x

char *
BinaryConstantPool::getTypeName (int ty)
{
  switch (ty)
    {
      CASE_S (CONSTANT_UTF8);
      CASE_S (CONSTANT_INTEGER);
      CASE_S (CONSTANT_FLOAT);
      CASE_S (CONSTANT_LONG);
      CASE_S (CONSTANT_DOUBLE);
      CASE_S (CONSTANT_CLASS);
      CASE_S (CONSTANT_STRING);
      CASE_S (CONSTANT_FIELD);
      CASE_S (CONSTANT_METHOD);
      CASE_S (CONSTANT_INTERFACEMETHOD);
      CASE_S (CONSTANT_NAMEANDTYPE);
      CASE_S (CONSTANT_METHODHANDLE);
      CASE_S (CONSTANT_METHODTYPE);
      CASE_S (CONSTANT_INVOKEDYNAMIC);
    default: return NTXT ("UNKNOWN_TYPE");
    }
}

char *
BinaryConstantPool::getString (int index)
{
  if (index >= nconst || index <= 0)
    return NULL;
  if (strings[index])
    return strings[index];
  input->reset ();
  input->skip (offsets[index]);
  switch (types[index])
    {
    case CONSTANT_CLASS:
    case CONSTANT_STRING:
    case CONSTANT_NAMEANDTYPE:
      strings[index] = dbe_strdup (getString (input->readUnsignedShort ()));
      return strings[index];
    case CONSTANT_METHOD:
      input->readUnsignedShort (); // cl_inx
      strings[index] = dbe_strdup (getString (input->readUnsignedShort ()));
      return strings[index];
    case CONSTANT_UTF8:
      break;
    default:
      return NULL;
    }
  u2 len = input->readUnsignedShort ();
  strings[index] = (char *) malloc (len + 1);
  input->copy_bytes (strings[index], len);
  return strings[index];
}

ClassFile::ClassFile () : Module ()
{
  input = NULL;
  bcpool = NULL;
  cf_buf = NULL;
  cur_jmthd = NULL;
  blanksCnt = 0;
  cf_bufsz = 0;
  lang_code = Sp_lang_java;
  class_name = NULL;
  class_filename = NULL;
  source_name = NULL;
  byteCodeInfo = NULL;
}

char *
ClassFile::get_opc_name (int op)
{
  if (op >= 0 && ((size_t) op) < sizeof (opcNames) / sizeof (char*))
    return opcNames[op];
  switch (op)
    {
    case opc_try:
      return NTXT ("try");
    case opc_dead:
      return NTXT ("dead");
    case opc_label:
      return NTXT ("label");
    default:
      return NTXT ("Unknown op code");
    }
}

void
ClassFile::openFile (const char *fname)
{
  if (fname == NULL)
    return;
  int fd = open64 (fname, O_RDONLY);
  if (fd == -1)
    {
      append_msg (CMSG_ERROR, GTXT ("Cannot open file %s"), fname);
      return;
    }
  struct stat64 stat_buf;
  if ((fstat64 (fd, &stat_buf) == -1) || (stat_buf.st_size == 0))
    {
      close (fd);
      append_msg (CMSG_ERROR, GTXT ("Cannot read file %s"), fname);
      return;
    }
  cf_bufsz = stat_buf.st_size;
  cf_buf = (unsigned char *) malloc (cf_bufsz);
  if (cf_bufsz != read_from_file (fd, cf_buf, cf_bufsz))
    {
      free (cf_buf);
      cf_buf = NULL;
      close (fd);
      append_msg (CMSG_ERROR, GTXT ("Cannot read file %s"), fname);
      return;
    }
  close (fd);

  input = new DataInputStream (cf_buf, cf_bufsz);
  u4 c_magic = input->readUnsigned ();
  if (c_magic != JAVA_MAGIC)
    {
      append_msg (CMSG_ERROR, GTXT ("Not a class file: %s"), fname);
      return;
    }
  /* u2 minor = */ input->readUnsignedShort ();
  /* u2 major = */ input->readUnsignedShort ();
  status = AE_OK;
}

ClassFile::~ClassFile ()
{
  free (cf_buf);
  free (class_name);
  free (class_filename);
  free (source_name);
  delete bcpool;
  delete input;
}

static void
convertName (char *s)
{
  while (*s)
    {
      if (*s == '/')
	*s = '.';
      s++;
    }
}

void
ClassFile::printConstant (StringBuilder *sb, int index)
{
  u1 type = bcpool->getType (index);
  switch (type)
    {
    case CONSTANT_METHOD:
      {
	char *str = bcpool->getString (index);
	if (str)
	  {
	    convertName (str);
	    sb->append (str);
	    sb->append (NTXT ("()"));
	  }
	break;
      }
    case CONSTANT_CLASS:
      {
	char *str = bcpool->getString (index);
	if (str)
	  {
	    convertName (str);
	    sb->append (str);
	  }
	break;
      }
    case CONSTANT_UTF8:
      {
	char *str = bcpool->getString (index);
	if (str)
	  sb->append (str);
	break;
      }
    case CONSTANT_STRING:
      {
	char *str = bcpool->getString (index);
	if (str)
	  {
	    sb->append ('"');
	    sb->append (str);
	    sb->append ('"');
	  }
	break;
      }
    default:
      sb->append ('#');
      sb->append ((int) index);
      break;
    }
}

long long
ClassFile::printCodeSequence (StringBuilder *sb, uint64_t addr, DataInputStream *in)
{
  int64_t offset = in->get_offset ();
  sb->appendf (NTXT ("%08llx: "), (long long) addr);
  int opcode = in->readByte ();
  if (opcode == opc_wide)
    {
      opcode = in->readByte ();
      sb->append (get_opc_name (opcode));
      sb->append (NTXT ("_w "));
      int arg = in->readUnsignedShort ();
      switch (opcode)
	{
	case opc_aload: case opc_astore:
	case opc_fload: case opc_fstore:
	case opc_iload: case opc_istore:
	case opc_lload: case opc_lstore:
	case opc_dload: case opc_dstore:
	case opc_ret:
	  sb->append (arg);
	  break;
	case opc_iinc:
	  sb->append (arg);
	  sb->append (' ');
	  sb->append (in->readUnsignedShort ());
	  break;
	default:
	  sb->append (GTXT ("Invalid opcode"));
	  break;
	}
    }
  else
    {
      sb->append (get_opc_name (opcode));
      sb->append (' ');
      switch (opcode)
	{
	case opc_aload: case opc_astore:
	case opc_fload: case opc_fstore:
	case opc_iload: case opc_istore:
	case opc_lload: case opc_lstore:
	case opc_dload: case opc_dstore:
	case opc_ret:
	  sb->append (in->readByte ());
	  break;
	case opc_iinc:
	  sb->append (in->readByte ());
	  sb->append (' ');
	  sb->append (in->readByte ());
	  break;
	case opc_tableswitch:
	  {
	    int align = (addr + 1) % 4; // 1 byte is a length of opc_lookupswitch
	    if (align != 0)
	      {
		in->skip (4 - align); // four byte boundry
	      }
	    long default_skip = in->readUnsigned ();
	    long low = in->readUnsigned ();
	    long high = in->readUnsigned ();
	    sb->appendf (GTXT ("%ld to %ld: default=0x%llx"),
		    (long) low, (long) high, (long long) (addr + default_skip));
	    for (long i = low; i <= high; ++i)
	      /* u4 i1 = */ in->readUnsigned ();
	    break;
	  }
	case opc_lookupswitch:
	  {
	    int align = (addr + 1) % 4; // 1 byte is a length of opc_lookupswitch
	    if (align != 0)
	      in->skip (4 - align); // four byte boundry
	    u4 default_skip = in->readUnsigned ();
	    u4 npairs = in->readUnsigned ();
	    sb->appendf (GTXT ("%d: default=0x%llx"), npairs,
			 (long long) (addr + default_skip));
	    for (int i = 0, nints = npairs * 2; i < nints; i += 2)
	      {
		/* u4 i1 = */ in->readUnsigned ();
		/* u4 i2 = */ in->readUnsigned ();
	      }
	    break;
	  }
	case opc_newarray:
	  switch (in->readByte ())
	    {
	    case T_INT:
	      sb->append (GTXT ("int"));
	      break;
	    case T_LONG:
	      sb->append (GTXT ("long"));
	      break;
	    case T_FLOAT:
	      sb->append (GTXT ("float"));
	      break;
	    case T_DOUBLE:
	      sb->append (GTXT ("double"));
	      break;
	    case T_CHAR:
	      sb->append (GTXT ("char"));
	      break;
	    case T_SHORT:
	      sb->append (GTXT ("short"));
	      break;
	    case T_BYTE:
	      sb->append (GTXT ("byte"));
	      break;
	    case T_BOOLEAN:
	      sb->append (GTXT ("boolean"));
	      break;
	    default:
	      sb->append (GTXT ("BOGUS TYPE"));
	      break;
	    }
	  break;
	case opc_anewarray:
	  sb->append (GTXT ("class "));
	  printConstant (sb, in->readUnsignedShort ());
	  break;
	case opc_sipush:
	  sb->append (in->readUnsignedShort ());
	  break;
	case opc_bipush:
	  sb->append (in->readByte ());
	  break;
	case opc_ldc:
	  printConstant (sb, in->readByte ());
	  break;
	case opc_ldc_w: case opc_ldc2_w:
	case opc_instanceof: case opc_checkcast:
	case opc_new:
	case opc_putstatic: case opc_getstatic:
	case opc_putfield: case opc_getfield:
	case opc_invokevirtual:
	case opc_invokespecial:
	case opc_invokestatic:
	  printConstant (sb, in->readUnsignedShort ());
	  break;
	case opc_invokeinterface:
	  {
	    u2 index = in->readUnsignedShort ();
	    u1 count = in->readByte ();
	    /* u1 zero = */ in->readByte ();
	    sb->appendf (" #%u, %u) ", (unsigned int) index, (unsigned int) count);
	    printConstant (sb, index);
	    break;
	  }
	case opc_multianewarray:
	  {
	    u2 index = in->readUnsignedShort ();
	    printConstant (sb, index);
	    sb->appendf (GTXT (" dim #%d "), index);
	    break;
	  }
	case opc_jsr: case opc_goto:
	case opc_ifeq: case opc_ifge: case opc_ifgt:
	case opc_ifle: case opc_iflt: case opc_ifne:
	case opc_if_icmpeq: case opc_if_icmpne: case opc_if_icmpge:
	case opc_if_icmpgt: case opc_if_icmple: case opc_if_icmplt:
	case opc_if_acmpeq: case opc_if_acmpne:
	case opc_ifnull: case opc_ifnonnull:
	  sb->appendf (NTXT ("0x%llx"), (long long) (addr + (short) in->readUnsignedShort ()));
	  break;
	case opc_jsr_w:
	case opc_goto_w:
	  sb->append (addr + (int) in->readUnsigned ());
	  break;
	default:
	  break;
	}
    }
  return in->get_offset () - offset;
}

void
ClassFile::readAttributes (int count)
{
  blanksCnt += 4;
  for (int ax = 0; ax < count; ax++)
    {
      u2 attribute_name_index = input->readUnsignedShort ();
      u4 attribute_length = input->readUnsigned ();
      char *attribute_name = bcpool->getString (attribute_name_index);
      if (!attribute_name)
	{
	  Dprintf (DUMP_JAVA_CLASS, NTXT ("%*c  %2d: attr_name=%3d %-15s len=%4d\n"),
		   (int) blanksCnt, ' ', (int) (ax + 1),
		   (int) attribute_name_index, STR (attribute_name), (int) attribute_length);
	  input->skip (attribute_length);
	  continue;
	}

      if (strcmp (attribute_name, NTXT ("SourceFile")) == 0)
	{
	  u2 sourcefile_index = input->readUnsignedShort ();
	  source_name = dbe_strdup (bcpool->getString (sourcefile_index));
	  Dprintf (DUMP_JAVA_CLASS, NTXT ("%*c  %2d: attr_name=%3d %-15s len=%4d file_name=%d %s\n"),
		   (int) blanksCnt, ' ', (int) (ax + 1),
		   (int) attribute_name_index, STR (attribute_name), (int) attribute_length,
		   (int) sourcefile_index, STR (source_name));
	}
      else if (strcmp (attribute_name, NTXT ("InnerClasses")) == 0)
	{
	  int niclasses = input->readUnsignedShort ();
	  for (int ix = 0; ix < niclasses; ix++)
	    {
	      u2 inner_class_info_index = input->readUnsignedShort ();
	      u2 outer_class_info_index = input->readUnsignedShort ();
	      u2 inner_name_index = input->readUnsignedShort ();
	      u2 inner_class_access_flags = input->readUnsignedShort ();
	      Dprintf (DUMP_JAVA_CLASS,
		       NTXT ("%*c  %2d: attr_name=%3d %-15s len=%4d name=%d '%s'\n"
			     "%*cinner_class_info_index=%d outer_class_info_index=%d flags=%s\n"),
		       (int) blanksCnt, ' ', (int) (ax + 1),
		       (int) attribute_name_index, STR (attribute_name), (int) attribute_length,
		       (int) inner_name_index, STR (bcpool->getString (inner_name_index)),
		       (int) (blanksCnt + 10), ' ',
		       (int) inner_class_info_index, (int) outer_class_info_index,
		       access_flags_to_str (NestedClassAccess, inner_class_access_flags));
	    }
	}
      else if (strcmp (attribute_name, NTXT ("Code")) == 0)
	{
	  u2 max_stack = input->readUnsignedShort ();
	  u2 max_locals = input->readUnsignedShort ();
	  u4 code_length = input->readUnsigned ();
	  if (cur_jmthd)
	    {
	      cur_jmthd->size = code_length;
	      cur_jmthd->img_fname = dbeFile->get_location ();
	      cur_jmthd->img_offset = input->get_offset ();
	    }
	  input->skip (code_length);
	  u2 exception_table_length = input->readUnsignedShort ();
	  input->skip (exception_table_length * (2 + 2 + 2 + 2));
	  Dprintf (DUMP_JAVA_CLASS,
		   NTXT ("%*c  %2d: attr_name=%3d %-15s len=%4d max_stack=%d max_locals=%d code_length=%d exception_table_length=%d\n"),
		   (int) blanksCnt, ' ', (int) (ax + 1),
		   (int) attribute_name_index, STR (attribute_name), (int) attribute_length,
		   (int) max_stack, (int) max_locals, (int) code_length, (int) exception_table_length);
	  readAttributes (input->readUnsignedShort ());
	}
      else if (strcmp (attribute_name, NTXT ("LineNumberTable")) == 0)
	{
	  int nlines = input->readUnsignedShort ();
	  Dprintf (DUMP_JAVA_CLASS, NTXT ("%*c  %2d: attr_name=%3d %-15s len=%4d nlines=%d\n"),
		   (int) blanksCnt, ' ', (int) (ax + 1),
		   (int) attribute_name_index, STR (attribute_name), (int) attribute_length,
		   (int) nlines);
	  for (int lx = 0; lx < nlines; lx++)
	    {
	      int bci = input->readUnsignedShort ();
	      int lno = input->readUnsignedShort ();
	      Dprintf (DUMP_JAVA_CLASS, NTXT ("%*c  %3d: pc=%4d (0x%04x)   line=%d\n"),
		       (int) (blanksCnt + 5), ' ', (int) (lx + 1), (int) bci, (int) bci, (int) lno);
	      if (cur_jmthd)
		byteCodeInfo->append (new ByteCodeInfo (cur_jmthd, bci, lno));
	    }
	}
      else
	{
	  Dprintf (DUMP_JAVA_CLASS, NTXT ("%*c  %2d: attr_name=%3d %-15s len=%4d\n"),
		   (int) blanksCnt, ' ', (int) (ax + 1),
		   (int) attribute_name_index, STR (attribute_name),
		   (int) attribute_length);
	  input->skip (attribute_length);
	}
    }
  blanksCnt -= 4;
}

int
ClassFile::readFile ()
{
  if (status != AE_NOTREAD)
    return status;
  status = AE_OTHER;

  // The ClassFile Structure http://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html
  try
    {
      blanksCnt = 4;
      cur_jmthd = NULL;
      char *fname = dbeFile->get_location ();
      openFile (fname);
      Dprintf (DUMP_JAVA_CLASS, NTXT ("\nClassFile::readFile status=%d %s location=%s\n"),
	       (unsigned int) status, STR (get_name ()), STR (fname));
      if (status != AE_OK)
	return status;
      byteCodeInfo = new Vector<ByteCodeInfo *>(512);
      bcpool = new BinaryConstantPool (*input);
      u2 access_flags = input->readUnsignedShort ();
      Dprintf (DUMP_JAVA_CLASS, NTXT ("\naccess_flags=%s; %s\n"),
	       access_flags_to_str (ClassAccess, access_flags),
	       STR (dbeFile->get_name ()));
      u2 classNameInd = input->readUnsignedShort ();
      class_filename = dbe_strdup (bcpool->getString (classNameInd));
      if (class_filename)
	{
	  class_name = strdup (class_filename);
	  convertName (class_name);
	}

      // Get superclass name
      u2 superClassInd = input->readUnsignedShort ();
      //char *str = bcpool->getString(superClassInd);
      //super_name = str ? convertName( str ) : NULL;

      // Read interfaces
      int interfaces_count = input->readUnsignedShort ();
      Dprintf (DUMP_JAVA_CLASS,
	       NTXT ("  class_name=%3d %-20s  superClass=%3d %s  interfaces_count=%d\n"),
	       (int) classNameInd, STR (class_name),
	       (int) superClassInd, STR (bcpool->getString (superClassInd)),
	       (int) interfaces_count);
      for (int i = 0; i < interfaces_count; i++)
	{
	  u2 index = input->readUnsignedShort ();
	  Dprintf (DUMP_JAVA_CLASS, NTXT ("  %6lld%s"), (long long) index,
		   (i % 8 == 7) || (i + 1 == interfaces_count) ? "\n" : "");
	}

      // Read fields
      int fields_count = input->readUnsignedShort ();
      Dprintf (DUMP_JAVA_CLASS, NTXT ("  fields_count=%d\n"), fields_count);
      for (int i = 0; i < fields_count; i++)
	{
	  u2 fld_access_flags = input->readUnsignedShort ();
	  u2 name_index = input->readUnsignedShort ();
	  u2 descriptor_index = input->readUnsignedShort ();
	  u2 attributes_count = input->readUnsignedShort ();
	  Dprintf (DUMP_JAVA_CLASS,
		   NTXT ("    %2d: name=%3d %-20s flags=%s; desc_ind=%d attr_count=%d\n"),
		   i, (int) name_index, STR (bcpool->getString (name_index)),
		   access_flags_to_str (FieldAccess, fld_access_flags),
		   (int) descriptor_index, (int) attributes_count);
	  readAttributes (attributes_count);
	}

      // Read methods
      int methods_count = input->readUnsignedShort ();
      Dprintf (DUMP_JAVA_CLASS, NTXT ("\n  methods_count=%d\n"), (int) methods_count);
      int func_cnt = functions->size ();
      for (int i = 0; i < methods_count; i++)
	{
	  u2 mthd_access_flags = input->readUnsignedShort ();
	  u2 name_index = input->readUnsignedShort ();
	  u2 descriptor_index = input->readUnsignedShort ();
	  char *mname = bcpool->getString (name_index);
	  if (mname == NULL)
	    {
	      DataReadException *e1 = new DataReadException (dbe_sprintf (GTXT ("method name[%d] is NULL\n"), i));
	      throw (e1);
	    }
	  char *msign = bcpool->getString (descriptor_index);
	  if (msign == NULL)
	    {
	      DataReadException *e1 = new DataReadException (dbe_sprintf (GTXT ("method signature[%d] is NULL\n"), i));
	      throw (e1);
	    }
	  size_t len = strlen (class_name);
	  cur_jmthd = NULL;
	  for (int idx = 0; idx < func_cnt; idx++)
	    {
	      JMethod *jmthd = (JMethod*) functions->fetch (idx);
	      char *jmt_name = jmthd->get_name (Histable::SHORT);
	      if (strncmp (jmt_name, class_name, len) == 0)
		{
		  if (strcmp (jmt_name + len + 1, mname) == 0 &&
		      strcmp (jmthd->get_signature (), msign) == 0)
		    {
		      cur_jmthd = jmthd;
		      break;
		    }
		}
	    }
	  if (cur_jmthd == NULL)
	    {
	      cur_jmthd = dbeSession->createJMethod ();
	      cur_jmthd->module = this;
	      cur_jmthd->set_signature (dbe_strdup (msign));
	      char *nm = dbe_sprintf (NTXT ("%s.%s"), class_name, mname);
	      cur_jmthd->set_name (nm);
	      free (nm);
	      functions->append (cur_jmthd);
	    }
	  if ((mthd_access_flags & ACC_NATIVE) != 0)
	    {
	      cur_jmthd->flags |= FUNC_FLAG_NATIVE;
	    }
	  u2 attributes_count = input->readUnsignedShort ();
	  Dprintf (DUMP_JAVA_CLASS,
		   NTXT ("    %2d: name=%d %-20s  flags=%s  desc_ind=%d attr_count=%d\n"),
		   (int) (i + 1), (int) name_index, STR (bcpool->getString (name_index)),
		   access_flags_to_str (MethodAccess, mthd_access_flags),
		   (int) descriptor_index, (int) attributes_count);
	  readAttributes (attributes_count);
	  cur_jmthd->popSrcFile ();
	}

      // Read global attributes
      u2 global_attributes_count = input->readUnsignedShort ();
      Dprintf (DUMP_JAVA_CLASS, NTXT ("  global_attributes_count=%d\n"), global_attributes_count);
      readAttributes (global_attributes_count);
      status = AE_OK;
    }
  catch (DataReadException *ex)
    {
      append_msg (CMSG_ERROR, GTXT ("Cannot read class file %s (%s)"), get_name (), ex->toString ());
      delete ex;
      status = AE_OTHER;
    }

  char *fnm = NULL;
  if (class_filename)
    {
      if (strcmp (class_filename, get_name ()) != 0)
	set_name (strdup (class_filename));
      if (source_name)
	{
	  char *bname = strrchr (class_filename, '/');
	  if (bname)
	    fnm = dbe_sprintf (NTXT ("%.*s/%s"), (int) (bname - class_filename),
			       class_filename, source_name);
	  else
	    fnm = strdup (source_name);
	}
      else
	fnm = get_java_file_name (class_filename, false);
    }
  else if (source_name)
    fnm = strdup (source_name);
  if (fnm)
    {
      set_file_name (fnm);
      main_source = findSource (file_name, true);
      main_source->dbeFile->filetype |= DbeFile::F_JAVA_SOURCE;
    }

  for (long i = 0, sz = VecSize (functions); i < sz; i++)
    functions->get (i)->def_source = main_source;
  JMethod *func = NULL;
  for (long i = 0, sz = VecSize (byteCodeInfo); i < sz; i++)
    {
      ByteCodeInfo *p = byteCodeInfo->get (i);
      if (func != p->func)
	{
	  if (func)
	    func->popSrcFile ();
	  func = p->func;
	  func->line_first = p->lno;
	  func->pushSrcFile (main_source, 0);
	}
      func->line_last = p->lno;
      func->add_PC_info (p->bci, p->lno, main_source);
    }
  if (func)
    func->popSrcFile ();
  Destroy (byteCodeInfo);
  Dprintf (DUMP_JAVA_CLASS, NTXT ("\n status=%d class_filename=%s class_name=%s source_name=%s file_name=%s %s\n"),
	   (unsigned int) status, STR (class_filename), STR (class_name),
	   STR (source_name), STR (file_name),
	   STR (get_name ()));
  return status;
}

#define MAX_CLASS_SIZE 65536

char *
ClassFile::get_disasm (uint64_t inst_address, uint64_t end_address,
		       uint64_t start_address, uint64_t f_offset, int64_t &inst_size)
{
  int64_t offset = f_offset + (inst_address - start_address);
  if ((cf_buf == NULL) || (inst_address >= end_address) || (offset >= cf_bufsz))
    {
      inst_size = 0;
      return NULL;
    }

  // Check for an implausibly large size
  if ((inst_address - start_address) > MAX_CLASS_SIZE)
    {
      append_msg (CMSG_ERROR, GTXT ("Cannot disassemble class file %s (%s), implausible size = %lld"),
		  get_name (), dbeFile->get_location (),
		  (end_address - start_address));
      inst_size = 0;
      return NULL;
    }

  StringBuilder sb;
  DataInputStream *in = new DataInputStream (input);
  try
    {
      in->skip (offset);
      inst_size = printCodeSequence (&sb, inst_address - start_address, in);
    }
  catch (DataReadException *ex)
    {
      append_msg (CMSG_ERROR, GTXT ("Cannot disassemble class file %s (%s) %s"),
		  get_name (), dbeFile->get_location (), ex->toString ());
      delete ex;
      inst_size = 0;
    }
  delete in;
  if (inst_size == 0)
    return NULL;
  return sb.toString ();
}

char *
ClassFile::get_java_file_name (char *clname, bool classSuffix)
{
  size_t len = strlen (clname);
  if (len > 6 && streq (clname + len - 6, NTXT (".class")))
    len -= 6;
  if (!classSuffix)
    { // remove $SubClassName from "ClassName$SubClassName"
      char *tmp = strchr (clname, '$');
      if (tmp)
	len = tmp - clname;
    }
  char *clpath = (char *) malloc (len + 10);
  for (size_t i = 0; i < len; i++)
    clpath[i] = (clname[i] == '.') ? '/' : clname[i];
  snprintf (clpath + len, 10, classSuffix ? NTXT (".class") : NTXT (".java"));
  return clpath;
}
