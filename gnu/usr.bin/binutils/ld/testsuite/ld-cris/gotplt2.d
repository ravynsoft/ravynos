#source: dso-2.s
#source: dsofnf.s
#source: gotrel1.s
#as: --pic --no-underscore --em=criself
#ld: -shared -m crislinux -z nocombreloc --hash-style=sysv
#objdump: -sR

# Make sure we merge a PLT-specific entry (usually
# R_CRIS_JUMP_SLOT) with a GOT-specific entry (R_CRIS_GLOB_DAT)
# in a DSO.  It's ok: we make a round-trip to the PLT in the
# executable if it's referenced there, but that's still
# perceived as better than having an unnecessary PLT, dynamic
# reloc and lookup in the DSO.)

.*:     file format elf32-cris

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
000021e4 R_CRIS_GLOB_DAT   dsofn

Contents of section .*
#...
Contents of section \.rela\.got:
 013c e4210000 0a050000 00000000           .*
Contents of section \.text:
 0148 5f1d0c00 30096f1d 0c000000 30090000  .*
 0158 6f0d0c00 0000611a 6f3e88df ffff0000  .*
Contents of section \.dynamic:
 2168 04000000 94000000 05000000 20010000  .*
 2178 06000000 c0000000 0a000000 19000000  .*
 2188 0b000000 10000000 07000000 3c010000  .*
 2198 08000000 0c000000 09000000 0c000000  .*
 21a8 00000000 00000000 00000000 00000000  .*
 21b8 00000000 00000000 00000000 00000000  .*
 21c8 00000000 00000000 00000000 00000000  .*
Contents of section \.got:
 21d8 68210000 00000000 00000000 00000000  .*
