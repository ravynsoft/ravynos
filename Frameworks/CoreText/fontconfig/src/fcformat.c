/*
 * Copyright © 2008,2009 Red Hat, Inc.
 *
 * Red Hat Author(s): Behdad Esfahbod
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fcint.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


/* The language is documented in doc/fcformat.fncs
 * These are the features implemented:
 *
 * simple	%{elt}
 * width	%width{elt}
 * index	%{elt[idx]}
 * name=	%{elt=}
 * :name=	%{:elt}
 * default	%{elt:-word}
 * count	%{#elt}
 * subexpr	%{{expr}}
 * filter-out	%{-elt1,elt2,elt3{expr}}
 * filter-in	%{+elt1,elt2,elt3{expr}}
 * conditional	%{?elt1,elt2,!elt3{}{}}
 * enumerate	%{[]elt1,elt2{expr}}
 * langset	langset enumeration using the same syntax
 * builtin	%{=blt}
 * convert	%{elt|conv1|conv2|conv3}
 *
 * converters:
 * basename	FcStrBasename
 * dirname	FcStrDirname
 * downcase	FcStrDowncase
 * shescape
 * cescape
 * xmlescape
 * delete	delete chars
 * escape	escape chars
 * translate	translate chars
 *
 * builtins:
 * unparse	FcNameUnparse
 * fcmatch	fc-match default
 * fclist	fc-list default
 * fccat	fc-cat default
 * pkgkit	PackageKit package tag format
 *
 *
 * Some ideas for future syntax extensions:
 *
 * - verbose builtin that is like FcPatternPrint
 * - allow indexing subexprs using '%{[idx]elt1,elt2{subexpr}}'
 * - allow indexing in +, -, ? filtering?
 * - conditional/filtering/deletion on binding (using '(w)'/'(s)'/'(=)' notation)
 */


#define FCCAT_FORMAT	"\"%{file|basename|cescape}\" %{index} \"%{-file{%{=unparse|cescape}}}\""
#define FCMATCH_FORMAT	"%{file:-<unknown filename>|basename}: \"%{family[0]:-<unknown family>}\" \"%{style[0]:-<unknown style>}\""
#define FCLIST_FORMAT	"%{?file{%{file}: }}%{-file{%{=unparse}}}"
#define PKGKIT_FORMAT	"%{[]family{font(%{family|downcase|delete( )})\n}}%{[]lang{font(:lang=%{lang|downcase|translate(_,-)})\n}}"


static void
message (const char *fmt, ...)
{
    va_list	args;
    va_start (args, fmt);
    fprintf (stderr, "Fontconfig: Pattern format error: ");
    vfprintf (stderr, fmt, args);
    fprintf (stderr, ".\n");
    va_end (args);
}


typedef struct _FcFormatContext
{
    const FcChar8 *format_orig;
    const FcChar8 *format;
    int            format_len;
    FcChar8       *word;
    FcBool         word_allocated;
} FcFormatContext;

static FcBool
FcFormatContextInit (FcFormatContext *c,
		     const FcChar8   *format,
		     FcChar8         *scratch,
		     int              scratch_len)
{
    c->format_orig = c->format = format;
    c->format_len = strlen ((const char *) format);

    if (c->format_len < scratch_len)
    {
	c->word = scratch;
	c->word_allocated = FcFalse;
    }
    else
    {
	c->word = malloc (c->format_len + 1);
	c->word_allocated = FcTrue;
    }

    return c->word != NULL;
}

static void
FcFormatContextDone (FcFormatContext *c)
{
    if (c && c->word_allocated)
    {
	free (c->word);
    }
}

static FcBool
consume_char (FcFormatContext *c,
	      FcChar8          term)
{
    if (*c->format != term)
	return FcFalse;

    c->format++;
    return FcTrue;
}

static FcBool
expect_char (FcFormatContext *c,
	      FcChar8          term)
{
    FcBool res = consume_char (c, term);
    if (!res)
    {
	if (c->format == c->format_orig + c->format_len)
	    message ("format ended while expecting '%c'",
		     term);
	else
	    message ("expected '%c' at %d",
		     term, c->format - c->format_orig + 1);
    }
    return res;
}

