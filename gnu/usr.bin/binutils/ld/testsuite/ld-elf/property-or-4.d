#source: property-or-1.s
#source: property-or-2.s
#source: property-or-3.s
#as: --generate-missing-build-notes=no
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
      Properties: UINT32_OR \(0xb000ffff\): 0x1103
#pass
