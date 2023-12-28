#name: -u --export-dynamic-symbol foo archive
#source: export-dynamic-symbol.s
#ld: -pie -u foo --export-dynamic-symbol foo --export-dynamic-symbol=bar tmpdir/libpr25910.a
#nm: -D
# mips-sgi-irix6 makes foo and bar SHN_MIPS_DATA (SHN_LOPROC+2) due to the
# testcase carelessly leaving them untyped which mips gas turns into
# STT_OBJECT type.  nm interprets SHN_MIPS_DATA (incorrectly) to be A type.

#...
[0-9a-f]+ [AT] +bar
[0-9a-f]+ [AT] +foo
#...
