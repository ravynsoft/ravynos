# There should not be any NONE or RELATIVE relocs for foo
#failif
#...
Relocation section '\.rela?\.got' .*
 Offset +Info +Type .*
[0-9a-f]+ +[0-9a-f]+ +R_.*_(NONE|RELATIVE).*
#pass
