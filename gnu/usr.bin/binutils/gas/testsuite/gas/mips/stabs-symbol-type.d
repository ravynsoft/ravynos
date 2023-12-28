#DUMPPROG: readelf
#readelf: -s
#name: MIPS .stab symbol type
#as: -32

# Verify the symbol type when emitting a .stab directive.
# In this case, it should not be MIPS16 or MICROMIPS.
#...
 *[0-9]+: +[0-9]+ +[0-9]+ +NOTYPE +LOCAL +DEFAULT +[0-9]+ foo
#pass
