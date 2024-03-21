PHDRS
{
  header PT_PHDR PHDRS ;
	 
  image PT_LOAD FLAGS (5) PHDRS;
  tls PT_TLS FLAGS (4);
  
}
SECTIONS
{
  .text 0x100 : { *(.text) } :image
  .tdata : { *(.tdata) } :image :tls
  .tbss : { *(.tbss) } :image : tls
  .map : {
    LONG (SIZEOF (.text))
    LONG (SIZEOF (.tdata))
    LONG (SIZEOF (.tbss))
  } :image
  /DISCARD/ : { *(*) }
}
