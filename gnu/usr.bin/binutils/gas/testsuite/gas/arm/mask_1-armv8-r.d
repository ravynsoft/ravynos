#objdump: -dr --prefix-address --show-raw-insn
#name: ARMv8-R vsel, vmaxnm, vminnm, vrint decoding mask.
#source: mask_1-armv8-ar.s
#as: -march=armv8-r
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

# Test VFMA instruction disassembly

.*: *file format .*arm.*


Disassembly of section .text:
0+000 <.*> fe011a10 	mcr2	10, 0, r1, cr1, cr0, \{0\}	@ <UNPREDICTABLE>
0+004 <.*> fe011b10 	mcr2	11, 0, r1, cr1, cr0, \{0\}	@ <UNPREDICTABLE>
0+008 <.*> fe811a10 	mcr2	10, 4, r1, cr1, cr0, \{0\}	@ <UNPREDICTABLE>
0+00c <.*> fe811b10 	mcr2	11, 4, r1, cr1, cr0, \{0\}	@ <UNPREDICTABLE>
0+010 <.*> fe811a50 	mcr2	10, 4, r1, cr1, cr0, \{2\}	@ <UNPREDICTABLE>
0+014 <.*> fe811b50 	mcr2	11, 4, r1, cr1, cr0, \{2\}	@ <UNPREDICTABLE>
0+018 <.*> fefb0ae0 			@ <UNDEFINED> instruction: 0xfefb0ae0
0+01c <.*> fefb0be0 			@ <UNDEFINED> instruction: 0xfefb0be0
0+020 <.*> fefb0ae0 			@ <UNDEFINED> instruction: 0xfefb0ae0
0+024 <.*> fefb0be0 			@ <UNDEFINED> instruction: 0xfefb0be0
0+028 <.*> fef80ae0 			@ <UNDEFINED> instruction: 0xfef80ae0
0+02c <.*> fef80be0 			@ <UNDEFINED> instruction: 0xfef80be0
0+030 <.*> fef90ae0 			@ <UNDEFINED> instruction: 0xfef90ae0
0+034 <.*> fef90be0 			@ <UNDEFINED> instruction: 0xfef90be0
0+038 <.*> fefa0ae0 			@ <UNDEFINED> instruction: 0xfefa0ae0
0+03c <.*> fefa0be0 			@ <UNDEFINED> instruction: 0xfefa0be0
