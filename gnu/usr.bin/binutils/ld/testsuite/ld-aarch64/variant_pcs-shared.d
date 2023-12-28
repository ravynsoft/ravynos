#source: variant_pcs-1.s
#source: variant_pcs-2.s
#target: [check_shared_lib_support]
#ld: -shared --hash-style=sysv -T variant_pcs.ld
#readelf: -rsW

Relocation section '\.rela\.plt' at offset 0x11000 contains 12 entries:
    Offset             Info             Type               Symbol's Value  Symbol's Name \+ Addend
0000000000009020  0000000100000402 R_AARCH64_JUMP_SLOT    0000000000000000 f_base_global_default_undef \+ 0
0000000000009028  0000000200000402 R_AARCH64_JUMP_SLOT    0000000000000000 f_spec_global_default_undef \+ 0
0000000000009030  0000000400000402 R_AARCH64_JUMP_SLOT    0000000000008000 f_base_global_default_def \+ 0
0000000000009038  0000000500000402 R_AARCH64_JUMP_SLOT    0000000000008000 f_spec_global_default_def \+ 0
0000000000009040  0000000000000408 R_AARCH64_IRELATIVE                       8000
0000000000009048  0000000300000402 R_AARCH64_JUMP_SLOT    f_spec_global_default_ifunc\(\) f_spec_global_default_ifunc \+ 0
0000000000009050  0000000000000408 R_AARCH64_IRELATIVE                       8000
0000000000009058  0000000600000402 R_AARCH64_JUMP_SLOT    f_base_global_default_ifunc\(\) f_base_global_default_ifunc \+ 0
0000000000009060  0000000000000408 R_AARCH64_IRELATIVE                       8038
0000000000009068  0000000000000408 R_AARCH64_IRELATIVE                       8000
0000000000009070  0000000000000408 R_AARCH64_IRELATIVE                       8000
0000000000009078  0000000000000408 R_AARCH64_IRELATIVE                       8038

Symbol table '\.dynsym' contains 7 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND f_base_global_default_undef
     2: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT \[VARIANT_PCS\]   UND f_spec_global_default_undef
     3: 0000000000008000     0 IFUNC   GLOBAL DEFAULT \[VARIANT_PCS\]     1 f_spec_global_default_ifunc
     4: 0000000000008000     0 NOTYPE  GLOBAL DEFAULT    1 f_base_global_default_def
     5: 0000000000008000     0 NOTYPE  GLOBAL DEFAULT \[VARIANT_PCS\]     1 f_spec_global_default_def
     6: 0000000000008000     0 IFUNC   GLOBAL DEFAULT    1 f_base_global_default_ifunc

Symbol table '\.symtab' contains 35 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000008000     0 SECTION LOCAL  DEFAULT    1.*
     2: 0000000000008070     0 SECTION LOCAL  DEFAULT    2.*
     3: 0000000000009000     0 SECTION LOCAL  DEFAULT    3.*
     4: 0000000000009080     0 SECTION LOCAL  DEFAULT    4.*
     5: 0000000000011000     0 SECTION LOCAL  DEFAULT    5.*
     6: 0000000000011120     0 SECTION LOCAL  DEFAULT    6.*
     7: 00000000000111c8     0 SECTION LOCAL  DEFAULT    7.*
     8: 0000000000011270     0 SECTION LOCAL  DEFAULT    8.*
     9: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS .*variant_pcs-1\.o
    10: 0000000000008000     0 NOTYPE  LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_local
    11: 0000000000008000     0 IFUNC   LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_local_ifunc
    12: 0000000000008000     0 IFUNC   LOCAL  DEFAULT    1 f_base_local_ifunc
    13: 0000000000008000     0 NOTYPE  LOCAL  DEFAULT    1 f_base_local
    14: 0000000000008000     0 NOTYPE  LOCAL  DEFAULT    1 \$x
    15: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS .*variant_pcs-2\.o
    16: 0000000000008038     0 NOTYPE  LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_local2
    17: 0000000000008038     0 IFUNC   LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_local2_ifunc
    18: 0000000000008038     0 IFUNC   LOCAL  DEFAULT    1 f_base_local2_ifunc
    19: 0000000000008038     0 NOTYPE  LOCAL  DEFAULT    1 f_base_local2
    20: 0000000000008038     0 NOTYPE  LOCAL  DEFAULT    1 \$x
    21: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS 
    22: 0000000000009080     0 OBJECT  LOCAL  DEFAULT  ABS _DYNAMIC
    23: 0000000000008000     0 NOTYPE  LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_global_hidden_def
    24: 0000000000008000     0 IFUNC   LOCAL  DEFAULT    1 f_base_global_hidden_ifunc
    25: 0000000000008000     0 NOTYPE  LOCAL  DEFAULT    1 f_base_global_hidden_def
    26: 0000000000009000     0 OBJECT  LOCAL  DEFAULT  ABS _GLOBAL_OFFSET_TABLE_
    27: 0000000000008000     0 IFUNC   LOCAL  DEFAULT \[VARIANT_PCS\]     1 f_spec_global_hidden_ifunc
    28: 0000000000008070     0 NOTYPE  LOCAL  DEFAULT    2 \$x
    29: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND f_base_global_default_undef
    30: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT \[VARIANT_PCS\]   UND f_spec_global_default_undef
    31: 0000000000008000     0 IFUNC   GLOBAL DEFAULT \[VARIANT_PCS\]     1 f_spec_global_default_ifunc
    32: 0000000000008000     0 NOTYPE  GLOBAL DEFAULT    1 f_base_global_default_def
    33: 0000000000008000     0 NOTYPE  GLOBAL DEFAULT \[VARIANT_PCS\]     1 f_spec_global_default_def
    34: 0000000000008000     0 IFUNC   GLOBAL DEFAULT    1 f_base_global_default_ifunc
