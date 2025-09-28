#PROG: strip
#strip: -g
#error: \A[^\n]*: unsupported relocation type 0x[0-9a-f]+\n
#error:   [^\n]*: bad value\Z
#notarget: rx-*
# The RX targets do not complain about unrecognised relocs, unless they
#  are actually used
#  (which is what should really happen with the other targets...)
