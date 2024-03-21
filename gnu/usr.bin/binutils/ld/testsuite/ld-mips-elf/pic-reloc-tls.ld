SECTIONS
{
  bar = foo;
  .text : { *(.text) }
  .tbss : { *(.tbss) }
  /DISCARD/ : { *(*) }
}
