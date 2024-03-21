#as:
#objdump: -dr
#name: reloc

.*: +file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	41 80 80 80 		i32.const 0
   4:	80 00 
			1: R_WASM32_PLT_SIG	__sigchar_FiiiiiiiE
			1: R_WASM32_LEB128_PLT	f
   6:	41 80 80 80 		i32.const 0
   a:	80 00 
			7: R_WASM32_LEB128_GOT	x
   c:	41 80 80 80 		i32.const 0
  10:	80 00 
			d: R_WASM32_LEB128_GOT_CODE	f
