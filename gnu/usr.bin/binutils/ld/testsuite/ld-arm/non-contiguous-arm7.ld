/*
 The section .bss.MY_BUF won't fit in RAM1 or RAM2
*/

MEMORY
{
  ROM  (rx)    : ORIGIN = 0x8000000,    LENGTH = 10K
  RAM1 (xrw)   : ORIGIN = 0x10000000,   LENGTH = 64K
  RAM2 (xrw)   : ORIGIN = 0x20000000,   LENGTH = 96K
}

SECTIONS
{
  .text :
  {
    KEEP(*(.text.foo)) ;
  } >ROM

  .bss :
  {
    _sbss = .;
    *(.bss) *(.bss*) ;
    _ebss = .;
  } >RAM1

  .bss_ram2 :
  {
    _sbss_ram2 = .;
    *(.bss) *(.bss*) ;
    _ebss_ram2 = .;
  } >RAM2
}
