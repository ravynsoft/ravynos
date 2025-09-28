SECTIONS
{
  .text : { SORT_BY_NAME(*)(.text*) }
  .data : { *(.data*) }
  /DISCARD/ : { *(.*) }
}
