/* Script for ld testsuite */
OUTPUT_ARCH(msp430)
ENTRY(_start)

SECTIONS
{
  .lower.data :
  {
    . = ALIGN(2);
    *(.lower.data .lower.data.*)
  }

  .lower.bss :
  {
    . = ALIGN(2);
    *(.lower.bss .lower.bss.*)
  }

  .lower.text :
  {
    PROVIDE (_start = .);
    . = ALIGN(2);
    *(.lower.text.* .lower.text)
  }

  .lower.rodata :
  {
    . = ALIGN(2);
    *(.lower.rodata .lower.rodata.*)
  }

  .text :
  {
    . = ALIGN(2);
    *(.text .stub .text.* .gnu.linkonce.t.* .text:*)
  }

  .rodata :
  {
    *(.rodata.* .rodata)
  }

  .data :
  {
    . = ALIGN(2);
    *(.data.* .data)
  }

  .bss :
  {
    . = ALIGN(2);
    *(.bss.* .bss)
  }

  .upper.text :
  {
    . = ALIGN(2);
    *(.upper.text.* .upper.text)
  }

  .upper.rodata :
  {
    . = ALIGN(2);
    *(.upper.rodata .upper.rodata.*)
  }

  .upper.data :
  {
    . = ALIGN(2);
    *(.upper.data .upper.data.*)
  }

  .upper.bss :
  {
    . = ALIGN(2);
    *(.upper.bss .upper.bss.*)
  }
}
