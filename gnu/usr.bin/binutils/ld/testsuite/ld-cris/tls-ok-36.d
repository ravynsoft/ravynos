#source: start1.s
#source: tls-tprelm.s --defsym r=32767
#as: --no-underscore --em=criself
#ld: -m crislinux
#objdump: -s -j .got -j .text -j .tdata

# Check that a R_CRIS_16_TPREL just below the theoretical limit works.

.*:     file format elf32-cris

Contents of section \.text:
 80094 41b20000 5fae0080                   .*
Contents of section \.tdata:
 8209c 2a2a2a2a 2a2a2a2a 2a2a2a2a 2a2a2a2a .*
#...
 8a08c 2a2a2a2a 2a2a2a2a 2a2a2a2a 2a2a2a2a .*
