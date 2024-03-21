#as: -mx86-used-note=no --generate-missing-build-notes=no
#objdump: -tWf
#name: .cfi_label directive

.*\.o:     file format elf.*

SYMBOL TABLE:
0*00 l    d  \.text	0*00 \.text
#...
0*00 l     F \.text	0*04 cfilabel
0*2f l       \.eh_frame	0*00 cfi2
0*2b g       \.eh_frame	0*00 cfi1


Contents of the .eh_frame section:

0*00 0*14 0*00 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -[48]
  Return address column: (8|16)
  Augmentation data:     1b

  DW_CFA_def_cfa: r.* \([er]sp\) ofs [48]
  DW_CFA_offset: r.* \([er]ip\) at cfa-[48]
  DW_CFA_nop
  DW_CFA_nop

0*18 0*1[8c] 0*1c FDE cie=0*00 pc=0*00..0*04
  DW_CFA_remember_state
  DW_CFA_advance_loc: 1 to 0*01
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_advance_loc: 1 to 0*02
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_advance_loc: 1 to 0*03
  DW_CFA_nop
  DW_CFA_restore_state
#pass
