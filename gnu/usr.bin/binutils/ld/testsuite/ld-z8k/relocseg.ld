/* one segment for each of .text, .data, and .bss */

SECTIONS
{
.text 0x00011000 :
        {
          *(.text)
          *(.strings)
          *(.rdata)
        }

.data 0x00022000 :
        {
           *(.data)
        }

.bss 0x00033000 :
        {
          *(.bss);
          *(COMMON);
        }
}
