# name: armv2a note with -march=armv2a
# source: note-march-armv2a.s RUN_OBJCOPY
# as: -march=armv2a
# objcopy_objects: -R .ARM.attributes
# ld: -e 0x10000
# objcopy_linked_file:
# readelf: -p .note.gnu.arm.ident
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

String dump of section '.note.gnu.arm.ident':
  \[\s*[0-9a-f]+\]  arch: 
  \[\s*[0-9a-f]+\]  armv2a