static FcBool
FcCharIsPunct (const FcChar8 c)
{
    if (c < '0')
	return FcTrue;
    if (c <= '9')
	return FcFalse;
    if (c < 'A')
	return FcTrue;
    if (c <= 'Z')
	return FcFalse;
    if (c < 'a')
	return FcTrue;
    if (c <= 'z')
	return FcFalse;
    if (c <= '~')
	return FcTrue;
    return FcFalse;
}

static char escaped_char(const char ch)
{
    switch (ch) {
    case 'a':   return '\a';
    case 'b':   return '\b';
    case 'f':   return '\f';
    case 'n':   return '\n';
    case 'r':   return '\r';
    case 't':   return '\t';
    case 'v':   return '\v';
    default:    return ch;
    }
}

static FcBool
read_word (FcFormatContext *c)
{
    FcChar8 *p;

    p = c->word;

    while (*c->format)
    {
	if (*c->format == '\\')
	{
	    c->format++;
	    if (*c->format)
	      *p++ = escaped_char (*c->format++);
	    continue;
	}
	else if (FcCharIsPunct (*c->format))
	    break;

	*p++ = *c->format++;
    }
    *p = '\0';

    if (p == c->word)
    {
	message ("expected identifier at %d",
		 c->format - c->format_orig + 1);
	return FcFalse;
    }

    return FcTrue;
}

static FcBool
read_chars (FcFormatContext *c,
	    FcChar8          term)
{
    FcChar8 *p;

    p = c->word;

    while (*c->format && *c->format != '}' && *c->format != term)
    {
	if (*c->format == '\\')
	{
	    c->format++;
	    if (*c->format)
	      *p++ = escaped_char (*c->format++);
	    continue;
	}

	*p++ = *c->format++;
    }
    *p = '\0';

    if (p == c->word)
    {
	message ("expected character data at %d",
		 c->format - c->format_orig + 1);
	return FcFalse;
    }

    return FcTrue;
}

static FcBool
FcPatternFormatToBuf (FcPattern     *pat,
		      const FcChar8 *format,
		      FcStrBuf      *buf);

static FcBool
interpret_builtin (FcFormatContext *c,
		   FcPattern       *pat,
		   FcStrBuf        *buf)
{
    FcChar8       *new_str;
    FcBool         ret;

    if (!expect_char (c, '=') ||
	!read_word (c))
	return FcFalse;

    /* try simple builtins first */
    if (0) { }
#define BUILTIN(name, func) \
    else if (0 == strcmp ((const char *) c->word, name))\
	do { new_str = func (pat); ret = FcTrue; } while (0)
    BUILTIN ("unparse",  FcNameUnparse);
 /* BUILTIN ("verbose",  FcPatternPrint); XXX */
#undef BUILTIN
    else
	ret = FcFalse;

    if (ret)
    {
	if (new_str)
	{
	    FcStrBufString (buf, new_str);
	    FcStrFree (new_str);
	    return FcTrue;
	}
	else
	    return FcFalse;
    }

    /* now try our custom formats */
    if (0) { }
#define BUILTIN(name, format) \
    else if (0 == strcmp ((const char *) c->word, name))\
	ret = FcPatternFormatToBuf (pat, (const FcChar8 *) format, buf)
    BUILTIN ("fccat",    FCCAT_FORMAT);
    BUILTIN ("fcmatch",  FCMATCH_FORMAT);
    BUILTIN ("fclist",   FCLIST_FORMAT);
    BUILTIN ("pkgkit",   PKGKIT_FORMAT);
#undef BUILTIN
    else
	ret = FcFalse;

    if (!ret)
	message ("unknown builtin \"%s\"",
		 c->word);

    return ret;
}

static FcBool
interpret_expr (FcFormatContext *c,
		FcPattern       *pat,
		FcStrBuf        *buf,
		FcChar8          term);

static FcBool
interpret_subexpr (FcFormatContext *c,
		   FcPattern       *pat,
		   FcStrBuf        *buf)
{
    return expect_char (c, '{') &&
	   interpret_expr (c, pat, buf, '}') &&
	   expect_char (c, '}');
}

