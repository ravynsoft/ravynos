PROVIDE (mem_origin = 0x100);
PROVIDE (mem_length = 0x200);

MEMORY
{
  FOO : ORIGIN = mem_origin, LENGTH = mem_length
}

SECTIONS
{
  .data : {
    *(.data .data.*)
  } >FOO

  .text : {
    *(.text .text.*)
  } >FOO

  .bss : {
    *(.bss .bss.*)
  } >FOO

  /DISCARD/ : { *(.*) }
}

