SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = SEGMENT_START("text-segment", 0) + SIZEOF_HEADERS;
  .hash           : { *(.hash) }
  .gnu.hash       : { *(.gnu.hash) }
  .dynsym         : { *(.dynsym) }
  .dynstr         : { *(.dynstr) }
  .init           : { *(.init) }
  .text           : { *(.text) }
  .fini           : { *(.fini) }
  .rodata         : { *(.rodata) }
  .foo_hdr : { *(.foo_hdr) }
  .foo : { *(.foo) }
  .xxx : { *(.xxx) }
  . = ALIGN (CONSTANT (MAXPAGESIZE)) - ((CONSTANT (MAXPAGESIZE) - .) & (CONSTANT (MAXPAGESIZE) - 1)); . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
  .yyy : { *(.yyy) }
  .tdata	  : { *(.tdata) }
  .tbss		  : { *(.tbss) }
  .init_array     : { *(.init_array) }
  .fini_array     : { *(.fini_array) }
  .jcr            : { *(.jcr) }
  .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*) }
  .dynamic        : { *(.dynamic) }
  .bar            : { *(.bar) }
  . = DATA_SEGMENT_RELRO_END (SIZEOF (.got.plt) >= 24 ? 24 : 0, .);
  .got.plt        : { *(.got.plt) }
  .data           : { *(.data) }
  __bss_start = .;
  .bss            :
  {
   *(.bss)
   . = ALIGN(. != 0 ? 64 / 8 : 1);
  }
  . = ALIGN(64 / 8);
  _end = .; PROVIDE (end = .);
  . = DATA_SEGMENT_END (.);
  /DISCARD/ : { *(.*) }
}
