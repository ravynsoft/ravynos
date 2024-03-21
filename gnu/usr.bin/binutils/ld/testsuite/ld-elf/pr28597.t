SECTIONS
{
  .text           :
  {
    *(.text .text.*)
  }
  /DISCARD/ : { *(.dynsym) }
  /DISCARD/ : { *(.dynstr*) }
  /DISCARD/ : { *(.dynamic*) }
  /DISCARD/ : { *(.plt*) }
  /DISCARD/ : { *(.interp*) }
  /DISCARD/ : { *(.gnu*) }
  /DISCARD/ : { *(.note.gnu.property) }
}
