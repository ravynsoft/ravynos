#as: -O0
#objdump: -Wf
#name: CFI on x86-64
#...
Contents of the .eh_frame section:

0+0000 0+0014 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: (16|32)
  Augmentation data:     1b

  DW_CFA_def_cfa: r7 \(rsp\) ofs 8
  DW_CFA_offset: (r16 \(rip\)|r32 \(xmm15\)) at cfa-8
  DW_CFA_nop
  DW_CFA_nop

0+0018 0+0014 0+001c FDE cie=0+0000 pc=0+0000..0+0014
  DW_CFA_advance_loc: 7 to 0+0007
  DW_CFA_def_cfa_offset: 4668
  DW_CFA_advance_loc: 12 to 0+0013
  DW_CFA_def_cfa_offset: 8

0+0030 0+001c 0+0034 FDE cie=0+0000 pc=0+0014..0+0022
  DW_CFA_advance_loc: 1 to 0+0015
  DW_CFA_def_cfa_offset: 16
  DW_CFA_offset: r6 \(rbp\) at cfa-16
  DW_CFA_advance_loc: 3 to 0+0018
  DW_CFA_def_cfa_register: r6 \(rbp\)
  DW_CFA_advance_loc: 9 to 0+0021
  DW_CFA_def_cfa: r7 \(rsp\) ofs 8
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0050 0+0014 0+0054 FDE cie=0+0000 pc=0+0022..0+0035
  DW_CFA_advance_loc: 3 to 0+0025
  DW_CFA_def_cfa_register: r8 \(r8\)
  DW_CFA_advance_loc: 15 to 0+0034
  DW_CFA_def_cfa_register: r7 \(rsp\)
  DW_CFA_nop

0+0068 0+0010 0+006c FDE cie=0+0000 pc=0+0035..0+003b
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+007c 0+0010 0+0080 FDE cie=0+0000 pc=0+003b..0+004d
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0090 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: (16|32)
  Augmentation data:     1b

  DW_CFA_def_cfa: r7 \(rsp\) ofs 8

0+00a4 0+002c 0+0018 FDE cie=0+0090 pc=0+004d..0+0058
  DW_CFA_advance_loc: 1 to 0+004e
  DW_CFA_def_cfa_offset: 16
  DW_CFA_advance_loc: 1 to 0+004f
  DW_CFA_def_cfa_register: r8 \(r8\)
  DW_CFA_advance_loc: 1 to 0+0050
  DW_CFA_def_cfa_offset: 4676
  DW_CFA_advance_loc: 1 to 0+0051
  DW_CFA_offset_extended_sf: r4 \(rsi\) at cfa\+16
  DW_CFA_advance_loc: 1 to 0+0052
  DW_CFA_register: r8 \(r8\) in r9 \(r9\)
  DW_CFA_advance_loc: 1 to 0+0053
  DW_CFA_remember_state
  DW_CFA_advance_loc: 1 to 0+0054
  DW_CFA_restore: r6 \(rbp\)
  DW_CFA_advance_loc: 1 to 0+0055
  DW_CFA_undefined: r16 \(rip\)
  DW_CFA_advance_loc: 1 to 0+0056
  DW_CFA_same_value: r3 \(rbx\)
  DW_CFA_advance_loc: 1 to 0+0057
  DW_CFA_restore_state
  DW_CFA_nop

0+00d4 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: (16|32)
  Augmentation data:     1b

  DW_CFA_undefined: r16 \(rip\)
  DW_CFA_nop