static FcBool
maybe_interpret_subexpr (FcFormatContext *c,
			 FcPattern       *pat,
			 FcStrBuf        *buf)
{
    return (*c->format == '{') ?
	   interpret_subexpr (c, pat, buf) :
	   FcTrue;
}

static FcBool
skip_subexpr (FcFormatContext *c);

static FcBool
skip_percent (FcFormatContext *c)
{
    if (!expect_char (c, '%'))
	return FcFalse;

    /* skip an optional width specifier */
    if (strtol ((const char *) c->format, (char **) &c->format, 10))
        {/* don't care */}

    if (!expect_char (c, '{'))
	return FcFalse;

    while(*c->format && *c->format != '}')
    {
	switch (*c->format)
	{
	case '\\':
	    c->format++; /* skip over '\\' */
	    if (*c->format)
		c->format++;
	    continue;
	case '{':
	    if (!skip_subexpr (c))
		return FcFalse;
	    continue;
	}
	c->format++;
    }

    return expect_char (c, '}');
}

static FcBool
skip_expr (FcFormatContext *c)
{
    while(*c->format && *c->format != '}')
    {
	switch (*c->format)
	{
	case '\\':
	    c->format++; /* skip over '\\' */
	    if (*c->format)
		c->format++;
	    continue;
	case '%':
	    if (!skip_percent (c))
		return FcFalse;
	    continue;
	}
	c->format++;
    }

    return FcTrue;
}

static FcBool
skip_subexpr (FcFormatContext *c)
{
    return expect_char (c, '{') &&
	   skip_expr (c) &&
	   expect_char (c, '}');
}

static FcBool
maybe_skip_subexpr (FcFormatContext *c)
{
    return (*c->format == '{') ?
	   skip_subexpr (c) :
	   FcTrue;
}

static FcBool
interpret_filter_in (FcFormatContext *c,
		     FcPattern       *pat,
		     FcStrBuf        *buf)
{
    FcObjectSet  *os;
    FcPattern    *subpat;

    if (!expect_char (c, '+'))
	return FcFalse;

    os = FcObjectSetCreate ();
    if (!os)
	return FcFalse;

    do
    {
	/* XXX binding */
	if (!read_word (c) ||
	    !FcObjectSetAdd (os, (const char *) c->word))
	{
	    FcObjectSetDestroy (os);
	    return FcFalse;
	}
    }
    while (consume_char (c, ','));

    subpat = FcPatternFilter (pat, os);
    FcObjectSetDestroy (os);

    if (!subpat ||
	!interpret_subexpr (c, subpat, buf))
	return FcFalse;

    FcPatternDestroy (subpat);
    return FcTrue;
}

static FcBool
interpret_filter_out (FcFormatContext *c,
		      FcPattern       *pat,
		      FcStrBuf        *buf)
{
    FcPattern    *subpat;

    if (!expect_char (c, '-'))
	return FcFalse;

    subpat = FcPatternDuplicate (pat);
    if (!subpat)
	return FcFalse;

    do
    {
	if (!read_word (c))
	{
	    FcPatternDestroy (subpat);
	    return FcFalse;
	}

	FcPatternDel (subpat, (const char *) c->word);
    }
    while (consume_char (c, ','));

    if (!interpret_subexpr (c, subpat, buf))
	return FcFalse;

    FcPatternDestroy (subpat);
    return FcTrue;
}

static FcBool
interpret_cond (FcFormatContext *c,
		FcPattern       *pat,
		FcStrBuf        *buf)
{
    FcBool pass;

    if (!expect_char (c, '?'))
	return FcFalse;

    pass = FcTrue;

    do
    {
	FcBool negate;
	FcValue v;

	negate = consume_char (c, '!');

	if (!read_word (c))
	    return FcFalse;

	pass = pass &&
	       (negate ^
		(FcResultMatch ==
		 FcPatternGet (pat, (const char *) c->word, 0, &v)));
    }
    while (consume_char (c, ','));

    if (pass)
    {
	if (!interpret_subexpr  (c, pat, buf) ||
	    !maybe_skip_subexpr (c))
	    return FcFalse;
    }
    else
    {
	if (!skip_subexpr (c) ||
	    !maybe_interpret_subexpr  (c, pat, buf))
	    return FcFalse;
    }

    return FcTrue;
}

