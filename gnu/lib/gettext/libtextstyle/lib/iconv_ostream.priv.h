/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

/* Field layout of superclass.  */
#include "ostream.priv.h"

/* Field layout of iconv_ostream class.  */
struct iconv_ostream_representation
{
  struct ostream_representation base;
#if HAVE_ICONV
   
  ostream_t destination;
   
  char *from_encoding;
  char *to_encoding;
   
  iconv_t cd;
   
  #define BUFSIZE 64
  char buf[BUFSIZE];
  size_t buflen;
#endif  
};
