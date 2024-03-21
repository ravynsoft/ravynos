SECTIONS {
  .rom  (NOLOAD)   : { LONG(1234); }
  .ro   (READONLY) : { LONG(5678); }
  .over (OVERLAY)  : { LONG(0123); }
  progbits (TYPE=SHT_PROGBITS) : { BYTE(1) }
  strtab (TYPE = SHT_STRTAB) : { BYTE(0) }
  note (TYPE =SHT_NOTE) : { BYTE(8) }
  init_array (TYPE= 14) : { QUAD(14) }
  fini_array ( TYPE=SHT_FINI_ARRAY) : { QUAD(15) }
  preinit_array (TYPE=SHT_PREINIT_ARRAY ) : { QUAD(16) }
  .ro.note   (READONLY (TYPE=SHT_NOTE)) : { LONG(5678); }
  /DISCARD/        : { *(*) }

}
