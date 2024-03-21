/* Test for the is_instance_of_* functions.  */

#include <config.h>

#include "textstyle.h"

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(x) if (!(x)) abort ()

int
main ()
{
  ostream_t stream1 = file_ostream_create (stdout);
  ostream_t stream2 = fd_ostream_create (1, "(stdout)", false);
  ostream_t stream3 = term_ostream_create (1, "(stdout)", TTYCTL_AUTO);
  ostream_t stream4 = memory_ostream_create ();
  ostream_t stream5 = html_ostream_create (stream1);
  ostream_t sstream1 =
    term_styled_ostream_create (1, "(stdout)", TTYCTL_AUTO,
                                SRCDIR "../adhoc-tests/hello-default.css");
  ostream_t sstream2 =
    html_styled_ostream_create (stream5,
                                SRCDIR "../adhoc-tests/hello-default.css");
  ostream_t sstream3 = noop_styled_ostream_create (stream1, false);

  if (stream1 != NULL)
    {
      ASSERT (! is_instance_of_styled_ostream (stream1));
      ASSERT (is_instance_of_file_ostream (stream1));
      ASSERT (! is_instance_of_fd_ostream (stream1));
      ASSERT (! is_instance_of_term_ostream (stream1));
      ASSERT (! is_instance_of_memory_ostream (stream1));
      ASSERT (! is_instance_of_iconv_ostream (stream1));
      ASSERT (! is_instance_of_html_ostream (stream1));
      ASSERT (! is_instance_of_term_styled_ostream (stream1));
      ASSERT (! is_instance_of_html_styled_ostream (stream1));
      ASSERT (! is_instance_of_noop_styled_ostream (stream1));
    }

  if (stream2 != NULL)
    {
      ASSERT (! is_instance_of_styled_ostream (stream2));
      ASSERT (! is_instance_of_file_ostream (stream2));
      ASSERT (is_instance_of_fd_ostream (stream2));
      ASSERT (! is_instance_of_term_ostream (stream2));
      ASSERT (! is_instance_of_memory_ostream (stream2));
      ASSERT (! is_instance_of_iconv_ostream (stream2));
      ASSERT (! is_instance_of_html_ostream (stream2));
      ASSERT (! is_instance_of_term_styled_ostream (stream2));
      ASSERT (! is_instance_of_html_styled_ostream (stream2));
      ASSERT (! is_instance_of_noop_styled_ostream (stream2));
    }

  if (stream3 != NULL)
    {
      ASSERT (! is_instance_of_styled_ostream (stream3));
      ASSERT (! is_instance_of_file_ostream (stream3));
      ASSERT (! is_instance_of_fd_ostream (stream3));
      ASSERT (is_instance_of_term_ostream (stream3));
      ASSERT (! is_instance_of_memory_ostream (stream3));
      ASSERT (! is_instance_of_iconv_ostream (stream3));
      ASSERT (! is_instance_of_html_ostream (stream3));
      ASSERT (! is_instance_of_term_styled_ostream (stream3));
      ASSERT (! is_instance_of_html_styled_ostream (stream3));
      ASSERT (! is_instance_of_noop_styled_ostream (stream3));
    }

  if (stream4 != NULL)
    {
      ASSERT (! is_instance_of_styled_ostream (stream4));
      ASSERT (! is_instance_of_file_ostream (stream4));
      ASSERT (! is_instance_of_fd_ostream (stream4));
      ASSERT (! is_instance_of_term_ostream (stream4));
      ASSERT (is_instance_of_memory_ostream (stream4));
      ASSERT (! is_instance_of_iconv_ostream (stream4));
      ASSERT (! is_instance_of_html_ostream (stream4));
      ASSERT (! is_instance_of_term_styled_ostream (stream4));
      ASSERT (! is_instance_of_html_styled_ostream (stream4));
      ASSERT (! is_instance_of_noop_styled_ostream (stream4));
    }

  if (stream5 != NULL)
    {
      ASSERT (! is_instance_of_styled_ostream (stream5));
      ASSERT (! is_instance_of_file_ostream (stream5));
      ASSERT (! is_instance_of_fd_ostream (stream5));
      ASSERT (! is_instance_of_term_ostream (stream5));
      ASSERT (! is_instance_of_memory_ostream (stream5));
      ASSERT (! is_instance_of_iconv_ostream (stream5));
      ASSERT (is_instance_of_html_ostream (stream5));
      ASSERT (! is_instance_of_term_styled_ostream (stream5));
      ASSERT (! is_instance_of_html_styled_ostream (stream5));
      ASSERT (! is_instance_of_noop_styled_ostream (stream5));
    }

  if (sstream1 != NULL)
    {
      ASSERT (is_instance_of_styled_ostream (sstream1));
      ASSERT (! is_instance_of_file_ostream (sstream1));
      ASSERT (! is_instance_of_fd_ostream (sstream1));
      ASSERT (! is_instance_of_term_ostream (sstream1));
      ASSERT (! is_instance_of_memory_ostream (sstream1));
      ASSERT (! is_instance_of_iconv_ostream (sstream1));
      ASSERT (! is_instance_of_html_ostream (sstream1));
      ASSERT (is_instance_of_term_styled_ostream (sstream1));
      ASSERT (! is_instance_of_html_styled_ostream (sstream1));
      ASSERT (! is_instance_of_noop_styled_ostream (sstream1));
    }

  if (sstream2 != NULL)
    {
      ASSERT (is_instance_of_styled_ostream (sstream2));
      ASSERT (! is_instance_of_file_ostream (sstream2));
      ASSERT (! is_instance_of_fd_ostream (sstream2));
      ASSERT (! is_instance_of_term_ostream (sstream2));
      ASSERT (! is_instance_of_memory_ostream (sstream2));
      ASSERT (! is_instance_of_iconv_ostream (sstream2));
      ASSERT (! is_instance_of_html_ostream (sstream2));
      ASSERT (! is_instance_of_term_styled_ostream (sstream2));
      ASSERT (is_instance_of_html_styled_ostream (sstream2));
      ASSERT (! is_instance_of_noop_styled_ostream (sstream2));
    }

  if (sstream3 != NULL)
    {
      ASSERT (is_instance_of_styled_ostream (sstream3));
      ASSERT (! is_instance_of_file_ostream (sstream3));
      ASSERT (! is_instance_of_fd_ostream (sstream3));
      ASSERT (! is_instance_of_term_ostream (sstream3));
      ASSERT (! is_instance_of_memory_ostream (sstream3));
      ASSERT (! is_instance_of_iconv_ostream (sstream3));
      ASSERT (! is_instance_of_html_ostream (sstream3));
      ASSERT (! is_instance_of_term_styled_ostream (sstream3));
      ASSERT (! is_instance_of_html_styled_ostream (sstream3));
      ASSERT (is_instance_of_noop_styled_ostream (sstream3));
    }

  return 0;
}
