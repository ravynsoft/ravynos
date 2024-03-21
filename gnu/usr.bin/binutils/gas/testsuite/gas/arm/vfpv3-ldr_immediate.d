# name: VFPv3 vldr to vmov
# as: -mfpu=vfp3
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section \.text:

0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|3fbe0000) 	.*
0[0-9a-fx]+ .*(3fbe0000|00000000) 	.*
0[0-9a-fx]+ .*3df00000 	.*
.*

0[0-9a-fx]+ .*eebc0b00 	(vmov\.f64|fconstd)	d0, #192.*
0[0-9a-fx]+ .*eebc0a00 	(vmov\.f32|fconsts)	s0, #192.*
0[0-9a-fx]+ .*eeb40b00 	(vmov\.f64|fconstd)	d0, #64.*
0[0-9a-fx]+ .*eeb40a00 	(vmov\.f32|fconsts)	s0, #64.*
0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|3fe08000) 	.*
0[0-9a-fx]+ .*(3fe08000|00000000) 	.*
0[0-9a-fx]+ .*3f040000 	.*
.*
0[0-9a-fx]+ .*eeb60b0f 	(vmov\.f64|fconstd)	d0, #111.*
0[0-9a-fx]+ .*eeb60a0f 	(vmov\.f32|fconsts)	s0, #111.*
0[0-9a-fx]+ .*eeb30b0f 	(vmov\.f64|fconstd)	d0, #63.*
0[0-9a-fx]+ .*eeb30a0f 	(vmov\.f32|fconsts)	s0, #63.*
0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|40400000) 	.*
0[0-9a-fx]+ .*(40400000|00000000) 	.*
0[0-9a-fx]+ .*42000000 	.*
0[0-9a-fx]+ .*ed1fbb01 	vldr	d11, \[pc, #-4\].*
#pass
