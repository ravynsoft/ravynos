SECTIONS
{
  .bar -0x7fef0000 : AT ((LOADADDR(.foo) + SIZEOF(.foo) + 4095) & ~(4095))
    { *(.bar) }
  . = LOADADDR(.bar) + 0x200000;
}
INSERT BEFORE .data;
