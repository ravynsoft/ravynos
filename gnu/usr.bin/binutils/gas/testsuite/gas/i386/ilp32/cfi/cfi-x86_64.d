#source: ../../../cfi/cfi-x86_64.s
#as: -O0
#readelf: -wf
#name: CFI on x86-64
Contents of the .eh_frame section:

00000000 00000014 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: 16
  Augmentation data:     1b

  DW_CFA_def_cfa: r7 \(rsp\) ofs 8
  DW_CFA_offset: r16 \(rip\) at cfa-8
  DW_CFA_nop
  DW_CFA_nop

00000018 00000014 0000001c FDE cie=00000000 pc=00000000..00000014
  DW_CFA_advance_loc: 7 to 00000007
  DW_CFA_def_cfa_offset: 4668
  DW_CFA_advance_loc: 12 to 00000013
  DW_CFA_def_cfa_offset: 8

00000030 0000001c 00000034 FDE cie=00000000 pc=00000014..00000022
  DW_CFA_advance_loc: 1 to 00000015
  DW_CFA_def_cfa_offset: 16
  DW_CFA_offset: r6 \(rbp\) at cfa-16
  DW_CFA_advance_loc: 3 to 00000018
  DW_CFA_def_cfa_register: r6 \(rbp\)
  DW_CFA_advance_loc: 9 to 00000021
  DW_CFA_def_cfa: r7 \(rsp\) ofs 8
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000050 00000014 00000054 FDE cie=00000000 pc=00000022..00000035
  DW_CFA_advance_loc: 3 to 00000025
  DW_CFA_def_cfa_register: r8 \(r8\)
  DW_CFA_advance_loc: 15 to 00000034
  DW_CFA_def_cfa_register: r7 \(rsp\)
  DW_CFA_nop

00000068 00000010 0000006c FDE cie=00000000 pc=00000035..0000003b
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0000007c 00000010 00000080 FDE cie=00000000 pc=0000003b..0000004d
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000090 00000010 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: 16
  Augmentation data:     1b

  DW_CFA_def_cfa: r7 \(rsp\) ofs 8

000000a4 0000002c 00000018 FDE cie=00000090 pc=0000004d..00000058
  DW_CFA_advance_loc: 1 to 0000004e
  DW_CFA_def_cfa_offset: 16
  DW_CFA_advance_loc: 1 to 0000004f
  DW_CFA_def_cfa_register: r8 \(r8\)
  DW_CFA_advance_loc: 1 to 00000050
  DW_CFA_def_cfa_offset: 4676
  DW_CFA_advance_loc: 1 to 00000051
  DW_CFA_offset_extended_sf: r4 \(rsi\) at cfa\+16
  DW_CFA_advance_loc: 1 to 00000052
  DW_CFA_register: r8 \(r8\) in r9 \(r9\)
  DW_CFA_advance_loc: 1 to 00000053
  DW_CFA_remember_state
  DW_CFA_advance_loc: 1 to 00000054
  DW_CFA_restore: r6 \(rbp\)
  DW_CFA_advance_loc: 1 to 00000055
  DW_CFA_undefined: r16 \(rip\)
  DW_CFA_advance_loc: 1 to 00000056
  DW_CFA_same_value: r3 \(rbx\)
  DW_CFA_advance_loc: 1 to 00000057
  DW_CFA_restore_state
  DW_CFA_nop

000000d4 00000010 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: 16
  Augmentation data:     1b

  DW_CFA_undefined: r16 \(rip\)
  DW_CFA_nop

