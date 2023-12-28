SECTIONS
{
  . = 0x80;
  .far : { *(.far) *(COMMON) }
  . = 0x100;
  .bss : { *(.scommon) }

  . = 0x10000000;
  .text : { *(.text*) }
  /DISCARD/ : { *(*) }
}