static FcBool
interpret_count (FcFormatContext *c,
		 FcPattern       *pat,
		 FcStrBuf        *buf)
{
    int count;
    FcPatternIter iter;
    FcChar8 buf_static[64];

    if (!expect_char (c, '#'))
	return FcFalse;

    if (!read_word (c))
	return FcFalse;

    count = 0;
    if (FcPatternFindIter (pat, &iter, (const char *) c->word))
    {
	count = FcPatternIterValueCount (pat, &iter);
    }

    snprintf ((char *) buf_static, sizeof (buf_static), "%d", count);
    FcStrBufString (buf, buf_static);

    return FcTrue;
}

static FcBool
interpret_enumerate (FcFormatContext *c,
		     FcPattern       *pat,
		     FcStrBuf        *buf)
{
    FcObjectSet   *os;
    FcPattern     *subpat;
    const FcChar8 *format_save;
    int            idx;
    FcBool         ret, done;
    FcStrList      *lang_strs;

    if (!expect_char (c, '[') ||
	!expect_char (c, ']'))
	return FcFalse;

    os = FcObjectSetCreate ();
    if (!os)
	return FcFalse;

    ret = FcTrue;

    do
    {
	if (!read_word (c) ||
	    !FcObjectSetAdd (os, (const char *) c->word))
	{
	    FcObjectSetDestroy (os);
	    return FcFalse;
	}
    }
    while (consume_char (c, ','));

    /* If we have one element and it's of type FcLangSet, we want
     * to enumerate the languages in it. */
    lang_strs = NULL;
    if (os->nobject == 1)
    {
	FcLangSet *langset;
	if (FcResultMatch ==
	    FcPatternGetLangSet (pat, os->objects[0], 0, &langset))
	{
	    FcStrSet *ss;
	    if (!(ss = FcLangSetGetLangs (langset)) ||
		!(lang_strs = FcStrListCreate (ss)))
		goto bail0;
	}
    }

    subpat = FcPatternDuplicate (pat);
    if (!subpat)
	goto bail0;

    format_save = c->format;
    idx = 0;
    do
    {
	int i;

	done = FcTrue;

	if (lang_strs)
	{
	    FcChar8 *lang;

	    FcPatternDel (subpat, os->objects[0]);
	    if ((lang = FcStrListNext (lang_strs)))
	    {
		/* XXX binding? */
		FcPatternAddString (subpat, os->objects[0], lang);
		done = FcFalse;
	    }
	}
	else
	{
	    for (i = 0; i < os->nobject; i++)
	    {
		FcValue v;

		/* XXX this can be optimized by accessing valuelist linked lists
		 * directly and remembering where we were.  Most (all) value lists
		 * in normal uses are pretty short though (language tags are
		 * stored as a LangSet, not separate values.). */
		FcPatternDel (subpat, os->objects[i]);
		if (FcResultMatch ==
		    FcPatternGet (pat, os->objects[i], idx, &v))
		{
		    /* XXX binding */
		    FcPatternAdd (subpat, os->objects[i], v, FcFalse);
		    done = FcFalse;
		}
	    }
	}

	if (!done)
	{
	    c->format = format_save;
	    ret = interpret_subexpr (c, subpat, buf);
	    if (!ret)
		goto bail;
	}

	idx++;
    } while (!done);

    if (c->format == format_save)
	skip_subexpr (c);

bail:
    FcPatternDestroy (subpat);
bail0:
    if (lang_strs)
	FcStrListDone (lang_strs);
    FcObjectSetDestroy (os);

    return ret;
}

