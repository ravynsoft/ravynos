/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

/* Field layout of superclass.  */
#include "ostream.priv.h"

/* Field layout of memory_ostream class.  */
struct memory_ostream_representation
{
  struct ostream_representation base;
  char *buffer;                  
  size_t buflen;                 
  size_t allocated;              
};
