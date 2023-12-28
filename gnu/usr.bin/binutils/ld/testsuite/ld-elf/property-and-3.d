#source: property-and-1.s
#source: property-and-empty.s
#source: property-and-2.s
#as: --generate-missing-build-notes=no
#ld: -shared
#readelf: -n
#xfail: ![check_shared_lib_support]
#notarget: am33_2.0-*-* hppa*-*-hpux* mn10300-*-*
# Assembly source file for the HPPA assembler is renamed and modifed by
# sed.  mn10300 has relocations in .note.gnu.property section which
# elf_parse_notes doesn't support.

#failif
#...
Displaying notes found in: .note.gnu.property
#pass
