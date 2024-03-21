#source: report-reloc-1.s
#as: --64
#ld: -pie -melf_x86_64 -z report-relative-reloc $NO_DT_RELR_LDFLAGS
#warning_output: report-reloc-1.l
#readelf: -r --wide

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 2 entries:
 +Offset +Info +Type +Sym.* Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_RELATIVE +[0-9a-f]+
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_IRELATIVE +[0-9a-f]+
