#objdump: -Wf
#name: CFI on i386
#...
Contents of the .eh_frame section:

00000000 0+0014 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 8
  Augmentation data:     1b

  DW_CFA_def_cfa: r4 \(esp\) ofs 4
  DW_CFA_offset: r8 \(eip\) at cfa-4
  DW_CFA_nop
  DW_CFA_nop

00000018 0+0014 0+001c FDE cie=0+0000 pc=0+0000..0+0012
  DW_CFA_advance_loc: 6 to 0+0006
  DW_CFA_def_cfa_offset: 4664
  DW_CFA_advance_loc: 11 to 0+0011
  DW_CFA_def_cfa_offset: 4

00000030 0+0018 0+0034 FDE cie=0+0000 pc=0+0012..0+001f
  DW_CFA_advance_loc: 1 to 00000013
  DW_CFA_def_cfa_offset: 8
  DW_CFA_offset: r5 \(ebp\) at cfa-8
  DW_CFA_advance_loc: 2 to 00000015
  DW_CFA_def_cfa_register: r5 \(ebp\)
  DW_CFA_advance_loc: 9 to 0000001e
  DW_CFA_def_cfa_register: r4 \(esp\)

0000004c 0+0014 0+0050 FDE cie=0+0000 pc=0+001f..0+002f
  DW_CFA_advance_loc: 2 to 0+0021
  DW_CFA_def_cfa_register: r3 \(ebx\)
  DW_CFA_advance_loc: 13 to 0+002e
  DW_CFA_def_cfa: r4 \(esp\) ofs 4

00000064 0+0010 0+0068 FDE cie=0+000 pc=0+002f..0+0035
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000078 0+0010 0+007c FDE cie=0+0000 pc=0+0035..0+0044
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0000008c 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 8
  Augmentation data:     1b

  DW_CFA_undefined: r8 \(eip\)
  DW_CFA_nop

