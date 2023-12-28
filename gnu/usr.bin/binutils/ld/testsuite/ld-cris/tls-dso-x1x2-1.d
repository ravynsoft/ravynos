#source: tls-hx.s
#source: tls-x1x2.s
#as: --no-underscore --em=criself
#ld: -m crislinux --shared --hash-style=sysv
#objdump: -T

# A DSO providing the TLS variables x1 and x2.

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
#...
0+8 g    D  \.tdata	0+4 x2
#...
0+4 g    D  \.tdata	0+4 x1
#...
