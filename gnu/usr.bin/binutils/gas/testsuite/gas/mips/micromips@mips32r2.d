#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: MIPS MIPS32r2 non-fp instructions
#source: mips32r2.s
#as: -32

# Check MIPS32 Release 2 (mips32r2) *non-fp* instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0000 1800 	ehb
[0-9a-f]+ <[^>]*> 0085 39ac 	ext	\$4,\$5,0x6,0x8
[0-9a-f]+ <[^>]*> 0085 698c 	ins	\$4,\$5,0x6,0x8
[0-9a-f]+ <[^>]*> 03e8 1f3c 	jalr\.hb	\$8
[0-9a-f]+ <[^>]*> 0289 1f3c 	jalr\.hb	\$20,\$9
[0-9a-f]+ <[^>]*> 0008 1f3c 	jr\.hb	\$8
[0-9a-f]+ <[^>]*> 0140 6b3c 	rdhwr	\$10,\$0
[0-9a-f]+ <[^>]*> 0161 6b3c 	rdhwr	\$11,\$1
[0-9a-f]+ <[^>]*> 0182 6b3c 	rdhwr	\$12,\$2
[0-9a-f]+ <[^>]*> 01a3 6b3c 	rdhwr	\$13,\$3
[0-9a-f]+ <[^>]*> 01c4 6b3c 	rdhwr	\$14,\$4
[0-9a-f]+ <[^>]*> 01e5 6b3c 	rdhwr	\$15,\$5
[0-9a-f]+ <[^>]*> 032a e0c0 	ror	\$25,\$10,0x1c
[0-9a-f]+ <[^>]*> 032a 20c0 	ror	\$25,\$10,0x4
[0-9a-f]+ <[^>]*> 0080 c9d0 	negu	\$25,\$4
[0-9a-f]+ <[^>]*> 0159 c8d0 	rorv	\$25,\$10,\$25
[0-9a-f]+ <[^>]*> 0144 c8d0 	rorv	\$25,\$10,\$4
[0-9a-f]+ <[^>]*> 0144 c8d0 	rorv	\$25,\$10,\$4
[0-9a-f]+ <[^>]*> 00e7 2b3c 	seb	\$7,\$7
[0-9a-f]+ <[^>]*> 010a 2b3c 	seb	\$8,\$10
[0-9a-f]+ <[^>]*> 00e7 3b3c 	seh	\$7,\$7
[0-9a-f]+ <[^>]*> 010a 3b3c 	seh	\$8,\$10
[0-9a-f]+ <[^>]*> 420a 5555 	synci	21845\(\$10\)
[0-9a-f]+ <[^>]*> 00e7 7b3c 	wsbh	\$7,\$7
[0-9a-f]+ <[^>]*> 010a 7b3c 	wsbh	\$8,\$10
[0-9a-f]+ <[^>]*> 0000 477c 	di
[0-9a-f]+ <[^>]*> 0000 477c 	di
[0-9a-f]+ <[^>]*> 000a 477c 	di	\$10
[0-9a-f]+ <[^>]*> 0000 577c 	ei
[0-9a-f]+ <[^>]*> 0000 577c 	ei
[0-9a-f]+ <[^>]*> 000a 577c 	ei	\$10
[0-9a-f]+ <[^>]*> 0159 e17c 	rdpgpr	\$10,\$25
[0-9a-f]+ <[^>]*> 0159 f17c 	wrpgpr	\$10,\$25
[0-9a-f]+ <[^>]*> 0000 2800 	pause
	\.\.\.
