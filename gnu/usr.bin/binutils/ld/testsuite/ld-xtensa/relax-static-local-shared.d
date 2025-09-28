#source: relax-static-local.s
#source: relax-static-defs.s
#ld: -shared
#readelf: -r
#...
Relocation section '\.rela\.dyn' .* 6 .*
#...
.*R_XTENSA_RELATIVE.*
.*R_XTENSA_RELATIVE.*
.*R_XTENSA_RELATIVE.*
.*R_XTENSA_RELATIVE.*
.*R_XTENSA_RELATIVE.*
.*R_XTENSA_RELATIVE.*
#failif
#...
Relocation section '\.rela\.plt' .*
#...
