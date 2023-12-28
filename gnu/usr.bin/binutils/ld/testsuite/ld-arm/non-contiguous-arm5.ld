/*
 section .code.1 fits in .raml
 section .code.2 does not fit in .raml and goes to .ramu
 section .code.3 would fit in .raml, but goes to .ramu:  Check that .code.2 and .code.3 are not swapped
 section .code.4 fits in .ramz
*/
MEMORY
{
  RAML (rwx) : ORIGIN = 0x1FFF0000, LENGTH = 0x00014
  RAMU (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00020
  RAMZ (rwx) : ORIGIN = 0x20040000, LENGTH = 0x00400
}

SECTIONS
{
   .raml :
   {  _raml_start = . ;
      *(.boot) ;
      *(.code) *(.code.*) ;
      _raml_end = . ;
   } > RAML

   .ramu : AT ( ADDR (.raml) + SIZEOF (.raml) )
   {  _ramu_start = . ;
      *(.code) *(.code.*) ;
      _ramu_end = . ;
   } > RAMU

   .ramz : AT ( ADDR (.ramu) + SIZEOF (.ramu) )
   {  _ramz_start = . ;
      *(.code) *(.code.*) ;
     _ramz_end = . ;
   } > RAMZ
}
