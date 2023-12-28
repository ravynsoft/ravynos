#name: PR ld/20830 (.plt.got)
#as: --32
#ld: -melf_i386 -shared -z relro --ld-generated-unwind-info --hash-style=sysv -z noseparate-code $NO_DT_RELR_LDFLAGS
#objdump: -dw -Wf

.*: +file format .*

Contents of the .eh_frame section:

0+ 00000014 00000000 CIE
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

0+18 00000010 0000001c FDE cie=00000000 pc=00000128..00000133
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+2c 00000020 00000030 FDE cie=00000000 pc=00000110..00000120
  DW_CFA_def_cfa_offset: 8
  DW_CFA_advance_loc: 6 to 00000116
  DW_CFA_def_cfa_offset: 12
  DW_CFA_advance_loc: 10 to 00000120
  DW_CFA_def_cfa_expression \(DW_OP_breg4 \(esp\): 4; DW_OP_breg8 \(eip\): 0; DW_OP_lit15; DW_OP_and; DW_OP_lit11; DW_OP_ge; DW_OP_lit2; DW_OP_shl; DW_OP_plus\)

0+50 00000010 00000054 FDE cie=00000000 pc=00000120..00000128
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop


Disassembly of section .plt:

0+110 <.plt>:
 +[a-f0-9]+:	ff b3 04 00 00 00    	push   0x4\(%ebx\)
 +[a-f0-9]+:	ff a3 08 00 00 00    	jmp    \*0x8\(%ebx\)
 +[a-f0-9]+:	00 00                	add    %al,\(%eax\)
	...

Disassembly of section .plt.got:

0+120 <func@plt>:
 +[a-f0-9]+:	ff a3 fc ff ff ff    	jmp    \*-0x4\(%ebx\)
 +[a-f0-9]+:	66 90                	xchg   %ax,%ax

Disassembly of section .text:

0+128 <foo>:
 +[a-f0-9]+:	e8 f3 ff ff ff       	call   120 <func@plt>
 +[a-f0-9]+:	8b 83 fc ff ff ff    	mov    -0x4\(%ebx\),%eax
#pass
