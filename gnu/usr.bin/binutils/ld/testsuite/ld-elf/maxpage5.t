OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_entry)
PHDRS
{
  data PT_LOAD;
  note PT_NOTE;
}
SECTIONS
{
  .text : { *(.text) } :data
  .foo : { *(.foo) } :data
  .note : { *(.note) } :note
  /DISCARD/ : { *(*) }
}
