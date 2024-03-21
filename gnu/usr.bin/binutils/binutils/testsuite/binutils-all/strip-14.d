#PROG: strip
#strip: -g
#error: \A[^\n]*: relocation 0 has invalid symbol index 1048560\Z
#notarget: rx-*
# The RX targets do not complain about bad relocs, unless they are
#  actually used
#  (which is what should really happen with the other targets...)