static FcBool
interpret_simple (FcFormatContext *c,
		  FcPattern       *pat,
		  FcStrBuf        *buf)
{
    FcPatternIter iter;
    FcBool        add_colon = FcFalse;
    FcBool        add_elt_name = FcFalse;
    int           idx;
    FcChar8      *else_string;

    if (consume_char (c, ':'))
	add_colon = FcTrue;

    if (!read_word (c))
	return FcFalse;

    idx = -1;
    if (consume_char (c, '['))
    {
	idx = strtol ((const char *) c->format, (char **) &c->format, 10);
	if (idx < 0)
	{
	    message ("expected non-negative number at %d",
		     c->format-1 - c->format_orig + 1);
	    return FcFalse;
	}
	if (!expect_char (c, ']'))
	    return FcFalse;
    }

    if (consume_char (c, '='))
	add_elt_name = FcTrue;

    /* modifiers */
    else_string = NULL;
    if (consume_char (c, ':'))
    {
	FcChar8 *orig;
	/* divert the c->word for now */
	orig = c->word;
	c->word = c->word + strlen ((const char *) c->word) + 1;
	/* for now we just support 'default value' */
	if (!expect_char (c, '-') ||
	    !read_chars (c, '|'))
	{
	    c->word = orig;
	    return FcFalse;
	}
	else_string = c->word;
	c->word = orig;
    }

    if (FcPatternFindIter (pat, &iter, (const char *) c->word) || else_string)
    {
	FcValueListPtr l = NULL;

	if (add_colon)
	    FcStrBufChar (buf, ':');
	if (add_elt_name)
	{
	    FcStrBufString (buf, c->word);
	    FcStrBufChar (buf, '=');
	}

	l = FcPatternIterGetValues (pat, &iter);

	if (idx != -1)
	{
	    while (l && idx > 0)
	    {
		l = FcValueListNext(l);
		idx--;
	    }
	    if (l && idx == 0)
	    {
		if (!FcNameUnparseValue (buf, &l->value, NULL))
		    return FcFalse;
	    }
	    else goto notfound;
        }
	else if (l)
	{
	    FcNameUnparseValueList (buf, l, NULL);
	}
	else
	{
    notfound:
	    if (else_string)
		FcStrBufString (buf, else_string);
	}
    }

    return FcTrue;
}

static FcBool
cescape (FcFormatContext *c FC_UNUSED,
	 const FcChar8   *str,
	 FcStrBuf        *buf)
{
    /* XXX escape \n etc? */

    while(*str)
    {
	switch (*str)
	{
	case '\\':
	case '"':
	    FcStrBufChar (buf, '\\');
	    break;
	}
	FcStrBufChar (buf, *str++);
    }
    return FcTrue;
}

static FcBool
shescape (FcFormatContext *c FC_UNUSED,
	  const FcChar8   *str,
	  FcStrBuf        *buf)
{
    FcStrBufChar (buf, '\'');
    while(*str)
    {
	if (*str == '\'')
	    FcStrBufString (buf, (const FcChar8 *) "'\\''");
	else
	    FcStrBufChar (buf, *str);
	str++;
    }
    FcStrBufChar (buf, '\'');
    return FcTrue;
}

static FcBool
xmlescape (FcFormatContext *c FC_UNUSED,
	   const FcChar8   *str,
	   FcStrBuf        *buf)
{
    /* XXX escape \n etc? */

    while(*str)
    {
	switch (*str)
	{
	case '&': FcStrBufString (buf, (const FcChar8 *) "&amp;"); break;
	case '<': FcStrBufString (buf, (const FcChar8 *) "&lt;");  break;
	case '>': FcStrBufString (buf, (const FcChar8 *) "&gt;");  break;
	default:  FcStrBufChar   (buf, *str);                      break;
	}
	str++;
    }
    return FcTrue;
}

static FcBool
delete_chars (FcFormatContext *c,
	      const FcChar8   *str,
	      FcStrBuf        *buf)
{
    /* XXX not UTF-8 aware */

    if (!expect_char (c, '(') ||
	!read_chars (c, ')') ||
	!expect_char (c, ')'))
	return FcFalse;

    while(*str)
    {
	FcChar8 *p;

	p = (FcChar8 *) strpbrk ((const char *) str, (const char *) c->word);
	if (p)
	{
	    FcStrBufData (buf, str, p - str);
	    str = p + 1;
	}
	else
	{
	    FcStrBufString (buf, str);
	    break;
	}

    }

    return FcTrue;
}

