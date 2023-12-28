OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
  . = 0x07000;
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .rel.dyn : { *(.rel.dyn) }
  .rel.plt : { *(.rel.plt) }
  . = 0x08000;
  .plt : { *(.plt) }
  . = 0x08ff0;
  .text : { *(.text) }
  . = 0x10000;
  .dynamic : { *(.dynamic) }
}
