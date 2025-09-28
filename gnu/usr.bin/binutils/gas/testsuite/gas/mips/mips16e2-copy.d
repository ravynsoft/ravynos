#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16e2 interAptiv MR2 COPYW/UCOPYW ASMACRO instructions
#as: -32 -mips16 -march=interaptiv-mr2

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f020 e260 	copyw	v0,v1,0,1
[0-9a-f]+ <[^>]*> f021 e381 	copyw	v1,a0,16,2
[0-9a-f]+ <[^>]*> f022 e4a2 	copyw	a0,a1,32,3
[0-9a-f]+ <[^>]*> f024 e5c3 	copyw	a1,a2,64,4
[0-9a-f]+ <[^>]*> f028 e6e0 	copyw	a2,a3,128,1
[0-9a-f]+ <[^>]*> f030 e701 	copyw	a3,s0,256,2
[0-9a-f]+ <[^>]*> f038 e022 	copyw	s0,s1,384,3
[0-9a-f]+ <[^>]*> f03c e143 	copyw	s1,v0,448,4
[0-9a-f]+ <[^>]*> f03e e260 	copyw	v0,v1,480,1
[0-9a-f]+ <[^>]*> f03f e381 	copyw	v1,a0,496,2
[0-9a-f]+ <[^>]*> f02a e4a2 	copyw	a0,a1,160,3
[0-9a-f]+ <[^>]*> f035 e5c3 	copyw	a1,a2,336,4
[0-9a-f]+ <[^>]*> f000 e6e0 	ucopyw	a2,a3,0,1
[0-9a-f]+ <[^>]*> f001 e701 	ucopyw	a3,s0,16,2
[0-9a-f]+ <[^>]*> f002 e022 	ucopyw	s0,s1,32,3
[0-9a-f]+ <[^>]*> f004 e143 	ucopyw	s1,v0,64,4
[0-9a-f]+ <[^>]*> f008 e260 	ucopyw	v0,v1,128,1
[0-9a-f]+ <[^>]*> f010 e381 	ucopyw	v1,a0,256,2
[0-9a-f]+ <[^>]*> f018 e4a2 	ucopyw	a0,a1,384,3
[0-9a-f]+ <[^>]*> f01c e5c3 	ucopyw	a1,a2,448,4
[0-9a-f]+ <[^>]*> f01e e6e0 	ucopyw	a2,a3,480,1
[0-9a-f]+ <[^>]*> f01f e701 	ucopyw	a3,s0,496,2
[0-9a-f]+ <[^>]*> f00a e022 	ucopyw	s0,s1,160,3
[0-9a-f]+ <[^>]*> f015 e143 	ucopyw	s1,v0,336,4
	\.\.\.
