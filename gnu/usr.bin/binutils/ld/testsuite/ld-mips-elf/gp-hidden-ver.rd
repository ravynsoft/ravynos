
Relocation section '\.rel\.dyn' at offset .* contains 3 entries:
 *Offset * Info * Type * Sym\. *Value * Sym\. *Name
[0-9a-f]+ * 0+00 * R_MIPS_NONE *
# This must be an absolute relocation, there must not be a _gp reference.
[0-9a-f]+ * 0+03 * R_MIPS_REL32 *
[0-9a-f]+ * [0-9a-f]+03 * R_MIPS_REL32 * [0-9a-f]+ * bar
