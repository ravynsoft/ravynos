SECTIONS
{
  . = SIZEOF_HEADERS;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss  : { *(.bss) *(COMMON) }
}
defined_pre = DEFINED (defined);
defined = 1;
defined_post = DEFINED (defined);
undef_pre = DEFINED (undef);
undef = 1;
undef_post = DEFINED (undef);
common_pre = DEFINED (common);
common = 1;
common_post = DEFINED (common);
weak_pre = DEFINED (weak);
weak = 1;
weak_post = DEFINED (weak);
undefweak_pre = DEFINED (undefweak);
undefweak = 1;
undefweak_post = DEFINED (undefweak);
