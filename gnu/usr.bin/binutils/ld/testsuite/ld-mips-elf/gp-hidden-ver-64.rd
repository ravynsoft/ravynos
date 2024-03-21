
Relocation section '\.rel\.dyn' at offset .* contains 3 entries:
 *Offset * Info * Type * Sym\. *Value * Sym\. *Name
[0-9a-f]+ * 0+00000000 * R_MIPS_NONE *
 * Type2: R_MIPS_NONE *
 * Type3: R_MIPS_NONE *
# This must be an absolute relocation, there must not be a _gp reference.
[0-9a-f]+ * 0+00001203 * R_MIPS_REL32 *
 * Type2: R_MIPS_64 *
 * Type3: R_MIPS_NONE *
[0-9a-f]+ * [0-9a-f]+00001203 * R_MIPS_REL32 * [0-9a-f]+ * bar
 * Type2: R_MIPS_64 *
 * Type3: R_MIPS_NONE *