0+00e8 0+011[04] 0+0018 FDE cie=0+00d4 pc=0+0058..0+00af
  DW_CFA_advance_loc: 1 to 0+0059
  DW_CFA_undefined: r0 \(rax\)
  DW_CFA_advance_loc: 1 to 0+005a
  DW_CFA_undefined: r2 \(rcx\)
  DW_CFA_advance_loc: 1 to 0+005b
  DW_CFA_undefined: r1 \(rdx\)
  DW_CFA_advance_loc: 1 to 0+005c
  DW_CFA_undefined: r3 \(rbx\)
  DW_CFA_advance_loc: 1 to 0+005d
  DW_CFA_undefined: r7 \(rsp\)
  DW_CFA_advance_loc: 1 to 0+005e
  DW_CFA_undefined: r6 \(rbp\)
  DW_CFA_advance_loc: 1 to 0+005f
  DW_CFA_undefined: r4 \(rsi\)
  DW_CFA_advance_loc: 1 to 0+0060
  DW_CFA_undefined: r5 \(rdi\)
  DW_CFA_advance_loc: 1 to 0+0061
  DW_CFA_undefined: r8 \(r8\)
  DW_CFA_advance_loc: 1 to 0+0062
  DW_CFA_undefined: r9 \(r9\)
  DW_CFA_advance_loc: 1 to 0+0063
  DW_CFA_undefined: r10 \(r10\)
  DW_CFA_advance_loc: 1 to 0+0064
  DW_CFA_undefined: r11 \(r11\)
  DW_CFA_advance_loc: 1 to 0+0065
  DW_CFA_undefined: r12 \(r12\)
  DW_CFA_advance_loc: 1 to 0+0066
  DW_CFA_undefined: r13 \(r13\)
  DW_CFA_advance_loc: 1 to 0+0067
  DW_CFA_undefined: r14 \(r14\)
  DW_CFA_advance_loc: 1 to 0+0068
  DW_CFA_undefined: r15 \(r15\)
  DW_CFA_advance_loc: 1 to 0+0069
  DW_CFA_undefined: r49 \([er]flags\)
  DW_CFA_advance_loc: 1 to 0+006a
  DW_CFA_undefined: r50 \(es\)
  DW_CFA_advance_loc: 1 to 0+006b
  DW_CFA_undefined: r51 \(cs\)
  DW_CFA_advance_loc: 1 to 0+006c
  DW_CFA_undefined: r53 \(ds\)
  DW_CFA_advance_loc: 1 to 0+006d
  DW_CFA_undefined: r52 \(ss\)
  DW_CFA_advance_loc: 1 to 0+006e
  DW_CFA_undefined: r54 \(fs\)
  DW_CFA_advance_loc: 1 to 0+006f
  DW_CFA_undefined: r55 \(gs\)
  DW_CFA_advance_loc: 1 to 0+0070
  DW_CFA_undefined: r62 \(tr\)
  DW_CFA_advance_loc: 1 to 0+0071
  DW_CFA_undefined: r63 \(ldtr\)
  DW_CFA_advance_loc: 1 to 0+0072
  DW_CFA_undefined: r58 \(fs\.base\)
  DW_CFA_advance_loc: 1 to 0+0073
  DW_CFA_undefined: r59 \(gs\.base\)
  DW_CFA_advance_loc: 1 to 0+0074
  DW_CFA_undefined: r64 \(mxcsr\)
  DW_CFA_advance_loc: 1 to 0+0075
  DW_CFA_undefined: r17 \(xmm0\)
  DW_CFA_advance_loc: 1 to 0+0076
  DW_CFA_undefined: r18 \(xmm1\)
  DW_CFA_advance_loc: 1 to 0+0077
  DW_CFA_undefined: r19 \(xmm2\)
  DW_CFA_advance_loc: 1 to 0+0078
  DW_CFA_undefined: r20 \(xmm3\)
  DW_CFA_advance_loc: 1 to 0+0079
  DW_CFA_undefined: r21 \(xmm4\)
  DW_CFA_advance_loc: 1 to 0+007a
  DW_CFA_undefined: r22 \(xmm5\)
  DW_CFA_advance_loc: 1 to 0+007b
  DW_CFA_undefined: r23 \(xmm6\)
  DW_CFA_advance_loc: 1 to 0+007c
  DW_CFA_undefined: r24 \(xmm7\)
  DW_CFA_advance_loc: 1 to 0+007d
  DW_CFA_undefined: r25 \(xmm8\)
  DW_CFA_advance_loc: 1 to 0+007e
  DW_CFA_undefined: r26 \(xmm9\)
  DW_CFA_advance_loc: 1 to 0+007f
  DW_CFA_undefined: r27 \(xmm10\)
  DW_CFA_advance_loc: 1 to 0+0080
  DW_CFA_undefined: r28 \(xmm11\)
  DW_CFA_advance_loc: 1 to 0+0081
  DW_CFA_undefined: r29 \(xmm12\)
  DW_CFA_advance_loc: 1 to 0+0082
  DW_CFA_undefined: r30 \(xmm13\)
  DW_CFA_advance_loc: 1 to 0+0083
  DW_CFA_undefined: r31 \(xmm14\)
  DW_CFA_advance_loc: 1 to 0+0084
  DW_CFA_undefined: r32 \(xmm15\)
  DW_CFA_advance_loc: 1 to 0+0085
  DW_CFA_undefined: r65 \(fcw\)
  DW_CFA_advance_loc: 1 to 0+0086
  DW_CFA_undefined: r66 \(fsw\)
  DW_CFA_advance_loc: 1 to 0+0087
  DW_CFA_undefined: r33 \(st\(?0?\)?\)
  DW_CFA_advance_loc: 1 to 0+0088
  DW_CFA_undefined: r34 \(st\(?1\)?\)
  DW_CFA_advance_loc: 1 to 0+0089
  DW_CFA_undefined: r35 \(st\(?2\)?\)
  DW_CFA_advance_loc: 1 to 0+008a
  DW_CFA_undefined: r36 \(st\(?3\)?\)
  DW_CFA_advance_loc: 1 to 0+008b
  DW_CFA_undefined: r37 \(st\(?4\)?\)
  DW_CFA_advance_loc: 1 to 0+008c
  DW_CFA_undefined: r38 \(st\(?5\)?\)
  DW_CFA_advance_loc: 1 to 0+008d
  DW_CFA_undefined: r39 \(st\(?6\)?\)
  DW_CFA_advance_loc: 1 to 0+008e
  DW_CFA_undefined: r40 \(st\(?7\)?\)
  DW_CFA_advance_loc: 1 to 0+008f
  DW_CFA_undefined: r41 \(mm0\)
  DW_CFA_advance_loc: 1 to 0+0090
  DW_CFA_undefined: r42 \(mm1\)
  DW_CFA_advance_loc: 1 to 0+0091
  DW_CFA_undefined: r43 \(mm2\)
  DW_CFA_advance_loc: 1 to 0+0092
  DW_CFA_undefined: r44 \(mm3\)
  DW_CFA_advance_loc: 1 to 0+0093
  DW_CFA_undefined: r45 \(mm4\)
  DW_CFA_advance_loc: 1 to 0+0094
  DW_CFA_undefined: r46 \(mm5\)
  DW_CFA_advance_loc: 1 to 0+0095
  DW_CFA_undefined: r47 \(mm6\)
  DW_CFA_advance_loc: 1 to 0+0096
  DW_CFA_undefined: r48 \(mm7\)
  DW_CFA_advance_loc: 1 to 0+0097
  DW_CFA_undefined: r67 \(xmm16\)
  DW_CFA_advance_loc: 1 to 0+0098
  DW_CFA_undefined: r68 \(xmm17\)
  DW_CFA_advance_loc: 1 to 0+0099
  DW_CFA_undefined: r69 \(xmm18\)
  DW_CFA_advance_loc: 1 to 0+009a
  DW_CFA_undefined: r70 \(xmm19\)
  DW_CFA_advance_loc: 1 to 0+009b
  DW_CFA_undefined: r71 \(xmm20\)
  DW_CFA_advance_loc: 1 to 0+009c
  DW_CFA_undefined: r72 \(xmm21\)
  DW_CFA_advance_loc: 1 to 0+009d
  DW_CFA_undefined: r73 \(xmm22\)
  DW_CFA_advance_loc: 1 to 0+009e
  DW_CFA_undefined: r74 \(xmm23\)
  DW_CFA_advance_loc: 1 to 0+009f
  DW_CFA_undefined: r75 \(xmm24\)
  DW_CFA_advance_loc: 1 to 0+00a0
  DW_CFA_undefined: r76 \(xmm25\)
  DW_CFA_advance_loc: 1 to 0+00a1
  DW_CFA_undefined: r77 \(xmm26\)
  DW_CFA_advance_loc: 1 to 0+00a2
  DW_CFA_undefined: r78 \(xmm27\)
  DW_CFA_advance_loc: 1 to 0+00a3
  DW_CFA_undefined: r79 \(xmm28\)
  DW_CFA_advance_loc: 1 to 0+00a4
  DW_CFA_undefined: r80 \(xmm29\)
  DW_CFA_advance_loc: 1 to 0+00a5
  DW_CFA_undefined: r81 \(xmm30\)
  DW_CFA_advance_loc: 1 to 0+00a6
  DW_CFA_undefined: r82 \(xmm31\)
  DW_CFA_advance_loc: 1 to 0+00a7
  DW_CFA_undefined: r118 \(k0\)
  DW_CFA_advance_loc: 1 to 0+00a8
  DW_CFA_undefined: r119 \(k1\)
  DW_CFA_advance_loc: 1 to 0+00a9
  DW_CFA_undefined: r120 \(k2\)
  DW_CFA_advance_loc: 1 to 0+00aa
  DW_CFA_undefined: r121 \(k3\)
  DW_CFA_advance_loc: 1 to 0+00ab
  DW_CFA_undefined: r122 \(k4\)
  DW_CFA_advance_loc: 1 to 0+00ac
  DW_CFA_undefined: r123 \(k5\)
  DW_CFA_advance_loc: 1 to 0+00ad
  DW_CFA_undefined: r124 \(k6\)
  DW_CFA_advance_loc: 1 to 0+00ae
  DW_CFA_undefined: r125 \(k7\)
  DW_CFA_nop
#pass
