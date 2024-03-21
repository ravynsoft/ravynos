/* Test for the various accessor functions.  */

#include <config.h>

#include "textstyle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT(x) if (!(x)) abort ()

int
main ()
{
  {
    ostream_t stream = file_ostream_create (stdout);

    ASSERT (file_ostream_get_stdio_stream (stream) == stdout);

    ostream_free (stream);
  }
  {
    ostream_t stream = fd_ostream_create (1, "(stdout)", false);

    ASSERT (fd_ostream_get_descriptor (stream) == 1);
    ASSERT (strcmp (fd_ostream_get_filename (stream), "(stdout)") == 0);
    ASSERT (! fd_ostream_is_buffered (stream));

    ostream_free (stream);
  }
  {
    ostream_t stream = term_ostream_create (1, "(stdout)", TTYCTL_AUTO);

    ASSERT (term_ostream_get_descriptor (stream) == 1);
    ASSERT (strcmp (term_ostream_get_filename (stream), "(stdout)") == 0);
    ASSERT (term_ostream_get_tty_control (stream) == TTYCTL_AUTO);
    ASSERT (term_ostream_get_effective_tty_control (stream) == TTYCTL_FULL);

    ostream_free (stream);
  }
#if LIBTEXTSTYLE_USES_ICONV
  {
    ostream_t stream1 = file_ostream_create (stdout);
    ostream_t stream = iconv_ostream_create ("ISO-8859-1", "UTF-8", stream1);
    ASSERT (strcmp (iconv_ostream_get_from_encoding (stream), "ISO-8859-1") == 0);
    ASSERT (strcmp (iconv_ostream_get_to_encoding (stream), "UTF-8") == 0);
    ASSERT (iconv_ostream_get_destination (stream) == stream1);

    ostream_free (stream);
    ostream_free (stream1);
  }
#endif
  {
    ostream_t stream1 = file_ostream_create (stdout);
    ostream_t stream = html_ostream_create (stream1);

    ASSERT (html_ostream_get_destination (stream) == stream1);

    ostream_free (stream);
    ostream_free (stream1);
  }
  {
    ostream_t stream =
      term_styled_ostream_create (1, "(stdout)", TTYCTL_AUTO,
                                  SRCDIR "../adhoc-tests/hello-default.css");

    ASSERT (is_instance_of_term_ostream (term_styled_ostream_get_destination (stream)));
    ASSERT (strcmp (term_styled_ostream_get_css_filename (stream),
                    SRCDIR "../adhoc-tests/hello-default.css") == 0);

    ostream_free (stream);
  }
  {
    ostream_t stream1 = file_ostream_create (stdout);
    ostream_t stream =
      html_styled_ostream_create (stream1,
                                  SRCDIR "../adhoc-tests/hello-default.css");

    ASSERT (html_styled_ostream_get_destination (stream) == stream1);
    ASSERT (is_instance_of_html_ostream (html_styled_ostream_get_html_destination (stream)));
    ASSERT (strcmp (html_styled_ostream_get_css_filename (stream),
                    SRCDIR "../adhoc-tests/hello-default.css") == 0);

    ostream_free (stream);
    ostream_free (stream1);
  }
  {
    ostream_t stream1 = file_ostream_create (stdout);
    ostream_t stream = noop_styled_ostream_create (stream1, false);

    ASSERT (noop_styled_ostream_get_destination (stream) == stream1);
    ASSERT (!noop_styled_ostream_is_owning_destination (stream));

    ostream_free (stream);
    ostream_free (stream1);
  }

  return 0;
}
