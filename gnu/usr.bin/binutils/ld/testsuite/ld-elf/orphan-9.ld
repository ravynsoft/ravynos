/* This linker script is used for orphan-9 and orphan-10 test.

   orphan-9: We have a single byte in .data, and an orphan .data.1
   section.  We are checking that the .data.1 orphan is assigned an
   LMA after .data rather than picking up the lma region of .rodata.

   orphan-10: In this case we have nothing in .data and an orphan
   .data.1, we are checking that .data.1 is assigned an LMA after
   .data, rather than defaulting to take LMA == VMA.  */

MEMORY
{
   MEM    : ORIGIN = 0x1000, LENGTH = 0x100
   TEXT   : ORIGIN = 0x200,  LENGTH = 0x50
   DATA   : ORIGIN = 0x300,  LENGTH = 0x50
   RODATA : ORIGIN = 0x400,  LENGTH = 0x50
}

SECTIONS
{
  .text : {
    *(.text)
  } >MEM AT>TEXT

  .data : AT(0x300) {
    *(.data)
  } >MEM

  .rodata : {
    *(.rodata)
  } >MEM AT>RODATA
}
