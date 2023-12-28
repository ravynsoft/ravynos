#name: Undefined local label error
# COFF and aout based ports, except Windows CE, 
# use a different naming convention for local labels.
#skip: *-unknown-pe *-*-vxworks
#error_output: undefined.l
