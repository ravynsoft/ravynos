
Relocation section '.rel.dyn' at offset 0x101b4 contains 8 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
00000000  .* R_MIPS_NONE *
#
# The order of the relocations doesn't really matter, but they must
# be some permutation of the list below.
#
00010008  .* R_MIPS_REL32      00000000   pers3
00000c7b  .* R_MIPS_REL32      00000828   global_pers
00000d7f  .* R_MIPS_REL32      00000000   extern_indirect_ptr
00010000  .* R_MIPS_REL32      00000000   pers1
00010004  .* R_MIPS_REL32      00000000   pers2
00000caf  .* R_MIPS_REL32      00000000   extern_pers
00000d4b  .* R_MIPS_REL32      00010008   global_indirect_ptr
Contents of the \.eh_frame section:

# Text addresses
# --------------
# f1 = 0x800
# f2 = 0x804
# f3 = 0x808
# f4 = 0x80c
# f5 = 0x810
# f6 = 0x814
# f7 = 0x818
# f8 = 0x81c
# local_pers = 0x820
# hidden_pers = 0x824
# global_pers = 0x828

# Data addresses
# --------------
# local_indirect_ptr = 0x10000
# hidden_indirect_ptr = 0x10004
# global_indirect_ptr = 0x10008
# LSDA = 0x1000c

#-------------------------------------------------------------------------
# f1
#-------------------------------------------------------------------------
00000000 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
#
# 0xc12: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for personality encoding
# 0xc13: 0x820 - 0xc13 (local_pers - .)
# 0xc17: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for LDSA encoding
# 0xc18: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for FDE encoding
#
  Augmentation data:     1b ff ff fc 0d 1b 1b

  DW_CFA_def_cfa_register: r29
  DW_CFA_nop

0000001c 00000014 00000020 FDE cie=00000000 pc=00000800..00000804
#
# 0xc2d: 0x1000c - 0xc2d (LDSA - .)
#
  Augmentation data:     00 00 f3 df

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

#-------------------------------------------------------------------------
# f2
#-------------------------------------------------------------------------
00000034 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
#
# 0xc46: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for personality encoding
# 0xc47: 0x824 - 0xc47 (hidden_pers - .)
# 0xc4d: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for LDSA encoding
# 0xc4e: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for FDE encoding
#
  Augmentation data:     1b ff ff fb dd 1b 1b

  DW_CFA_def_cfa_register: r29
  DW_CFA_nop

00000050 00000014 00000020 FDE cie=00000034 pc=00000804..00000808
#
# 0xc61: 0x1000c - 0xc61 (LDSA - .)
#
  Augmentation data:     00 00 f3 ab

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

#-------------------------------------------------------------------------
# f3
#-------------------------------------------------------------------------
00000068 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
#
# 0xc7a: DW_EH_PE_absptr for personality encoding
# 0xc7b: global_pers (reloc above)
# 0xc7f: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for LDSA encoding
# 0xc80: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for FDE encoding
#
  Augmentation data:     00 00 00 00 00 1b 1b

  DW_CFA_def_cfa_register: r29
  DW_CFA_nop

00000084 00000014 00000020 FDE cie=00000068 pc=00000808..0000080c
#
# 0xc95: 0x1000c - 0xc95 (LDSA - .)
#
  Augmentation data:     00 00 f3 77

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

#-------------------------------------------------------------------------
# f4
#-------------------------------------------------------------------------
0000009c 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
#
# 0xcae: DW_EH_PE_absptr for personality encoding
# 0xcaf: extern_pers (reloc above)
# 0xcb3: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for LDSA encoding
# 0xcb4: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for FDE encoding
#
  Augmentation data:     00 00 00 00 00 1b 1b

  DW_CFA_def_cfa_register: r29
  DW_CFA_nop

000000b8 00000014 00000020 FDE cie=0000009c pc=0000080c..00000810
#
# 0xcc9: 0x1000c - 0xcc9 (LDSA - .)
#
  Augmentation data:     00 00 f3 43

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

#-------------------------------------------------------------------------
# f5
#-------------------------------------------------------------------------
000000d0 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
#
# 0xce2: DW_EH_PE_indirect | DW_EH_PE_pcrel | DW_EH_PE_sdata4
#          for personality encoding
# 0xce3: 0x10000 - 0xce3 (local_indirect_ptr - .)
# 0xce7: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for LDSA encoding
# 0xce8: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for FDE encoding
#
  Augmentation data:     9b 00 00 f3 1d 1b 1b

  DW_CFA_def_cfa_register: r29
  DW_CFA_nop

000000ec 00000014 00000020 FDE cie=000000d0 pc=00000810..00000814
#
# 0xcfd: 0x1000c - 0xcfd (LDSA - .)
#
  Augmentation data:     00 00 f3 0f

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

#-------------------------------------------------------------------------
# f6
#-------------------------------------------------------------------------
00000104 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
#
# 0xd16: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for personality encoding
# 0xd17: 0x10004 - 0xd17 (hidden_indirect_ptr - .)
# 0xd1d: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for LDSA encoding
# 0xd1e: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for FDE encoding
#
  Augmentation data:     9b 00 00 f2 ed 1b 1b

  DW_CFA_def_cfa_register: r29
  DW_CFA_nop

00000120 00000014 00000020 FDE cie=00000104 pc=00000814..00000818
#
# 0xd31: 0x1000c - 0xd31 (LDSA - .)
#
  Augmentation data:     00 00 f2 db

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

#-------------------------------------------------------------------------
# f7
#-------------------------------------------------------------------------
00000138 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
#
# 0xd4a: DW_EH_PE_indirect | DW_EH_PE_absptr for personality encoding
# 0xd4b: global_indirect_ptr (reloc above)
# 0xd4f: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for LDSA encoding
# 0xd50: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for FDE encoding
#
  Augmentation data:     80 00 00 00 00 1b 1b

  DW_CFA_def_cfa_register: r29
  DW_CFA_nop

00000154 00000014 00000020 FDE cie=00000138 pc=00000818..0000081c
#
# 0xd65: 0x1000c - 0xd65 (LDSA - .)
#
  Augmentation data:     00 00 f2 a7

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

#-------------------------------------------------------------------------
# f8
#-------------------------------------------------------------------------
0000016c 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 31
#
# 0xd7e: DW_EH_PE_indirect | DW_EH_PE_absptr for personality encoding
# 0xd7f: extern_indirect_ptr (reloc above)
# 0xd83: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for LDSA encoding
# 0xd84: DW_EH_PE_pcrel | DW_EH_PE_sdata4 for FDE encoding
#
  Augmentation data:     80 00 00 00 00 1b 1b

  DW_CFA_def_cfa_register: r29
  DW_CFA_nop

00000188 00000014 00000020 FDE cie=0000016c pc=0000081c..00000820
#
# 0xd99: 0x1000c - 0xd99 (LDSA - .)
#
  Augmentation data:     00 00 f2 73

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

