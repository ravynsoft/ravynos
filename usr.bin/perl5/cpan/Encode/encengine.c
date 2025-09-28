/*
Data structures for encoding transformations.

Perl works internally in either a native 'byte' encoding or
in UTF-8 encoded Unicode.  We have no immediate need for a "wchar_t"
representation. When we do we can use utf8_to_uv().

Most character encodings are either simple byte mappings or
variable length multi-byte encodings. UTF-8 can be viewed as a
rather extreme case of the latter.

So to solve an important part of perl's encode needs we need to solve the
"multi-byte -> multi-byte" case. The simple byte forms are then just degenerate
case. (Where one of multi-bytes will usually be UTF-8.)

The other type of encoding is a shift encoding where a prefix sequence
determines what subsequent bytes mean. Such encodings have state.

We also need to handle case where a character in one encoding has to be
represented as multiple characters in the other. e.g. letter+diacritic.

The process can be considered as pseudo perl:

my $dst = '';
while (length($src))
 {
  my $size    = src_count($src);
  my $in_seq  = substr($src,0,$size,'');
  my $out_seq = $s2d_hash{$in_seq};
  if (defined $out_seq)
   {
    $dst .= $out_seq;
   }
  else
   {
    # an error condition
   }
 }
return $dst;

That has the following components:
 &src_count - a "rule" for how many bytes make up the next character in the
              source.
 %s2d_hash  - a mapping from input sequences to output sequences

The problem with that scheme is that it does not allow the output
character repertoire to affect the characters considered from the
input.

So we use a "trie" representation which can also be considered
a state machine:

my $dst   = '';
my $seq   = \@s2d_seq;
my $next  = \@s2d_next;
while (length($src))
 {
  my $byte    = $substr($src,0,1,'');
  my $out_seq = $seq->[$byte];
  if (defined $out_seq)
   {
    $dst .= $out_seq;
   }
  else
   {
    # an error condition
   }
  ($next,$seq) = @$next->[$byte] if $next;
 }
return $dst;

There is now a pair of data structures to represent everything.
It is valid for output sequence at a particular point to
be defined but zero length, that just means "don't know yet".
For the single byte case there is no 'next' so new tables will be the same as
the original tables. For a multi-byte case a prefix byte will flip to the tables
for  the next page (adding nothing to the output), then the tables for the page
will provide the actual output and set tables back to original base page.

This scheme can also handle shift encodings.

A slight enhancement to the scheme also allows for look-ahead - if
we add a flag to re-add the removed byte to the source we could handle
  a" -> U+00E4 (LATIN SMALL LETTER A WITH DIAERESIS)
  ab -> a (and take b back please)

*/

#define PERL_NO_GET_CONTEXT
#include <EXTERN.h>
#include <perl.h>
#include "encode.h"

int
do_encode(const encpage_t * enc, const U8 * src, STRLEN * slen, U8 * dst,
      STRLEN dlen, STRLEN * dout, int approx, const U8 *term, STRLEN tlen)
{
    const U8 *s = src;
    const U8 *send = s + *slen;
    const U8 *last = s;
    U8 *d = dst;
    U8 *dend = d + dlen, *dlast = d;
    int code = 0;
    if (!dst)
      return ENCODE_NOSPACE;
    while (s < send) {
        const encpage_t *e = enc;
        U8 byte = *s;
        while (byte > e->max)
            e++;
        if (byte >= e->min && e->slen && (approx || !(e->slen & 0x80))) {
            const U8 *cend = s + (e->slen & 0x7f);
            if (cend <= send) {
                STRLEN n;
                if ((n = e->dlen)) {
                    const U8 *out = e->seq + n * (byte - e->min);
                    U8 *oend = d + n;
                    if (dst) {
                        if (oend <= dend) {
                            while (d < oend)
                                *d++ = *out++;
                        }
                        else {
                            /* Out of space */
                            code = ENCODE_NOSPACE;
                            break;
                        }
                    }
                    else
                        d = oend;
                }
                enc = e->next;
                s++;
                if (s == cend) {
                    if (approx && (e->slen & 0x80))
                        code = ENCODE_FALLBACK;
                    last = s;
                    if (term && (STRLEN)(d-dlast) == tlen && memEQ(dlast, term, tlen)) {
                        code = ENCODE_FOUND_TERM;
                        break;
                    }
                    dlast = d;
                }
            }
            else {
                /* partial source character */
                code = ENCODE_PARTIAL;
                break;
            }
        }
        else {
            /* Cannot represent */
            code = ENCODE_NOREP;
            break;
        }
    }
    *slen = last - src;
    *dout = d - dst;
    return code;
}
