/*
 sections .code.1 and .code.2 fit in .raml
 section .code.3 fits in .ramu and does not need a farcall stub to jump to code4
 section .code.4 fits in .ramz
*/
MEMORY
{
  RAML (rwx) : ORIGIN = 0x1FFF0000, LENGTH = 0x0001c
  RAMU (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008
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
