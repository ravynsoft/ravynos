SECTIONS
{
  . = 0;
  .reginfo : { *(.reginfo) }
  .dynamic : { *(.dynamic) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .rel.dyn : { *(.rel.dyn) }
  . = 0x800;
  .text : { *(.text) }
  . = 0xc00;
  .eh_frame : { *(.eh_frame) }
  . = 0x10000;
  .data : { *(.data) }
  . = 0x10400;
  HIDDEN (_gp = . + 0x7ff0);
  .got : { *(.got) }
}
