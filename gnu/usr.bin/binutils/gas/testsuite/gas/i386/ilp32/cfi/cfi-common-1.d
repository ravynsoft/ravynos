#source: ../../../cfi/cfi-common-1.s
#readelf: -wf
#name: CFI common 1
Contents of the .eh_frame section:

00000000 00000010 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01]b

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000014 000000(18|1c|20) 00000018 FDE cie=00000000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0( \([er]ax\)|) ofs 16
  DW_CFA_offset(_extended_sf|): r1( \((rdx|ecx)\)|) at cfa-8
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa_offset: 32
  DW_CFA_offset(_extended_sf|): r2( \((rcx|edx)\)|) at cfa-24
#...
