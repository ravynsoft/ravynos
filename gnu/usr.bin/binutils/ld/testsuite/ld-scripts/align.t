SECTIONS
{
  .text : {*(.text .pr)}
  .data ALIGN(0x1000) : AT (ALIGN (LOADADDR (.text) + SIZEOF (.text), 0x80))
    {}
  ASSERT (LOADADDR(.data) == 0x80, "dyadic ALIGN broken")
  ASSERT (ADDR(.data) == 0x1000, "monadic ALIGN broken")
}
