/*
 sections .code.1, .code.2 and .code.3 (+ farcall stub) fit in .raml
 section .code.4 fits in .ramz
 nothing fits in .ramu
*/
MEMORY
{
  RAML (rwx) : ORIGIN = 0x1FFF0000, LENGTH = 0x00030
  RAMU (rwx) : ORIGIN = 0x30000000, LENGTH = 0x00010
  RAMZ (rwx) : ORIGIN = 0x40040000, LENGTH = 0x00400
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
