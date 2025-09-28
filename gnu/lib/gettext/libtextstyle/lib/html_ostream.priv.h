/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

/* Field layout of superclass.  */
#include "ostream.priv.h"

/* Field layout of html_ostream class.  */
struct html_ostream_representation
{
  struct ostream_representation base;
   
  ostream_t destination;
   
  char *hyperlink_ref;
   
  gl_list_t   class_stack;
   
  size_t curr_class_stack_size;
  size_t last_class_stack_size;
   
  #define BUFSIZE 6
  char buf[BUFSIZE];
  size_t buflen;
};
