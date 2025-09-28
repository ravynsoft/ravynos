/*
 * fontconfig/test/test-issue180.c
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
#include <stdio.h>
#include <stdlib.h>
#include <fontconfig/fontconfig.h>

int
main (void)
{
    const FcChar8 *doc = (const FcChar8 *) ""
	"<fontconfig>\n"
	"  <cachedir></cachedir>\n"
	"</fontconfig>\n"
	"";
    const FcChar8 *doc2 = (const FcChar8 *) ""
	"<fontconfig>\n"
	"  <cachedir prefix=\"xdg\"></cachedir>\n"
	"</fontconfig>\n"
	"";
    FcConfig *cfg = FcConfigCreate ();
    FcStrList *l;
    FcChar8 *p;

    if (!FcConfigParseAndLoadFromMemory (cfg, doc, FcTrue))
    {
	fprintf (stderr, "Unable to load a config from memory.\n");
	return 1;
    }
    l = FcConfigGetCacheDirs (cfg);
    if ((p = FcStrListNext (l)) != NULL)
    {
	fprintf (stderr, "There was one or more cachedirs\n");
	return 1;
    }
    FcStrListDone (l);
    FcConfigDestroy (cfg);

    cfg = FcConfigCreate ();
    if (!FcConfigParseAndLoadFromMemory (cfg, doc2, FcTrue))
    {
	fprintf (stderr, "Unable to load a config from memory (with prefix).\n");
	return 1;
    }
    l = FcConfigGetCacheDirs (cfg);
    if ((p = FcStrListNext (l)) != NULL)
    {
	fprintf (stderr, "There was one or more cachedirs (with prefix)\n");
	return 1;
    }
    FcStrListDone (l);
    FcConfigDestroy (cfg);

    return 0;
}
