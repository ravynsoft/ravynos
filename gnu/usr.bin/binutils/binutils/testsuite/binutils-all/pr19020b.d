#PROG: objcopy
#source: pr19020.in
#as: binary
#objcopy: -O binary -I binary --pad-to=10 --gap-fill=65 --reverse-bytes=8
#objdump: -b binary -s

#...
Contents of section .data:
 0000 68676665 64636261 4141 +hgfedcbaAA +
