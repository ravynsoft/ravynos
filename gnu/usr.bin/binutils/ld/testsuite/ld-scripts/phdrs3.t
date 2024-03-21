PHDRS
{
  data PT_LOAD ;
  header PT_PHDR PHDRS ; /* OK */
  text PT_LOAD FILEHDR PHDRS ;
}

SECTIONS
{
  /* This test will fail on architectures where the startaddress below
     is less than the constant MAXPAGESIZE.  */
  . = 0x800000 + SIZEOF_HEADERS;
  .text : { *(.text) } :text
  .data : { *(.data) } :data
  /DISCARD/ : { *(.*) }
}
