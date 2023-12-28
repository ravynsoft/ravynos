#name: MIPS reloc estimation 1
#source: reloc-estimate-1a.s
#source: reloc-estimate-1b.s
#ld: -shared -T reloc-estimate-1.ld
#objdump: -R -sj.foo

.*

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
0+000000 R_MIPS_NONE       \*ABS\*
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+010000 R_MIPS_REL32      foo@@V2
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*


# The address must be 0x810.  We should only ever allocate one dynamic
# reloc over and above the first R_MIPS_NONE entry.
Contents of section \.foo:
 0810 (deadbeef|efbeadde)                             ....            
