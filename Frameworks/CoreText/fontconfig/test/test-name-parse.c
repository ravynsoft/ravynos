#include <fontconfig/fontconfig.h>
#include <stdio.h>

static int
test (const FcChar8 *query, const FcPattern *expect)
{
    FcPattern *pat;
    int c = 0;

    c++;
    pat = FcNameParse (query);
    if (!pat)
	goto bail;
    c++;
    if (!FcPatternEqual (pat, expect))
	goto bail;
    c = 0;
bail:
    FcPatternDestroy (pat);

    return c;
}

#define BEGIN(x)	(x) = FcPatternCreate (); c++;
#define END(x)		FcPatternDestroy (x); (x) = NULL
int
main (void)
{
    FcPattern *expect;
    int c = 0, ret;

    BEGIN (expect) {
	FcPatternAddString (expect, FC_FAMILY, (const FcChar8 *)"sans-serif");
	if ((ret = test ((const FcChar8 *)"sans\\-serif", expect)) != 0)
	    goto bail;
    } END (expect);
    BEGIN (expect) {
	FcPatternAddString (expect, FC_FAMILY, (const FcChar8 *)"Foo");
	FcPatternAddInteger (expect, FC_SIZE, 10);
	if ((ret = test ((const FcChar8 *)"Foo-10", expect)) != 0)
	    goto bail;
    } END (expect);
    BEGIN (expect) {
	FcPatternAddString (expect, FC_FAMILY, (const FcChar8 *)"Foo");
	FcPatternAddString (expect, FC_FAMILY, (const FcChar8 *)"Bar");
	FcPatternAddInteger (expect, FC_SIZE, 10);
	if ((ret = test ((const FcChar8 *)"Foo,Bar-10", expect)) != 0)
	    goto bail;
    } END (expect);
    BEGIN (expect) {
	FcPatternAddString (expect, FC_FAMILY, (const FcChar8 *)"Foo");
	FcPatternAddInteger (expect, FC_WEIGHT, FC_WEIGHT_MEDIUM);
	if ((ret = test ((const FcChar8 *)"Foo:weight=medium", expect)) != 0)
	    goto bail;
    } END (expect);
    BEGIN (expect) {
	FcPatternAddString (expect, FC_FAMILY, (const FcChar8 *)"Foo");
	FcPatternAddInteger (expect, FC_WEIGHT, FC_WEIGHT_MEDIUM);
	if ((ret = test ((const FcChar8 *)"Foo:weight_medium", expect)) != 0)
	    goto bail;
    } END (expect);
    BEGIN (expect) {
	FcPatternAddInteger (expect, FC_WEIGHT, FC_WEIGHT_MEDIUM);
	if ((ret = test ((const FcChar8 *)":medium", expect)) != 0)
	    goto bail;
    } END (expect);
    BEGIN (expect) {
	FcPatternAddInteger (expect, FC_WEIGHT, FC_WEIGHT_NORMAL);
	if ((ret = test ((const FcChar8 *)":weight=normal", expect)) != 0)
	    goto bail;
    } END (expect);
    BEGIN (expect) {
	FcPatternAddInteger (expect, FC_WIDTH, FC_WIDTH_NORMAL);
	if ((ret = test ((const FcChar8 *)":width=normal", expect)) != 0)
	    goto bail;
    } END (expect);
    BEGIN (expect) {
	FcRange *r = FcRangeCreateDouble (FC_WEIGHT_MEDIUM, FC_WEIGHT_BOLD);
	FcPatternAddRange (expect, FC_WEIGHT, r);
	FcRangeDestroy (r);
	if ((ret = test ((const FcChar8 *)":weight=[medium bold]", expect)) != 0)
	    goto bail;
    } END (expect);

bail:
    if (expect)
	FcPatternDestroy (expect);

    return ret == 0 ? 0 : (c - 1) * 2 + ret;
}
