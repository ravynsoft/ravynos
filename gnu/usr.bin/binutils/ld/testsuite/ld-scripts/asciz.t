
_start = 0x000000;
SECTIONS
{
  . = 0x1000 + SIZEOF_HEADERS;
  
  .data : AT (0x10000)
  {
      ASCIZ "This is a string"
      ASCIZ "This is another\n\123tring"
      ASCIZ ""
      ASCIZ noquotes
  }
  
  /DISCARD/ : { *(*) }
}
