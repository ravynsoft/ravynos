SECTIONS
{
  . = SIZEOF_HEADERS;
  .text :
  {
    *(.text)
  } =0
  .post_text_reserve :
  {
    . += 0x160;
  }
  /DISCARD/ : { *(.*) }
}
