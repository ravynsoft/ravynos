SECTIONS
{
  .text 0x1000 :
  {
    *(.text)
    *(.pr)
  }

  .data :
  {
    *(.data)
    *(.rw)
  }
}