static FcBool
escape_chars (FcFormatContext *c,
	      const FcChar8   *str,
	      FcStrBuf        *buf)
{
    /* XXX not UTF-8 aware */

    if (!expect_char (c, '(') ||
	!read_chars (c, ')') ||
	!expect_char (c, ')'))
	return FcFalse;

    while(*str)
    {
	FcChar8 *p;

	p = (FcChar8 *) strpbrk ((const char *) str, (const char *) c->word);
	if (p)
	{
	    FcStrBufData (buf, str, p - str);
	    FcStrBufChar (buf, c->word[0]);
	    FcStrBufChar (buf, *p);
	    str = p + 1;
	}
	else
	{
	    FcStrBufString (buf, str);
	    break;
	}

    }

    return FcTrue;
}

static FcBool
translate_chars (FcFormatContext *c,
		 const FcChar8   *str,
		 FcStrBuf        *buf)
{
    char *from, *to, repeat;
    int from_len, to_len;

    /* XXX not UTF-8 aware */

    if (!expect_char (c, '(') ||
	!read_chars (c, ',') ||
	!expect_char (c, ','))
	return FcFalse;

    from = (char *) c->word;
    from_len = strlen (from);
    to = from + from_len + 1;

    /* hack: we temporarily divert c->word */
    c->word = (FcChar8 *) to;
    if (!read_chars (c, ')'))
    {
      c->word = (FcChar8 *) from;
      return FcFalse;
    }
    c->word = (FcChar8 *) from;

    to_len = strlen (to);
    repeat = to[to_len - 1];

    if (!expect_char (c, ')'))
	return FcFalse;

    while(*str)
    {
	FcChar8 *p;

	p = (FcChar8 *) strpbrk ((const char *) str, (const char *) from);
	if (p)
	{
	    int i;
	    FcStrBufData (buf, str, p - str);
	    i = strchr (from, *p) - from;
	    FcStrBufChar (buf, i < to_len ? to[i] : repeat);
	    str = p + 1;
	}
	else
	{
	    FcStrBufString (buf, str);
	    break;
	}

    }

    return FcTrue;
}

static FcBool
interpret_convert (FcFormatContext *c,
		   FcStrBuf        *buf,
		   int              start)
{
    const FcChar8 *str;
    FcChar8       *new_str;
    FcStrBuf       new_buf;
    FcChar8        buf_static[8192];
    FcBool         ret;

    if (!expect_char (c, '|') ||
	!read_word (c))
	return FcFalse;

    /* prepare the buffer */
    FcStrBufChar (buf, '\0');
    if (buf->failed)
	return FcFalse;
    str = buf->buf + start;
    buf->len = start;

    /* try simple converters first */
    if (0) { }
#define CONVERTER(name, func) \
    else if (0 == strcmp ((const char *) c->word, name))\
	do { new_str = func (str); ret = FcTrue; } while (0)
    CONVERTER  ("downcase",  FcStrDowncase);
    CONVERTER  ("basename",  FcStrBasename);
    CONVERTER  ("dirname",   FcStrDirname);
#undef CONVERTER
    else
	ret = FcFalse;

    if (ret)
    {
	if (new_str)
	{
	    FcStrBufString (buf, new_str);
	    FcStrFree (new_str);
	    return FcTrue;
	}
	else
	    return FcFalse;
    }

    FcStrBufInit (&new_buf, buf_static, sizeof (buf_static));

    /* now try our custom converters */
    if (0) { }
#define CONVERTER(name, func) \
    else if (0 == strcmp ((const char *) c->word, name))\
	ret = func (c, str, &new_buf)
    CONVERTER ("cescape",   cescape);
    CONVERTER ("shescape",  shescape);
    CONVERTER ("xmlescape", xmlescape);
    CONVERTER ("delete",    delete_chars);
    CONVERTER ("escape",    escape_chars);
    CONVERTER ("translate", translate_chars);
#undef CONVERTER
    else
	ret = FcFalse;

    if (ret)
    {
	FcStrBufChar (&new_buf, '\0');
	FcStrBufString (buf, new_buf.buf);
    }
    else
	message ("unknown converter \"%s\"",
		 c->word);

    FcStrBufDestroy (&new_buf);

    return ret;
}