0+00a0 0+00ac 0+0018 FDE cie=0+008c pc=0+0044..0+0079
  DW_CFA_advance_loc: 1 to 0+0045
  DW_CFA_undefined: r0 \(eax\)
  DW_CFA_advance_loc: 1 to 0+0046
  DW_CFA_undefined: r1 \(ecx\)
  DW_CFA_advance_loc: 1 to 0+0047
  DW_CFA_undefined: r2 \(edx\)
  DW_CFA_advance_loc: 1 to 0+0048
  DW_CFA_undefined: r3 \(ebx\)
  DW_CFA_advance_loc: 1 to 0+0049
  DW_CFA_undefined: r4 \(esp\)
  DW_CFA_advance_loc: 1 to 0+004a
  DW_CFA_undefined: r5 \(ebp\)
  DW_CFA_advance_loc: 1 to 0+004b
  DW_CFA_undefined: r6 \(esi\)
  DW_CFA_advance_loc: 1 to 0+004c
  DW_CFA_undefined: r7 \(edi\)
  DW_CFA_advance_loc: 1 to 0+004d
  DW_CFA_undefined: r9 \(eflags\)
  DW_CFA_advance_loc: 1 to 0+004e
  DW_CFA_undefined: r40 \(es\)
  DW_CFA_advance_loc: 1 to 0+004f
  DW_CFA_undefined: r41 \(cs\)
  DW_CFA_advance_loc: 1 to 0+0050
  DW_CFA_undefined: r43 \(ds\)
  DW_CFA_advance_loc: 1 to 0+0051
  DW_CFA_undefined: r42 \(ss\)
  DW_CFA_advance_loc: 1 to 0+0052
  DW_CFA_undefined: r44 \(fs\)
  DW_CFA_advance_loc: 1 to 0+0053
  DW_CFA_undefined: r45 \(gs\)
  DW_CFA_advance_loc: 1 to 0+0054
  DW_CFA_undefined: r48 \(tr\)
  DW_CFA_advance_loc: 1 to 0+0055
  DW_CFA_undefined: r49 \(ldtr\)
  DW_CFA_advance_loc: 1 to 0+0056
  DW_CFA_undefined: r39 \(mxcsr\)
  DW_CFA_advance_loc: 1 to 0+0057
  DW_CFA_undefined: r21 \(xmm0\)
  DW_CFA_advance_loc: 1 to 0+0058
  DW_CFA_undefined: r22 \(xmm1\)
  DW_CFA_advance_loc: 1 to 0+0059
  DW_CFA_undefined: r23 \(xmm2\)
  DW_CFA_advance_loc: 1 to 0+005a
  DW_CFA_undefined: r24 \(xmm3\)
  DW_CFA_advance_loc: 1 to 0+005b
  DW_CFA_undefined: r25 \(xmm4\)
  DW_CFA_advance_loc: 1 to 0+005c
  DW_CFA_undefined: r26 \(xmm5\)
  DW_CFA_advance_loc: 1 to 0+005d
  DW_CFA_undefined: r27 \(xmm6\)
  DW_CFA_advance_loc: 1 to 0+005e
  DW_CFA_undefined: r28 \(xmm7\)
  DW_CFA_advance_loc: 1 to 0+005f
  DW_CFA_undefined: r37 \(fcw\)
  DW_CFA_advance_loc: 1 to 0+0060
  DW_CFA_undefined: r38 \(fsw\)
  DW_CFA_advance_loc: 1 to 0+0061
  DW_CFA_undefined: r11 \(st\(?0?\)?\)
  DW_CFA_advance_loc: 1 to 0+0062
  DW_CFA_undefined: r12 \(st\(?1\)?\)
  DW_CFA_advance_loc: 1 to 0+0063
  DW_CFA_undefined: r13 \(st\(?2\)?\)
  DW_CFA_advance_loc: 1 to 0+0064
  DW_CFA_undefined: r14 \(st\(?3\)?\)
  DW_CFA_advance_loc: 1 to 0+0065
  DW_CFA_undefined: r15 \(st\(?4\)?\)
  DW_CFA_advance_loc: 1 to 0+0066
  DW_CFA_undefined: r16 \(st\(?5\)?\)
  DW_CFA_advance_loc: 1 to 0+0067
  DW_CFA_undefined: r17 \(st\(?6\)?\)
  DW_CFA_advance_loc: 1 to 0+0068
  DW_CFA_undefined: r18 \(st\(?7\)?\)
  DW_CFA_advance_loc: 1 to 0+0069
  DW_CFA_undefined: r29 \(mm0\)
  DW_CFA_advance_loc: 1 to 0+006a
  DW_CFA_undefined: r30 \(mm1\)
  DW_CFA_advance_loc: 1 to 0+006b
  DW_CFA_undefined: r31 \(mm2\)
  DW_CFA_advance_loc: 1 to 0+006c
  DW_CFA_undefined: r32 \(mm3\)
  DW_CFA_advance_loc: 1 to 0+006d
  DW_CFA_undefined: r33 \(mm4\)
  DW_CFA_advance_loc: 1 to 0+006e
  DW_CFA_undefined: r34 \(mm5\)
  DW_CFA_advance_loc: 1 to 0+006f
  DW_CFA_undefined: r35 \(mm6\)
  DW_CFA_advance_loc: 1 to 0+0070
  DW_CFA_undefined: r36 \(mm7\)
  DW_CFA_advance_loc: 1 to 0+0071
  DW_CFA_undefined: r93 \(k0\)
  DW_CFA_advance_loc: 1 to 0+0072
  DW_CFA_undefined: r94 \(k1\)
  DW_CFA_advance_loc: 1 to 0+0073
  DW_CFA_undefined: r95 \(k2\)
  DW_CFA_advance_loc: 1 to 0+0074
  DW_CFA_undefined: r96 \(k3\)
  DW_CFA_advance_loc: 1 to 0+0075
  DW_CFA_undefined: r97 \(k4\)
  DW_CFA_advance_loc: 1 to 0+0076
  DW_CFA_undefined: r98 \(k5\)
  DW_CFA_advance_loc: 1 to 0+0077
  DW_CFA_undefined: r99 \(k6\)
  DW_CFA_advance_loc: 1 to 0+0078
  DW_CFA_undefined: r100 \(k7\)
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

