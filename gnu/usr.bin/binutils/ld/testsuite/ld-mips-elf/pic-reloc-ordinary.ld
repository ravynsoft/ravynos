SECTIONS
{
  bar = foo;
  .text : { *(.text) }
  /DISCARD/ : { *(*) }
}
