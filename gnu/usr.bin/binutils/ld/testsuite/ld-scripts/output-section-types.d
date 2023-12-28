#ld: -Toutput-section-types.t
#source: align2a.s
#readelf: -S --wide
#target: [is_elf_format]

#...
.* .rom          +NOBITS        +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +00 +A +0 +0 +[1248]
.* .ro           +PROGBITS      +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +00 +A +0 +0 +[1248]
.* .over         +PROGBITS      +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +00  + +0 +0 +[1248]
.* progbits      +PROGBITS      +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +00 +A +0 +0 +[1248]
.* strtab        +STRTAB        +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +00 +A +0 +0 +[1248]
.* note          +NOTE          +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +00 +A +0 +0 +[1248]
.* init_array    +INIT_ARRAY    +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +0[48] +A +0 +0 +[1248]
.* fini_array    +FINI_ARRAY    +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +0[48] +A +0 +0 +[1248]
.* preinit_array +PREINIT_ARRAY +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +0[48] +A +0 +0 +[1248]
.* .ro.note      +NOTE          +[0-9a-f]+ +[0-9a-f]+ +[0-9a-f]+ +00 +A +0 +0 +[1248]
#pass
