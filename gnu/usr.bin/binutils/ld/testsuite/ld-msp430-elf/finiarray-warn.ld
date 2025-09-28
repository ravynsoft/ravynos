/* Script for ld testsuite */
OUTPUT_ARCH(msp430)
ENTRY(_start)

SECTIONS
{
  .text :
  {
    PROVIDE (_start = .);
    . = ALIGN(2);
    *(.text .stub .text.* .gnu.linkonce.t.* .text:*)
  }

  .rodata :
  {
    *(.rodata.* .rodata)
    . = ALIGN(2);
    PROVIDE (__preinit_array_start = .);
    KEEP (*(SORT(.preinit_array.*)))
    KEEP (*(.preinit_array))
    PROVIDE (__preinit_array_end = .);
    . = ALIGN(2);
    PROVIDE (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE (__init_array_end = .);
    . = ALIGN(2);
    . += 1;
    PROVIDE (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array))
    PROVIDE (__fini_array_end = .);
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
}
