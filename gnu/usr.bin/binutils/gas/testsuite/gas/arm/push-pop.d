#objdump: -dr --prefix-addresses --show-raw-insn
#name: PUSH and POP

# Test the `PUSH' and `POP' instructions

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <.*> e52d0004 	push	{r0}		@ \(str r0, \[sp, #-4\]!\)
0+004 <.*> e92d000e 	push	{r1, r2, r3}
0+008 <.*> e52d9004 	push	{r9}		@ \(str r9, \[sp, #-4\]!\)
0+00c <.*> e49d9004 	pop	{r9}		@ \(ldr r9, \[sp\], #4\)
0+010 <.*> e8bd000e 	pop	{r1, r2, r3}
0+014 <.*> e49d0004 	pop	{r0}		@ \(ldr r0, \[sp\], #4\)
