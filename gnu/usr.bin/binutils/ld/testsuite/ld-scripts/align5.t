SECTIONS
{
  .text : {
    SORT_NONE (*) (.text .text.* .pr)
  }

  .data : {
    SORT_NONE (*) (.data .data.* .rw)
    foo = .;
  }
  .bss : {
    SORT_NONE (*) (.bss)
  }
  /DISCARD/ : {*(*)}
}
