#PROG: objcopy
#source: pr19020.in
#as: binary
#objcopy: -O binary -I binary --pad-to=10 --gap-fill=65 --interleave=2 --interleave-width=1 --byte=0
#objdump: -b binary -s

#...
Contents of section .data:
 0000 61636567 41414141 4141 +acegAAAAAA +
