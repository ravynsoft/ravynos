#source: dso-2.s
#source: dsofnf2.s
#source: gotrel1.s
#as: --pic --no-underscore --em=criself
#ld: -m crislinux tmpdir/libdso-1.so --hash-style=sysv
#objdump: -sR

# Make sure we don't merge a PLT-specific entry
# (R_CRIS_JUMP_SLOT) with a non-PLT-GOT-specific entry
# (R_CRIS_GLOB_DAT) in an executable, since they may have
# different contents there.  (If we merge them in a DSO it's ok:
# we make a round-trip to the PLT in the executable if it's
# referenced there, but that's still perceived as better than
# having an unnecessary PLT, dynamic reloc and lookup in the
# DSO.)  In the executable, the GOT contents for the non-PLT
# reloc should be constant.

.*:     file format elf32-cris

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
00082224 R_CRIS_JUMP_SLOT  dsofn

Contents of section .*
#...
Contents of section \.rela\.plt:
 80134 24220800 0b010000 00000000           .*
Contents of section \.plt:
 80140 fce17e7e 7f0d1c22 0800307a 7f0d2022  .*
 80150 08003009 7f0d2422 08003009 3f7e0000  .*
 80160 00002ffe d8ffffff                    .*
Contents of section \.text:
 80168 5f1d0c00 30096f1d 0c000000 30090000  .*
 80178 6f0d1000 0000611a 6f2e5401 08000000  .*
 80188 6f3e70df ffff0000                    .*
Contents of section \.dynamic:
 82190 01000000 07000000 04000000 e4000800  .*
 821a0 05000000 18010800 06000000 f8000800  .*
 821b0 0a000000 1a000000 0b000000 10000000  .*
 821c0 15000000 00000000 03000000 18220800  .*
 821d0 02000000 0c000000 14000000 07000000  .*
 821e0 17000000 34010800 00000000 00000000  .*
 821f0 00000000 00000000 00000000 00000000  .*
 82200 00000000 00000000 00000000 00000000  .*
 82210 00000000 00000000                    .*
Contents of section \.got:
 82218 90210800 00000000 00000000 5c010800  .*
 82228 54010800                             .*
