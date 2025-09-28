ENTRY (test)
_start_text_phys = 0x88000000;
_start_text = _start_text_phys;

SECTIONS
{
  .text _start_text : AT (ADDR (.text)) { *(.text) }
}
