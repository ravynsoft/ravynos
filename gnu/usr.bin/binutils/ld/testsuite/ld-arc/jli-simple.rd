
Relocation section '\.rela\.text' .*:
 Offset +Info +Type +Sym.Value +Sym. Name \+ Addend
00010054  [0-9a-f]+ R_ARC_JLI_SECTOFF 00010060   __jli\.foo \+ 0
00010056  [0-9a-f]+ R_ARC_JLI_SECTOFF 00010064   __jli\.bar \+ 0

Relocation section '\.rela\.jlitab' .*:
 Offset +Info +Type +Sym.Value +Sym. Name \+ Addend
00010060  [0-9a-f]+ R_ARC_S25H_PCREL  00010054   .text \+ 4
00010064  [0-9a-f]+ R_ARC_S25H_PCREL  00010054   .text \+ 8
