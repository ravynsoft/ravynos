#source: variant_cc-1.s
#source: variant_cc-2.s
#as: -mno-relax
#ld: -r
#readelf: -rsW

Relocation section '.rela.text' at .*
#...
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+cc_global_default_def \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+cc_global_default_undef \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+cc_global_hidden_def \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+cc_global_default_ifunc\(\)[ 	]+cc_global_default_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+cc_global_hidden_ifunc\(\)[ 	]+cc_global_hidden_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+cc_local \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+cc_local_ifunc\(\)[ 	]+cc_local_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+nocc_global_default_def \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+nocc_global_default_undef \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+nocc_global_hidden_def \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+nocc_global_default_ifunc\(\)[ 	]+nocc_global_default_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+nocc_global_hidden_ifunc\(\)[ 	]+nocc_global_hidden_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+nocc_local \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+nocc_local_ifunc\(\)[ 	]+nocc_local_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+cc_global_default_def \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+cc_global_default_undef \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+cc_global_hidden_def \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+cc_global_default_ifunc\(\)[ 	]+cc_global_default_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+cc_global_hidden_ifunc\(\)[ 	]+cc_global_hidden_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0070[ 	]+cc_local2 \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+cc_local2_ifunc\(\)[ 	]+cc_local2_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+nocc_global_default_def \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+nocc_global_default_undef \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0000[ 	]+nocc_global_hidden_def \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+nocc_global_default_ifunc\(\)[ 	]+nocc_global_default_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+nocc_global_hidden_ifunc\(\)[ 	]+nocc_global_hidden_ifunc \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+0+0070[ 	]+nocc_local2 \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_CALL_PLT[ 	]+nocc_local2_ifunc\(\)[ 	]+nocc_local2_ifunc \+ 0
#...
Symbol table '.symtab' contains .*
.*
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+NOTYPE[ 	]+LOCAL[ 	]+DEFAULT[ 	]+\[VARIANT_CC\][ 	]+1[ 	]+cc_local
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+IFUNC[ 	]+LOCAL[ 	]+DEFAULT[ 	]+\[VARIANT_CC\][ 	]+1[ 	]+cc_local_ifunc
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+IFUNC[ 	]+LOCAL[ 	]+DEFAULT[ 	]+1[ 	]+nocc_local_ifunc
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+NOTYPE[ 	]+LOCAL[ 	]+DEFAULT[ 	]+1[ 	]+nocc_local
#...
[ 	]+[0-9a-f]+:[ 	]+0+0070[ 	]+0[ 	]+NOTYPE[ 	]+LOCAL[ 	]+DEFAULT[ 	]+\[VARIANT_CC\][ 	]+1[ 	]+cc_local2
#...
[ 	]+[0-9a-f]+:[ 	]+0+0070[ 	]+0[ 	]+IFUNC[ 	]+LOCAL[ 	]+DEFAULT[ 	]+\[VARIANT_CC\][ 	]+1[ 	]+cc_local2_ifunc
#...
[ 	]+[0-9a-f]+:[ 	]+0+0070[ 	]+0[ 	]+IFUNC[ 	]+LOCAL[ 	]+DEFAULT[ 	]+1[ 	]+nocc_local2_ifunc
#...
[ 	]+[0-9a-f]+:[ 	]+0+0070[ 	]+0[ 	]+NOTYPE[ 	]+LOCAL[ 	]+DEFAULT[ 	]+1[ 	]+nocc_local2
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+NOTYPE[ 	]+GLOBAL[ 	]+DEFAULT[ 	]+UND[ 	]+nocc_global_default_undef
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+NOTYPE[ 	]+GLOBAL[ 	]+HIDDEN[ 	]+\[VARIANT_CC\][ 	]+1[ 	]+cc_global_hidden_def
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+NOTYPE[ 	]+GLOBAL[ 	]+DEFAULT[ 	]+\[VARIANT_CC\][ 	]+UND[ 	]+cc_global_default_undef
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+IFUNC[ 	]+GLOBAL[ 	]+DEFAULT[ 	]+\[VARIANT_CC\][ 	]+1[ 	]+cc_global_default_ifunc
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+NOTYPE[ 	]+GLOBAL[ 	]+HIDDEN[ 	]+1[ 	]+nocc_global_hidden_def
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+IFUNC[ 	]+GLOBAL[ 	]+HIDDEN[ 	]+1[ 	]+nocc_global_hidden_ifunc
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+IFUNC[ 	]+GLOBAL[ 	]+DEFAULT[ 	]+1[ 	]+nocc_global_default_ifunc
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+NOTYPE[ 	]+GLOBAL[ 	]+DEFAULT[ 	]+\[VARIANT_CC\][ 	]+1[ 	]+cc_global_default_def
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+NOTYPE[ 	]+GLOBAL[ 	]+DEFAULT[ 	]+1[ 	]+nocc_global_default_def
#...
[ 	]+[0-9a-f]+:[ 	]+0+0000[ 	]+0[ 	]+IFUNC[ 	]+GLOBAL[ 	]+HIDDEN[ 	]+\[VARIANT_CC\][ 	]+1[ 	]+cc_global_hidden_ifunc
#...
