SECTIONS
{
  .text CONSTANT(COMMONPAGESIZE) : {
    *(.text)
  }

  .data : ALIGN(CONSTANT(COMMONPAGESIZE)) {
    *(.data)
  }
  /DISCARD/ : {*(*)}
}
