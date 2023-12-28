PROVIDE ( __start = DEFINED(start) ? start : 0xbfc00000);
ENTRY (__start)
SECTIONS
{
  .text :
  {
    *(.text)
  }
}
