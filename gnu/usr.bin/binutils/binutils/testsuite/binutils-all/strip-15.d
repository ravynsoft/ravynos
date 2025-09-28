#PROG: strip
#strip: -g
#readelf: -r

Relocation section '\.rela?\.text' at offset .* contains 2 entries:
 *Offset * Info * Type * Sym\. *Value * Sym\. *Name(?: * \+ * Addend)?
0+00 * 0+0(?:1|b|32|103) * R_[^ ]* *(?: * 55aa)?
#pass
