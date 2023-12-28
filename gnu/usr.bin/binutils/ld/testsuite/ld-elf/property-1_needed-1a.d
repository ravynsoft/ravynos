#source: empty.s
#source: property-1_needed-1.s
#as:
#ld: -shared
#readelf: -n
#xfail: ![check_shared_lib_support]
#notarget: am33_2.0-*-* hppa*-*-hpux* mn10300-*-*
# Assembly source file for the HPPA assembler is renamed and modifed by
# sed.  mn10300 has relocations in .note.gnu.property section which
# elf_parse_notes doesn't support.

#...
Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x[0-9a-f]+	NT_GNU_PROPERTY_TYPE_0
      Properties: 1_needed: indirect external access, <unknown: 2>
#pass
