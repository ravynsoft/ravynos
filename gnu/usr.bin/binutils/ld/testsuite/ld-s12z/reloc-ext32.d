#source: reloc-ext32.s
#ld: --no-relax --defsym foobar=0x12345678
#objdump: -r -s  --section=.text

tmpdir/dump:     file format elf32-s12z

Contents of section .text:
 fe0000 01123456 7801                        ..4Vx.          
