#source: dso-2.s
#source: dsofnf.s
#source: gotrel1.s
#source: dso-1.s
#as: --pic --no-underscore --em=criself
#ld: -shared -m crislinux -z nocombreloc --hash-style=sysv
#objdump: -sR

# Like gotplt2, but make sure we merge right when we have a
# definition of the function too.

.*:     file format elf32-cris

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
000021e8 R_CRIS_GLOB_DAT   dsofn

Contents of section .*
#...
Contents of section \.rela\.got:
 013c e8210000 0a050000 00000000           .*
Contents of section \.text:
 0148 5f1d0c00 30096f1d 0c000000 30090000  .*
 0158 6f0d0c00 0000611a 6f3e84df ffff0000  .*
 0168 0f050000                             .*
Contents of section \.dynamic:
 216c 04000000 94000000 05000000 20010000  .*
 217c 06000000 c0000000 0a000000 19000000  .*
 218c 0b000000 10000000 07000000 3c010000  .*
 219c 08000000 0c000000 09000000 0c000000  .*
 21ac 00000000 00000000 00000000 00000000  .*
 21bc 00000000 00000000 00000000 00000000  .*
 21cc 00000000 00000000 00000000 00000000  .*
Contents of section \.got:
 21dc 6c210000 00000000 00000000 00000000  .*
