/* Script for ld testsuite */
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
  .text :
  {
    *(.text)
    *(.after1)
    *(.after2)
    *(.after3)
    *(.after4)
    *(.after5)
  } =0
}

