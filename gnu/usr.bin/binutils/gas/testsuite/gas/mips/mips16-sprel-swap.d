#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS16 jump delay slot scheduling for SP-relative instructions
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> eb00      	jr	v1
[0-9a-f]+ <[^>]*> d204      	sw	v0,16\(sp\)
[0-9a-f]+ <[^>]*> eb00      	jr	v1
[0-9a-f]+ <[^>]*> 6206      	sw	ra,24\(sp\)
[0-9a-f]+ <[^>]*> eb00      	jr	v1
[0-9a-f]+ <[^>]*> f944      	sd	v0,32\(sp\)
[0-9a-f]+ <[^>]*> eb00      	jr	v1
[0-9a-f]+ <[^>]*> fa05      	sd	ra,40\(sp\)
[0-9a-f]+ <[^>]*> eb00      	jr	v1
[0-9a-f]+ <[^>]*> 920c      	lw	v0,48\(sp\)
[0-9a-f]+ <[^>]*> eb00      	jr	v1
[0-9a-f]+ <[^>]*> f847      	ld	v0,56\(sp\)
	\.\.\.
