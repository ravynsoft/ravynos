PHDRS
{
  text PT_LOAD FILEHDR PHDRS ;
  note PT_NOTE;
}
SECTIONS
{
  . = . + SIZEOF_HEADERS ;
  .text : { *(.text) *(.rodata) } :text
  .note : { *(.note) } :note :text

  /* BUG:  This linker script is broken here.  It has not reset the
     output segment for the following sections, so they are all
     treated as notes...   */
  
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  
  .dynstr : { *(.dynstr) }
  .dynsym : { *(.dynsym) }
  .got : { *(.got .toc) *(.igot) }
  .got.plt : { *(.got.plt) *(.igot.plt) }
  /DISCARD/ : { *(*) }
}
