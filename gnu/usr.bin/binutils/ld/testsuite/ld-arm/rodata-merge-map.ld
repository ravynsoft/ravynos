/* Script for ld testsuite */
OUTPUT_ARCH(arm)
SECTIONS
{
  .rodata :
  {
    *(.rodata*)
  }
}
