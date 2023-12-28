SECTIONS
{
  .bar -0xa00000 : AT ((LOADADDR(.foo) + SIZEOF(.foo) + 4095) & ~(4095))
    { *(.bar) }
  . = LOADADDR(.bar) + 4096;
}
INSERT AFTER .foo;
