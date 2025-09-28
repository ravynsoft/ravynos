/*
 * fontconfig/src/fcweight.c
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

static const struct {
  int ot;
  int fc;
} map[] = {
    {   0, FC_WEIGHT_THIN },
    { 100, FC_WEIGHT_THIN },
    { 200, FC_WEIGHT_EXTRALIGHT },
    { 300, FC_WEIGHT_LIGHT },
    { 350, FC_WEIGHT_DEMILIGHT },
    { 380, FC_WEIGHT_BOOK },
    { 400, FC_WEIGHT_REGULAR },
    { 500, FC_WEIGHT_MEDIUM },
    { 600, FC_WEIGHT_DEMIBOLD },
    { 700, FC_WEIGHT_BOLD },
    { 800, FC_WEIGHT_EXTRABOLD },
    { 900, FC_WEIGHT_BLACK },
    {1000, FC_WEIGHT_EXTRABLACK },
};

static double lerp(double x, int x1, int x2, int y1, int y2)
{
  int dx = x2 - x1;
  int dy = y2 - y1;
  assert (dx > 0 && dy >= 0 && x1 <= x && x <= x2);
  return y1 + (x-x1) * dy / dx;
}

double
FcWeightFromOpenTypeDouble (double ot_weight)
{
	int i;

	if (ot_weight < 0)
	    return -1;

	ot_weight = FC_MIN (ot_weight, map[(sizeof (map) / sizeof (map[0])) - 1].ot);

	for (i = 1; ot_weight > map[i].ot; i++)
	  ;

	if (ot_weight == map[i].ot)
	  return map[i].fc;

	/* Interpolate between two items. */
	return lerp (ot_weight, map[i-1].ot, map[i].ot, map[i-1].fc, map[i].fc);
}

double
FcWeightToOpenTypeDouble (double fc_weight)
{
	int i;
	if (fc_weight < 0 || fc_weight > FC_WEIGHT_EXTRABLACK)
	    return -1;

	for (i = 1; fc_weight > map[i].fc; i++)
	  ;

	if (fc_weight == map[i].fc)
	  return map[i].ot;

	/* Interpolate between two items. */
	return lerp (fc_weight, map[i-1].fc, map[i].fc, map[i-1].ot, map[i].ot);
}

int
FcWeightFromOpenType (int ot_weight)
{
	return FcWeightFromOpenTypeDouble (ot_weight) + .5;
}

int
FcWeightToOpenType (int fc_weight)
{
	return FcWeightToOpenTypeDouble (fc_weight) + .5;
}

#define __fcweight__
#include "fcaliastail.h"
#undef __fcweight__
