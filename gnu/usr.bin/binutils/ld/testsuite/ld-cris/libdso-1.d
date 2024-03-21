#source: dso-1.s
#as: --pic --no-underscore --em=criself
#ld: --shared -m crislinux
#objdump: -T

# Just check that we actually got a DSO with the dsofn symbol.
# The pattern also makes sure that the address (modulo 16) is non-zero
# and even.

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
#...
0+[^0]+0*[02468ace] g    DF .text	0+2 dsofn
#pass
