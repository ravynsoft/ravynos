#as:
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+00000517[ 	]+auipc[ 	]+a0,0x0
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_PCREL_HI20[ 	]+a
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_RELAX[ 	]+\*ABS\*
[ 	]+[0-9a-f]+:[ 	]+00050513[ 	]+mv[ 	]+a0,a0
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_PCREL_LO12_I[ 	]+\.L0[ ]+
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_RELAX[ 	]+\*ABS\*
[ 	]+[0-9a-f]+:[ 	]+00000597[ 	]+auipc[ 	]+a1,0x0
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_PCREL_HI20[ 	]+a
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_RELAX[ 	]+\*ABS\*
[ 	]+[0-9a-f]+:[ 	]+00058593[ 	]+mv[ 	]+a1,a1
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_PCREL_LO12_I[ 	]+\.L0[ ]+
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_RELAX[ 	]+\*ABS\*
[ 	]+[0-9a-f]+:[ 	]+00000617[ 	]+auipc[ 	]+a2,0x0
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_GOT_HI20[ 	]+a
[ 	]+[0-9a-f]+:[ 	]+(00062603|00063603)[ 	]+(lw|ld)[ 	]+a2,0\(a2\).*
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_PCREL_LO12_I[ 	]+\.L0[ ]+
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_RELAX[ 	]+\*ABS\*
[ 	]+[0-9a-f]+:[ 	]+00000697[ 	]+auipc[ 	]+a3,0x0
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_GOT_HI20[ 	]+a
[ 	]+[0-9a-f]+:[ 	]+(0006a683|0006b683)[ 	]+(lw|ld)[ 	]+a3,0\(a3\).*
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_PCREL_LO12_I[ 	]+\.L0[ ]+
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_RELAX[ 	]+\*ABS\*
[ 	]+[0-9a-f]+:[ 	]+00000717[ 	]+auipc[ 	]+a4,0x0
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_PCREL_HI20[ 	]+a
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_RELAX[ 	]+\*ABS\*
[ 	]+[0-9a-f]+:[ 	]+00070713[ 	]+mv[ 	]+a4,a4
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_PCREL_LO12_I[ 	]+\.L0[ ]+
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_RELAX[ 	]+\*ABS\*
[ 	]+[0-9a-f]+:[ 	]+00000797[ 	]+auipc[ 	]+a5,0x0
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_GOT_HI20[ 	]+a
[ 	]+[0-9a-f]+:[ 	]+(0007a783|0007b783)[ 	]+(lw|ld)[ 	]+a5,0\(a5\).*
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_PCREL_LO12_I[ 	]+\.L0[ ]+
[ 	]+[0-9a-f]+:[ 	]+R_RISCV_RELAX[ 	]+\*ABS\*
