#nm: -n
#name: ARM Mapping Symbols Ignored
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

# Check ARM ELF Mapping Symbols are ignored properly
0+0 t sym1
0+c t sym2
