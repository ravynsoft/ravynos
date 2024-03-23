#include <cairo-boilerplate.h>
#include <stdio.h>

int
main (void)
{
  printf ("Check linking to the just built cairo boilerplate library\n");
  if (cairo_boilerplate_version () == CAIRO_VERSION) {
    return 0;
  } else {
    fprintf (stderr,
	     "Error: linked to cairo boilerplate version %s instead of %s\n",
	     cairo_boilerplate_version_string (),
	     CAIRO_VERSION_STRING);
    return 1;
  }
}
