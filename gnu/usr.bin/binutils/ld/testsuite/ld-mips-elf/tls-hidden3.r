
Relocation section '\.rel\.dyn' at offset .* contains 6 entries:
 Offset     Info    Type            Sym\.Value  Sym\. Name
00000000  00000000 R_MIPS_NONE      
#
# The order of the next four entries doesn't matter.  The important thing
# is that there is exactly one entry per GOT TLS slot.
#
0009000c  0000002f R_MIPS_TLS_TPREL3
00090010  0000002f R_MIPS_TLS_TPREL3
00090014  0000002f R_MIPS_TLS_TPREL3
00090018  0000002f R_MIPS_TLS_TPREL3
00090020  .*03 R_MIPS_REL32      00000000   undef
