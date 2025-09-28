SECTIONS
{
  . = SIZEOF_HEADERS;
  .text : { *(.text) }
  OVERLAY 0x1000 : AT (0x4000)
  {
    .text1 {*(.text1)}
    .text2 {*(.text2)}
    .silly-name1 { *(.silly-name1) } = 0
    .silly-name2 { *(.silly-name2) } = 0
  } = 0
  .silly-name3 : { *(.silly-name3) } = 0
  .silly-name4 : { *(.silly-name4) } = 0
  /DISCARD/ : { *(.*) }
  ASSERT(ADDR(.text1)==ADDR(.text2), "overlay error")
  ASSERT(ADDR(.silly-name1)==ADDR(.silly-name2), "silly overlay error")
}
