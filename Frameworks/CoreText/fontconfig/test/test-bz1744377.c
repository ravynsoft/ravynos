/*
 * fontconfig/test/test-bz1744377.c
 *
 * Copyright © 2000 Keith Packard
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
#include <fontconfig/fontconfig.h>

int
main (void)
{
    const FcChar8 *doc = (const FcChar8 *) ""
	"<fontconfig>\n"
	"  <include ignore_missing=\"yes\">blahblahblah</include>\n"
	"</fontconfig>\n"
	"";
    const FcChar8 *doc2 = (const FcChar8 *) ""
	"<fontconfig>\n"
	"  <include ignore_missing=\"no\">blahblahblah</include>\n"
	"</fontconfig>\n"
	"";
    FcConfig *cfg = FcConfigCreate ();

    if (!FcConfigParseAndLoadFromMemory (cfg, doc, FcTrue))
	return 1;
    if (FcConfigParseAndLoadFromMemory (cfg, doc2, FcTrue))
	return 1;
    if (!FcConfigParseAndLoadFromMemory (cfg, doc2, FcFalse))
	return 1;

    FcConfigDestroy (cfg);

    return 0;
}
