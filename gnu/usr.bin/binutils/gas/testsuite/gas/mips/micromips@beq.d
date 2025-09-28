#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS beq
#source: beq.s
#as: -32

# Test the beq macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 94a4 fffe 	beq	a0,a1,0+0000 <text_label>
			0: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 40e4 fffe 	beqzc	a0,0+0006 <text_label\+0x6>
			6: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 3020 0001 	li	at,1
[0-9a-f]+ <[^>]*> 9424 fffe 	beq	a0,at,0+000e <text_label\+0xe>
			e: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 5020 8000 	li	at,0x8000
[0-9a-f]+ <[^>]*> 9424 fffe 	beq	a0,at,0+0018 <text_label\+0x18>
			18: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 3020 8000 	li	at,-32768
[0-9a-f]+ <[^>]*> 9424 fffe 	beq	a0,at,0+0022 <text_label\+0x22>
			22: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 9424 fffe 	beq	a0,at,0+002c <text_label\+0x2c>
			2c: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 5021 a5a5 	ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> 9424 fffe 	beq	a0,at,0+003a <text_label\+0x3a>
			3a: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 40a4 fffe 	bnezc	a0,0+0040 <text_label\+0x40>
			40: R_MICROMIPS_PC16_S1	text_label
	\.\.\.
[0-9a-f]+ <[^>]*> 40e0 fffe 	bc	00020044 <text_label\+0x20044>
			20044: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	00020048 <text_label\+0x20048>
			20048: R_MICROMIPS_PC16_S1	text_label
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 40e0 fffe 	bc	00020050 <text_label\+0x20050>
			20050: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	00020054 <text_label\+0x20054>
			20054: R_MICROMIPS_PC16_S1	external_label
[0-9a-f]+ <[^>]*> 0000 0000 	nop
	\.\.\.
