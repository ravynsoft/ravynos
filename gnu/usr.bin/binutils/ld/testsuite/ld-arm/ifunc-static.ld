OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
  . = 0x08000;
  .rel.dyn : {
    PROVIDE_HIDDEN (__irel_start = .);
    *(.rel.iplt)
    PROVIDE_HIDDEN (__irel_end = .);
  }
  . = 0x09000;
  .iplt : { *(.iplt) }
  . = 0x0A000;
  .text : { *(.text) }
  . = 0x10000;
  .data : { *(.data) }
  . = 0x11000;
  .got : { *(.got.plt) *(.igot.plt) *(.got) *(.igot) }
  . = 0x12000;
  .bss : { *(.bss) }
}
