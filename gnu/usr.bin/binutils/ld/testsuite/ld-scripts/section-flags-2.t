MEMORY
{
  ram (rwx) : ORIGIN = 0x100000, LENGTH = 144M
}

SECTIONS
{
  .text :
  {
    INPUT_SECTION_FLAGS (!SHF_TLS) *(EXCLUDE_FILE (section-flags-1.o) .text .text.* .text_* .gnu.linkonce.t.*)
  } >ram
}
