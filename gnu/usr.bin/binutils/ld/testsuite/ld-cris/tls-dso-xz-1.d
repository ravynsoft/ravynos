#source: tls128.s
#source: tls-hx1x2.s
#source: tls-x.s
#source: tls-z.s
#as: --no-underscore --em=criself
#ld: -m crislinux --shared
#objdump: -T

# A DSO providing the TLS variables x and z.

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
#...
0+88 g    D  \.tdata	0+4 x
#...
0+8c g    D  \.tdata	0+4 z
#...
