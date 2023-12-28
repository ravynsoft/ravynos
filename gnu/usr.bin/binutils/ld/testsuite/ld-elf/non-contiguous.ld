/*
 section .data.1 fits in .raml
 sections .data.2 .data.3 fit in .ramu
 section .data.4 fits in .ramz
*/
MEMORY
{
  RAML (rwx) : ORIGIN = 0x1FFF0000, LENGTH = 0x00014
  RAMU (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00040
  RAMZ (rwx) : ORIGIN = 0x20040000, LENGTH = 0x00040
}

SECTIONS
{
   /* Ignore this target specific info in output comparison.  */
   /DISCARD/ : {
             *(.ARM.attributes)
	     *(.ARC.attributes)
	     *(.riscv.attributes)
	     *(.c6xabi.attributes)
	     *(.trampolines)
	     *(.reginfo)
	     *(.note.renesas)
	     *(.MIPS.abiflags)
	     *(.MSP430.attributes)
	     *(.csky.attributes)
	     *(.gnu.attributes)
   }

   .raml : /*AT ( ADDR (.text) + SIZEOF (.text) )*/
   {  _raml_start = . ;
      *(.boot) ;
      *(.data) *(.data.*) ;
      _raml_end = . ;
   } > RAML

   .ramu : AT ( ADDR (.raml) + SIZEOF (.raml) )
   {  _ramu_start = . ;
      *(.data) *(.data.*) ;
      _ramu_end = . ;
   } > RAMU

   .ramz : AT ( ADDR (.ramu) + SIZEOF (.ramu) )
   {  _ramz_start = . ;
      *(.data) *(.data.*) ;
     _ramz_end = . ;
   } > RAMZ
}
