# name: VFPv2 vldr to vmov
# as: -mfpu=vfpv2
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section \.text:

0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|3fbe0000) 	.*
0[0-9a-fx]+ .*(3fbe0000|00000000) 	.*
0[0-9a-fx]+ .*3df00000 	.*
.*
0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|bfc00000) 	.*
0[0-9a-fx]+ .*(bfc00000|00000000) 	.*
0[0-9a-fx]+ .*be000000 	.*
.*
0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|3fc00000) 	.*
0[0-9a-fx]+ .*(3fc00000|00000000) 	.*
0[0-9a-fx]+ .*3e000000 	.*
.*
0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|3fe08000) 	.*
0[0-9a-fx]+ .*(3fe08000|00000000) 	.*
0[0-9a-fx]+ .*3f040000 	.*
.*
0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|3fef0000) 	.*
0[0-9a-fx]+ .*(3fef0000|00000000) 	.*
0[0-9a-fx]+ .*3f780000 	.*
.*
0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|403f0000) 	.*
0[0-9a-fx]+ .*(403f0000|00000000) 	.*
0[0-9a-fx]+ .*41f80000 	.*
.*
0[0-9a-fx]+ .*ed9f0b00 	vldr	d0, \[pc\].*
0[0-9a-fx]+ .*ed9f0a01 	vldr	s0, \[pc, #4\].*
0[0-9a-fx]+ .*(00000000|40400000) 	.*
0[0-9a-fx]+ .*(40400000|00000000) 	.*
0[0-9a-fx]+ .*42000000 	.*
#pass
