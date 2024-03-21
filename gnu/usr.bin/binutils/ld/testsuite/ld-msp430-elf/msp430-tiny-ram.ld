/* Script for ld testsuite */
OUTPUT_ARCH(msp430)
ENTRY(_start)

MEMORY
{
  RAM : ORIGIN = 0x0, LENGTH = 0x2
  ROM : ORIGIN = 0x2, LENGTH = 0x1fe
  HIFRAM : ORIGIN = 0x200, LENGTH = 0x1000
}

SECTIONS
{
  .text :
  {
    PROVIDE (_start = .);
    . = ALIGN(2);
    *(.text .stub .text.* .gnu.linkonce.t.* .text:*)
  } > ROM

  .rodata :
  {
    *(.upper.rodata.* .rodata)
  } > ROM

  .data :
  {
    . = ALIGN(2);
    *(.data.* .data)
  } > RAM AT> ROM

  .bss :
  {
    . = ALIGN(2);
    *(.bss.* .bss)
  } > RAM

  .upper.data :
  {
    . = ALIGN(2);
    *(.upper.data.* .upper.data)
  } > HIFRAM AT> ROM

  .upper.bss :
  {
    . = ALIGN(2);
    *(.upper.bss.* .upper.bss)
  } > HIFRAM
}
