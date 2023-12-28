OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
  . = 0x07000;
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  . = 0x08000;
  .rel.dyn : {
    *(.rel.dyn)
    *(.rel.data)
    *(.rel.got)
    PROVIDE_HIDDEN (__irel_start = .);
    *(.rel.iplt)
    PROVIDE_HIDDEN (__irel_end = .);
  }
  .rel.plt : { *(.rel.plt) }
  . = 0x09000;
  .plt : { *(.plt) }
  .iplt : { *(.iplt) }
  . = 0x0A000;
  .text : { *(.text) }
  . = 0x10000;
  .data : { *(.data) }
  . = 0x11000;
  .got : { *(.got.plt) *(.igot.plt) *(.got) *(.igot) }
  . = 0x12000;
  .dynamic : { *(.dynamic) }
  . = 0x13000;
  .bss : { *(.bss) }
}
