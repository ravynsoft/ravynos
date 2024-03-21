/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

/* Field layout of superclass.  */
#include "ostream.priv.h"

/* Field layout of fd_ostream class.  */
struct fd_ostream_representation
{
  struct ostream_representation base;
  int fd;
  char *filename;
  char *buffer;                  
  size_t avail;                  
};
