#source: pr27590.s
#ld: -r tmpdir/pr27590.o
#readelf: -rW
#xfail: [is_generic]

Relocation section '\.rel.*\.gnu\.debuglto_\.debug_macro' at offset 0x[0-9a-z]+ contains 2 entries:
[ \t]+Offset[ \t]+Info[ \t]+Type[ \t]+Sym.*
[0-9a-f]+[ \t]+[0-9a-f]+[ \t]+R_.*[ \t]+[0]+[ \t]+(\.gnu\.debuglto_\.debug_macro|\.Ldebug_macro2).*
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
[0-9a-f]+[ \t]+[0-9a-f]+[ \t]+R_.*[ \t]+[0]+[ \t]+(\.gnu\.debuglto_\.debug_macro|\.Ldebug_macro2).*
#pass
