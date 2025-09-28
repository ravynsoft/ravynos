SECTIONS
{
        . = 0x0;
        .text :
        {
                *(.text)
        }

        . = 0x700;
        .text1 :
        {
                *(.text1)
        }
        . = 0x8100;
        .bss :
        {
                *(.bss)
        }
        .data :
        {
                *(.data)
        }
}
