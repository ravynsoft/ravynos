SECTIONS
{
  . = 0x1000;
  .sec1 : { *sec-to-seg1.o(.rodata) }
  . += CONSTANT(MAXPAGESIZE);
  .sec2  : { *sec-to-seg2.o(.rodata) }

  .data : { *(.data) } /* For hppa64.  */
  
  /DISCARD/	: {*(*) }
}
