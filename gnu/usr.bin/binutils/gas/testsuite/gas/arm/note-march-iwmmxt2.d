# name: iWMMXt2 note with -march=iwmmxt2
# source: note-march-iwmmxt2.s RUN_OBJCOPY
# as: -march=iwmmxt2
# objcopy_objects: -R .ARM.attributes
# ld: -e 0x10000
# objcopy_linked_file:
# readelf: -p .note.gnu.arm.ident
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

String dump of section '.note.gnu.arm.ident':
  \[\s*[0-9a-f]+\]  arch: 
  \[\s*[0-9a-f]+\]  iWMMXt2
