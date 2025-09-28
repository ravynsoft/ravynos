OUTPUT_FORMAT("binary")
ENTRY(start)
SECTIONS
{
    . = 0x8000000000100000;
    .text : AT(ADDR(.text) - 0x8000000000100000)
    {
        *(.text)
        *(.data)
        *(.rodata*)
        *(COMMON*)
        *(.bss*)
    }
}
