/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

/* Field layout of superclass.  */
#include "styled_ostream.priv.h"

/* Field layout of noop_styled_ostream class.  */
struct noop_styled_ostream_representation
{
  struct styled_ostream_representation base;
   
  ostream_t destination;
  bool own_destination;
   
  char *hyperlink_ref;
  char *hyperlink_id;
};
