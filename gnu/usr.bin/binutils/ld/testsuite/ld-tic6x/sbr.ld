EXTERN (__c6xabi_DSBT_BASE);
SECTIONS
{
  . = 0x80;
  .data : { PROVIDE_HIDDEN (__c6xabi_DSBT_BASE = .); *(.data*) }
  . = 0x10000000;
  .text : { *(.text*) }
  /DISCARD/ : { *(*) }
}
