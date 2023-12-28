/* everything in one segment */

SECTIONS
{
.text 0x00001000 :
        {
          *(.text)
          *(.strings)
          *(.rdata)
        }

.data 0x00002000 :
        {
           *(.data)
        }

.bss 0x00003000 :
        {
          *(.bss);
          *(COMMON);
        }
}
