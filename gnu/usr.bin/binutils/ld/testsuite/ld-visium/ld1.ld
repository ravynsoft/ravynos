SECTIONS
{
  .text 0x10000 : {
    *(.text)
  }
  .data 0x30000 : {
    *(.data)
  }
}
