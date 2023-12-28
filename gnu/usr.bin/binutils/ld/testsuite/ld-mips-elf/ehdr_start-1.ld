ENTRY (__start)
SECTIONS
{
  . = 0x12300000 + SIZEOF_HEADERS;
  .text : { *(.text) }
  . = 0x23400000;
  HIDDEN (_gp = ALIGN (16) + 0x7ff0);
  .got : { *(.got) }
}
