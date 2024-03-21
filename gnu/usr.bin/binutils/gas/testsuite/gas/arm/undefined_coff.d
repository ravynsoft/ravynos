#name: Undefined local label error
# COFF and aout based ports, except Windows CE, 
# use a different naming convention for local labels.
#noskip: *-unknown-pe
#error_output: undefined_coff.l