000000e8 0000011[04] 00000018 FDE cie=000000d4 pc=00000058..000000af
  DW_CFA_advance_loc: 1 to 00000059
  DW_CFA_undefined: r0 \(rax\)
  DW_CFA_advance_loc: 1 to 0000005a
  DW_CFA_undefined: r2 \(rcx\)
  DW_CFA_advance_loc: 1 to 0000005b
  DW_CFA_undefined: r1 \(rdx\)
  DW_CFA_advance_loc: 1 to 0000005c
  DW_CFA_undefined: r3 \(rbx\)
  DW_CFA_advance_loc: 1 to 0000005d
  DW_CFA_undefined: r7 \(rsp\)
  DW_CFA_advance_loc: 1 to 0000005e
  DW_CFA_undefined: r6 \(rbp\)
  DW_CFA_advance_loc: 1 to 0000005f
  DW_CFA_undefined: r4 \(rsi\)
  DW_CFA_advance_loc: 1 to 00000060
  DW_CFA_undefined: r5 \(rdi\)
  DW_CFA_advance_loc: 1 to 00000061
  DW_CFA_undefined: r8 \(r8\)
  DW_CFA_advance_loc: 1 to 00000062
  DW_CFA_undefined: r9 \(r9\)
  DW_CFA_advance_loc: 1 to 00000063
  DW_CFA_undefined: r10 \(r10\)
  DW_CFA_advance_loc: 1 to 00000064
  DW_CFA_undefined: r11 \(r11\)
  DW_CFA_advance_loc: 1 to 00000065
  DW_CFA_undefined: r12 \(r12\)
  DW_CFA_advance_loc: 1 to 00000066
  DW_CFA_undefined: r13 \(r13\)
  DW_CFA_advance_loc: 1 to 00000067
  DW_CFA_undefined: r14 \(r14\)
  DW_CFA_advance_loc: 1 to 00000068
  DW_CFA_undefined: r15 \(r15\)
  DW_CFA_advance_loc: 1 to 00000069
  DW_CFA_undefined: r49 \([er]flags\)
  DW_CFA_advance_loc: 1 to 0000006a
  DW_CFA_undefined: r50 \(es\)
  DW_CFA_advance_loc: 1 to 0000006b
  DW_CFA_undefined: r51 \(cs\)
  DW_CFA_advance_loc: 1 to 0000006c
  DW_CFA_undefined: r53 \(ds\)
  DW_CFA_advance_loc: 1 to 0000006d
  DW_CFA_undefined: r52 \(ss\)
  DW_CFA_advance_loc: 1 to 0000006e
  DW_CFA_undefined: r54 \(fs\)
  DW_CFA_advance_loc: 1 to 0000006f
  DW_CFA_undefined: r55 \(gs\)
  DW_CFA_advance_loc: 1 to 00000070
  DW_CFA_undefined: r62 \(tr\)
  DW_CFA_advance_loc: 1 to 00000071
  DW_CFA_undefined: r63 \(ldtr\)
  DW_CFA_advance_loc: 1 to 00000072
  DW_CFA_undefined: r58 \(fs\.base\)
  DW_CFA_advance_loc: 1 to 00000073
  DW_CFA_undefined: r59 \(gs\.base\)
  DW_CFA_advance_loc: 1 to 00000074
  DW_CFA_undefined: r64 \(mxcsr\)
  DW_CFA_advance_loc: 1 to 00000075
  DW_CFA_undefined: r17 \(xmm0\)
  DW_CFA_advance_loc: 1 to 00000076
  DW_CFA_undefined: r18 \(xmm1\)
  DW_CFA_advance_loc: 1 to 00000077
  DW_CFA_undefined: r19 \(xmm2\)
  DW_CFA_advance_loc: 1 to 00000078
  DW_CFA_undefined: r20 \(xmm3\)
  DW_CFA_advance_loc: 1 to 00000079
  DW_CFA_undefined: r21 \(xmm4\)
  DW_CFA_advance_loc: 1 to 0000007a
  DW_CFA_undefined: r22 \(xmm5\)
  DW_CFA_advance_loc: 1 to 0000007b
  DW_CFA_undefined: r23 \(xmm6\)
  DW_CFA_advance_loc: 1 to 0000007c
  DW_CFA_undefined: r24 \(xmm7\)
  DW_CFA_advance_loc: 1 to 0000007d
  DW_CFA_undefined: r25 \(xmm8\)
  DW_CFA_advance_loc: 1 to 0000007e
  DW_CFA_undefined: r26 \(xmm9\)
  DW_CFA_advance_loc: 1 to 0000007f
  DW_CFA_undefined: r27 \(xmm10\)
  DW_CFA_advance_loc: 1 to 00000080
  DW_CFA_undefined: r28 \(xmm11\)
  DW_CFA_advance_loc: 1 to 00000081
  DW_CFA_undefined: r29 \(xmm12\)
  DW_CFA_advance_loc: 1 to 00000082
  DW_CFA_undefined: r30 \(xmm13\)
  DW_CFA_advance_loc: 1 to 00000083
  DW_CFA_undefined: r31 \(xmm14\)
  DW_CFA_advance_loc: 1 to 00000084
  DW_CFA_undefined: r32 \(xmm15\)
  DW_CFA_advance_loc: 1 to 00000085
  DW_CFA_undefined: r65 \(fcw\)
  DW_CFA_advance_loc: 1 to 00000086
  DW_CFA_undefined: r66 \(fsw\)
  DW_CFA_advance_loc: 1 to 00000087
  DW_CFA_undefined: r33 \(st\(?0?\)?\)
  DW_CFA_advance_loc: 1 to 00000088
  DW_CFA_undefined: r34 \(st\(?1\)?\)
  DW_CFA_advance_loc: 1 to 00000089
  DW_CFA_undefined: r35 \(st\(?2\)?\)
  DW_CFA_advance_loc: 1 to 0000008a
  DW_CFA_undefined: r36 \(st\(?3\)?\)
  DW_CFA_advance_loc: 1 to 0000008b
  DW_CFA_undefined: r37 \(st\(?4\)?\)
  DW_CFA_advance_loc: 1 to 0000008c
  DW_CFA_undefined: r38 \(st\(?5\)?\)
  DW_CFA_advance_loc: 1 to 0000008d
  DW_CFA_undefined: r39 \(st\(?6\)?\)
  DW_CFA_advance_loc: 1 to 0000008e
  DW_CFA_undefined: r40 \(st\(?7\)?\)
  DW_CFA_advance_loc: 1 to 0000008f
  DW_CFA_undefined: r41 \(mm0\)
  DW_CFA_advance_loc: 1 to 00000090
  DW_CFA_undefined: r42 \(mm1\)
  DW_CFA_advance_loc: 1 to 00000091
  DW_CFA_undefined: r43 \(mm2\)
  DW_CFA_advance_loc: 1 to 00000092
  DW_CFA_undefined: r44 \(mm3\)
  DW_CFA_advance_loc: 1 to 00000093
  DW_CFA_undefined: r45 \(mm4\)
  DW_CFA_advance_loc: 1 to 00000094
  DW_CFA_undefined: r46 \(mm5\)
  DW_CFA_advance_loc: 1 to 00000095
  DW_CFA_undefined: r47 \(mm6\)
  DW_CFA_advance_loc: 1 to 00000096
  DW_CFA_undefined: r48 \(mm7\)
  DW_CFA_advance_loc: 1 to 00000097
  DW_CFA_undefined: r67 \(xmm16\)
  DW_CFA_advance_loc: 1 to 00000098
  DW_CFA_undefined: r68 \(xmm17\)
  DW_CFA_advance_loc: 1 to 00000099
  DW_CFA_undefined: r69 \(xmm18\)
  DW_CFA_advance_loc: 1 to 0000009a
  DW_CFA_undefined: r70 \(xmm19\)
  DW_CFA_advance_loc: 1 to 0000009b
  DW_CFA_undefined: r71 \(xmm20\)
  DW_CFA_advance_loc: 1 to 0000009c
  DW_CFA_undefined: r72 \(xmm21\)
  DW_CFA_advance_loc: 1 to 0000009d
  DW_CFA_undefined: r73 \(xmm22\)
  DW_CFA_advance_loc: 1 to 0000009e
  DW_CFA_undefined: r74 \(xmm23\)
  DW_CFA_advance_loc: 1 to 0000009f
  DW_CFA_undefined: r75 \(xmm24\)
  DW_CFA_advance_loc: 1 to 000000a0
  DW_CFA_undefined: r76 \(xmm25\)
  DW_CFA_advance_loc: 1 to 000000a1
  DW_CFA_undefined: r77 \(xmm26\)
  DW_CFA_advance_loc: 1 to 000000a2
  DW_CFA_undefined: r78 \(xmm27\)
  DW_CFA_advance_loc: 1 to 000000a3
  DW_CFA_undefined: r79 \(xmm28\)
  DW_CFA_advance_loc: 1 to 000000a4
  DW_CFA_undefined: r80 \(xmm29\)
  DW_CFA_advance_loc: 1 to 000000a5
  DW_CFA_undefined: r81 \(xmm30\)
  DW_CFA_advance_loc: 1 to 000000a6
  DW_CFA_undefined: r82 \(xmm31\)
  DW_CFA_advance_loc: 1 to 000000a7
  DW_CFA_undefined: r118 \(k0\)
  DW_CFA_advance_loc: 1 to 000000a8
  DW_CFA_undefined: r119 \(k1\)
  DW_CFA_advance_loc: 1 to 000000a9
  DW_CFA_undefined: r120 \(k2\)
  DW_CFA_advance_loc: 1 to 000000aa
  DW_CFA_undefined: r121 \(k3\)
  DW_CFA_advance_loc: 1 to 000000ab
  DW_CFA_undefined: r122 \(k4\)
  DW_CFA_advance_loc: 1 to 000000ac
  DW_CFA_undefined: r123 \(k5\)
  DW_CFA_advance_loc: 1 to 000000ad
  DW_CFA_undefined: r124 \(k6\)
  DW_CFA_advance_loc: 1 to 000000ae
  DW_CFA_undefined: r125 \(k7\)
  DW_CFA_nop

