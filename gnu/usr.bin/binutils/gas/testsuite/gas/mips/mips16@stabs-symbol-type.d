#DUMPPROG: readelf
#readelf: -s
#name: MIPS .stab symbol type
#as: -32
#source: stabs-symbol-type.s

# Verify the symbol type when emitting a .stab directive.
# In this case, it should be MIPS16.
#...
 *[0-9]+: +[0-9]+ +[0-9]+ +NOTYPE +LOCAL +DEFAULT +\[MIPS16\] +[0-9]+ foo
#pass
