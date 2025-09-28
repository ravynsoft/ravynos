SECTIONS
{
  .text :
  {
    *(.text)
    *(.pr)
  }

  .data :
  {
    *(.data)
    *(.rw)
  }

  /DISCARD/ : { *(.note.gnu.property) }
}