static FcBool
maybe_interpret_converts (FcFormatContext *c,
			   FcStrBuf        *buf,
			   int              start)
{
    while (*c->format == '|')
	if (!interpret_convert (c, buf, start))
	    return FcFalse;

    return FcTrue;
}

static FcBool
align_to_width (FcStrBuf *buf,
		int       start,
		int       width)
{
    int len;

    if (buf->failed)
	return FcFalse;

    len = buf->len - start;
    if (len < -width)
    {
	/* left align */
	while (len++ < -width)
	    FcStrBufChar (buf, ' ');
    }
    else if (len < width)
    {
	int old_len;
	old_len = len;
	/* right align */
	while (len++ < width)
	    FcStrBufChar (buf, ' ');
	if (buf->failed)
	    return FcFalse;
	len = old_len;
	memmove (buf->buf + buf->len - len,
		 buf->buf + buf->len - width,
		 len);
	memset (buf->buf + buf->len - width,
		' ',
		width - len);
    }

    return !buf->failed;
}
static FcBool
interpret_percent (FcFormatContext *c,
		   FcPattern       *pat,
		   FcStrBuf        *buf)
{
    int width, start;
    FcBool ret;

    if (!expect_char (c, '%'))
	return FcFalse;

    if (consume_char (c, '%')) /* "%%" */
    {
	FcStrBufChar (buf, '%');
	return FcTrue;
    }

    /* parse an optional width specifier */
    width = strtol ((const char *) c->format, (char **) &c->format, 10);

    if (!expect_char (c, '{'))
	return FcFalse;

    start = buf->len;

    switch (*c->format) {
    case '=': ret = interpret_builtin    (c, pat, buf); break;
    case '{': ret = interpret_subexpr    (c, pat, buf); break;
    case '+': ret = interpret_filter_in  (c, pat, buf); break;
    case '-': ret = interpret_filter_out (c, pat, buf); break;
    case '?': ret = interpret_cond       (c, pat, buf); break;
    case '#': ret = interpret_count      (c, pat, buf); break;
    case '[': ret = interpret_enumerate  (c, pat, buf); break;
    default:  ret = interpret_simple     (c, pat, buf); break;
    }

    return ret &&
	   maybe_interpret_converts (c, buf, start) &&
	   align_to_width (buf, start, width) &&
	   expect_char (c, '}');
}

static FcBool
interpret_expr (FcFormatContext *c,
		FcPattern       *pat,
		FcStrBuf        *buf,
		FcChar8          term)
{
    while (*c->format && *c->format != term)
    {
	switch (*c->format)
	{
	case '\\':
	    c->format++; /* skip over '\\' */
	    if (*c->format)
		FcStrBufChar (buf, escaped_char (*c->format++));
	    continue;
	case '%':
	    if (!interpret_percent (c, pat, buf))
		return FcFalse;
	    continue;
	}
	FcStrBufChar (buf, *c->format++);
    }
    return FcTrue;
}

static FcBool
FcPatternFormatToBuf (FcPattern     *pat,
		      const FcChar8 *format,
		      FcStrBuf      *buf)
{
    FcFormatContext c;
    FcChar8         word_static[1024];
    FcBool          ret;

    if (!FcFormatContextInit (&c, format, word_static, sizeof (word_static)))
	return FcFalse;

    ret = interpret_expr (&c, pat, buf, '\0');

    FcFormatContextDone (&c);

    return ret;
}

FcChar8 *
FcPatternFormat (FcPattern *pat,
		 const FcChar8 *format)
{
    FcStrBuf        buf;
    FcChar8         buf_static[8192 - 1024];
    FcPattern      *alloced = NULL;
    FcBool          ret;

    if (!pat)
	alloced = pat = FcPatternCreate ();

    FcStrBufInit (&buf, buf_static, sizeof (buf_static));

    ret = FcPatternFormatToBuf (pat, format, &buf);

    if (alloced)
      FcPatternDestroy (alloced);

    if (ret)
	return FcStrBufDone (&buf);
    else
    {
	FcStrBufDestroy (&buf);
	return NULL;
    }
}

#define __fcformat__
#include "fcaliastail.h"
#undef __fcformat__
