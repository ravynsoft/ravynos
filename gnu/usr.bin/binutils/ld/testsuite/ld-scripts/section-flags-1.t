MEMORY
{
  ram (rwx) : ORIGIN = 0x100000, LENGTH = 144M
}

SECTIONS
{
  .text :
  {
    INPUT_SECTION_FLAGS (!SHF_TLS) *(.text .text.* .text_* .gnu.linkonce.t.*)
  } >ram

  .text_vle : 
  {
    INPUT_SECTION_FLAGS (SHF_MERGE & SHF_STRINGS & SHF_LINK_ORDER) *(.text .text.* .text_* .gnu.linkonce.t.*)
  } >ram
  .text_other :
  {
    INPUT_SECTION_FLAGS (SHF_MERGE & !SHF_STRINGS) *(.text .text.* .text_* .gnu.linkonce.t.*)
  }
}
