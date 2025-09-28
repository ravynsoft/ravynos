/*
 sections .code.1 and .code.2 fit in .raml
 section .code.3 fits in .ramu
 section .code.4 too large to fit
 expect an error about .code.4
*/
MEMORY
{
  RAML (rwx) : ORIGIN = 0x1FFF0000, LENGTH = 0x0001c
  RAMU (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00040
  RAMZ (rwx) : ORIGIN = 0x20040000, LENGTH = 0x00040
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
