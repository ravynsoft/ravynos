#DUMPPROG: readelf
#readelf: -Wr
#name: MIPS ELF reloc 27
#as: -32

Relocation section '\.rel\.text' at offset .* contains [34] entries:
 *Offset * Info * Type * Sym\. Value * Symbol's Name
[0-9a-f]+ * [0-9a-f]+ R_(MIPS|MIPS16|MICROMIPS)_HI16 * [0-9a-f]+ * (\.text|\.L0)
[0-9a-f]+ * [0-9a-f]+ R_(MIPS|MIPS16|MICROMIPS)_HI16 * [0-9a-f]+ * (\.text|\.L0)
[0-9a-f]+ * [0-9a-f]+ R_(MIPS|MIPS16|MICROMIPS)_LO16 * [0-9a-f]+ * (\.text|\.L0)
# There's an extra R_MICROMIPS_PC10_S1 relocation here for microMIPS
# assembly.  We don't care about it and the entry count regexp above
# catches other possible discrepancies, hence:
#pass
