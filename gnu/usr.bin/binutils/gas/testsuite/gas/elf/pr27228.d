#as:
#name: .reloc against undefined local symbol (PR 27228)
#readelf: -sW
# hppa doesn't support use of any BFD_RELOC_*
#notarget: hppa*-*-*

Symbol table '\.symtab' contains [0-9]+ entries:
#...
 +[a-f0-9]+: 0+ +0 (NOTYPE|OBJECT) +GLOBAL DEFAULT +UND .LTHUNK5.lto_priv.0
#pass
