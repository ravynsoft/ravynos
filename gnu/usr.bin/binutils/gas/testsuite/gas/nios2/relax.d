#source: relax.s
#as: --gdwarf-5
#objdump: -w -WL -dr --prefix-addresses --show-raw-insn
#name: NIOS2 relax with --gdwarf-5

# Test relaxation with assembler generated debug info.
.*:     file format elf32-littlenios2

Contents of the .debug_line section:

CU: .*relax.s:
File +name +Line +number +Starting +address +View +Stmt
relax.s +2 +0 +x
relax.s +5 +0x10018 +x
relax.s +- +0x1001c

Disassembly of section .text:
0x00000000 21400526 	beq	r4,r5,0x00000018
0x00000004 0002e03a 	nextpc	at
0x00000008 085fffc4 	addi	at,at,32767
0x0000000c 085fffc4 	addi	at,at,32767
0x00000010 08400484 	addi	at,at,18
0x00000014 0800683a 	jmp	at
	...
0x00010018 f800283a 	ret
