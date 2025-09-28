#PROG: objcopy
#readelf: --debug-dump=frames

Contents of the .eh_frame section:


00000000 0000000000000018 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 4
  Data alignment factor: -8
  Return address column: 30
  Augmentation data:     1b
  DW_CFA_def_cfa: r31 \(sp\) ofs 0
  DW_CFA_def_cfa_register: r96 \(z0\)
  DW_CFA_def_cfa_offset: 5
  DW_CFA_restore_extended: r96 \(z0\)
  DW_CFA_nop
  DW_CFA_nop

0000001c 0000000000000010 00000020 FDE cie=00000000 pc=0000000000000000..0000000000000000
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

