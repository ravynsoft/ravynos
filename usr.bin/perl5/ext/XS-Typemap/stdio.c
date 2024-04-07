
/* This provides a test of STDIO and emulates a library that
   has been built outside of the PerlIO system and therefore is
   built using FILE* rather than PerlIO * (a common occurrence
   for XS).

   Use a separate file to make sure we are not contaminated by
   PerlIO.
*/

#include <stdio.h>

/* Open a file for write */
FILE * xsfopen ( const char * path ) {
  FILE * stream;
  stream = fopen( path, "w");
  return stream;
}

int xsfclose ( FILE * stream ) {
  return fclose( stream );
}


int xsfprintf ( FILE * stream, const char * text ) {
  return fprintf( stream, "%s", text );
}

