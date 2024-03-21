#source: variant_pcs-1.s
#source: variant_pcs-2.s
#ld: -r
#readelf: -rsW

Relocation section '\.rela\.text' at offset .* contains 24 entries:
    Offset             Info             Type               Symbol's Value  Symbol's Name \+ Addend
0000000000000000  000000180000011b R_AARCH64_CALL26       0000000000000000 f_spec_global_default_def \+ 0
0000000000000004  000000110000011b R_AARCH64_CALL26       0000000000000000 f_spec_global_default_undef \+ 0
0000000000000008  000000120000011b R_AARCH64_CALL26       0000000000000000 f_spec_global_hidden_def \+ 0
0000000000000010  000000170000011b R_AARCH64_CALL26       0000000000000000 f_base_global_default_def \+ 0
0000000000000014  000000100000011b R_AARCH64_CALL26       0000000000000000 f_base_global_default_undef \+ 0
0000000000000018  000000150000011b R_AARCH64_CALL26       0000000000000000 f_base_global_hidden_def \+ 0
0000000000000020  000000140000011b R_AARCH64_CALL26       f_spec_global_default_ifunc\(\) f_spec_global_default_ifunc \+ 0
0000000000000024  000000160000011b R_AARCH64_CALL26       f_spec_global_hidden_ifunc\(\) f_spec_global_hidden_ifunc \+ 0
0000000000000028  000000060000011b R_AARCH64_CALL26       f_spec_local_ifunc\(\) f_spec_local_ifunc \+ 0
000000000000002c  000000190000011b R_AARCH64_CALL26       f_base_global_default_ifunc\(\) f_base_global_default_ifunc \+ 0
0000000000000030  000000130000011b R_AARCH64_CALL26       f_base_global_hidden_ifunc\(\) f_base_global_hidden_ifunc \+ 0
0000000000000034  000000070000011b R_AARCH64_CALL26       f_base_local_ifunc\(\) f_base_local_ifunc \+ 0
0000000000000038  000000180000011b R_AARCH64_CALL26       0000000000000000 f_spec_global_default_def \+ 0
000000000000003c  000000110000011b R_AARCH64_CALL26       0000000000000000 f_spec_global_default_undef \+ 0
0000000000000040  000000120000011b R_AARCH64_CALL26       0000000000000000 f_spec_global_hidden_def \+ 0
0000000000000048  000000170000011b R_AARCH64_CALL26       0000000000000000 f_base_global_default_def \+ 0
000000000000004c  000000100000011b R_AARCH64_CALL26       0000000000000000 f_base_global_default_undef \+ 0
0000000000000050  000000150000011b R_AARCH64_CALL26       0000000000000000 f_base_global_hidden_def \+ 0
0000000000000058  000000140000011b R_AARCH64_CALL26       f_spec_global_default_ifunc\(\) f_spec_global_default_ifunc \+ 0
000000000000005c  000000160000011b R_AARCH64_CALL26       f_spec_global_hidden_ifunc\(\) f_spec_global_hidden_ifunc \+ 0
0000000000000060  0000000c0000011b R_AARCH64_CALL26       f_spec_local2_ifunc\(\) f_spec_local2_ifunc \+ 0
0000000000000064  000000190000011b R_AARCH64_CALL26       f_base_global_default_ifunc\(\) f_base_global_default_ifunc \+ 0
0000000000000068  000000130000011b R_AARCH64_CALL26       f_base_global_hidden_ifunc\(\) f_base_global_hidden_ifunc \+ 0
000000000000006c  0000000d0000011b R_AARCH64_CALL26       f_base_local2_ifunc\(\) f_base_local2_ifunc \+ 0

Symbol table '\.symtab' contains 26 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 SECTION LOCAL  DEFAULT    1.*
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    3.*
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    4.*
     4: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS .*variant_pcs-1\.o
     5: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_local
     6: 0000000000000000     0 IFUNC   LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_local_ifunc
     7: 0000000000000000     0 IFUNC   LOCAL  DEFAULT    1 f_base_local_ifunc
     8: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    1 f_base_local
     9: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    1 \$x
    10: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS .*variant_pcs-2\.o
    11: 0000000000000038     0 NOTYPE  LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_local2
    12: 0000000000000038     0 IFUNC   LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_local2_ifunc
    13: 0000000000000038     0 IFUNC   LOCAL  DEFAULT    1 f_base_local2_ifunc
    14: 0000000000000038     0 NOTYPE  LOCAL  DEFAULT    1 f_base_local2
    15: 0000000000000038     0 NOTYPE  LOCAL  DEFAULT    1 \$x
    16: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND f_base_global_default_undef
    17: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT \[VARIANT_PCS\]   UND f_spec_global_default_undef
    18: 0000000000000000     0 NOTYPE  GLOBAL HIDDEN  \[VARIANT_PCS\]     1 f_spec_global_hidden_def
    19: 0000000000000000     0 IFUNC   GLOBAL HIDDEN     1 f_base_global_hidden_ifunc
    20: 0000000000000000     0 IFUNC   GLOBAL DEFAULT \[VARIANT_PCS\]     1 f_spec_global_default_ifunc
    21: 0000000000000000     0 NOTYPE  GLOBAL HIDDEN     1 f_base_global_hidden_def
    22: 0000000000000000     0 IFUNC   GLOBAL HIDDEN  \[VARIANT_PCS\]     1 f_spec_global_hidden_ifunc
    23: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT    1 f_base_global_default_def
    24: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT \[VARIANT_PCS\]     1 f_spec_global_default_def
    25: 0000000000000000     0 IFUNC   GLOBAL DEFAULT    1 f_base_global_default_ifunc
