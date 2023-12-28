#source: tls-gdgotrelm.s --defsym r=8191
#as: --no-underscore --em=criself --pic
#ld: --shared -m crislinux --hash-style=sysv
#objdump: -s -j .got -R

# Verify that the first and last R_CRIS_16_GOT_GD entries are ok just
# below the limit, in a DSO.  Beware, the order here is quite random,
# supposedly depending on symbol hashes.

.*:     file format elf32-cris

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
000b38a8 R_CRIS_DTP        x2814
#...
000b82e8 R_CRIS_DTP        x8190
#...
000c12a0 R_CRIS_DTP        x0
#...
000c3898 R_CRIS_DTP        x1345

Contents of section .got:
 b389c 2c380b00 00000000 00000000 00000000  .*
 b38ac 00000000 00000000 00000000 00000000  .*
#...
 c387c 00000000 00000000 00000000 00000000  .*
 c388c 00000000 00000000 00000000 00000000  .*
 c389c 00000000                             .*
#PASS
