/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

/* Field layout of superclass.  */
#include "styled_ostream.priv.h"

/* Field layout of html_styled_ostream class.  */
struct html_styled_ostream_representation
{
  struct styled_ostream_representation base;
   
  ostream_t destination;
   
  char *css_filename;
   
  html_ostream_t html_destination;
   
  char *hyperlink_id;
};
