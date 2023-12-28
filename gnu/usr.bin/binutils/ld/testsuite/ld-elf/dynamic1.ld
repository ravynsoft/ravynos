SECTIONS
{
  . = SIZEOF_HEADERS;
  .text : { *(.text) }
  .data : { *(.data) }
  .data1 : { KEEP (*(.data1)) }
  .dynamic : { *(.dynamic) }
}
