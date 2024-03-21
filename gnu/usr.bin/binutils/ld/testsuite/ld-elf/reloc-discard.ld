SECTIONS
{
  /* .dynamic needs to go first with MIPS IRIX-style emulations.  */
  .dynamic : { *(.dynamic) }
  /DISCARD/ : { *(.rel.*) *(.rela.*) }
}
