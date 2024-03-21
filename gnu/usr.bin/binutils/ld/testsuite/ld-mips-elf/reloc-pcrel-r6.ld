SECTIONS
{
  x1 = 0x10000001;	/* lower and far  */
  x2 = 0x3ffff000;	/* lower and near  */
  . = 0x40000000;
  .text : { *(.text) }
  x3 = 0x4000f000;	/* high and near  */
  x4 = 0x80000001;	/* high and far  */
}
