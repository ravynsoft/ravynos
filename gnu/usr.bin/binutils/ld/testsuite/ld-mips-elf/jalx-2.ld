ENTRY (internal_function)
_start_text_phys = 0x4400000;
_start_text = _start_text_phys;

SECTIONS
{
  .text _start_text : AT (ADDR (.text)) { *(.text) }
  .plt : { *(.plt) }
  .rel.plt : { *(.rel.plt) }
}
