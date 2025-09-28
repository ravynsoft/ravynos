#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16e2 link PC-relative operations 1
#source: ../../../gas/testsuite/gas/mips/mips16-pcrel-1.s
#as: -mips64r2 -mmips16e2
#ld: -Ttext 0 -e 0

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> fe40      	dla	v0,0+010000 <.*>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> fc40      	ld	v0,0+010000 <.*>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> fe5f      	dla	v0,0+010084 <.*>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> fc5f      	ld	v0,0+010100 <.*>
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f080 fe40 	dla	v0,0+010090 <.*>
[0-9a-f]+ <[^>]*> f100 fc40 	ld	v0,0+010110 <.*>
[0-9a-f]+ <[^>]*> f7ff fe5c 	dla	v0,0+010014 <.*>
[0-9a-f]+ <[^>]*> f7ff fc5c 	ld	v0,0+010014 <.*>
[0-9a-f]+ <[^>]*> f7ef fe5f 	dla	v0,0+01801f <.*>
[0-9a-f]+ <[^>]*> f7ef fc5f 	ld	v0,0+01801f <.*>
[0-9a-f]+ <[^>]*> f010 fe40 	dla	v0,0+008028 <.*>
[0-9a-f]+ <[^>]*> f010 fc40 	ld	v0,0+008028 <.*>
[0-9a-f]+ <[^>]*> f000 6a22 	lui	v0,0x2
[0-9a-f]+ <[^>]*> f030 fd50 	daddiu	v0,-32720
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f000 6a22 	lui	v0,0x2
[0-9a-f]+ <[^>]*> f030 3a58 	ld	v0,-32712\(v0\)
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f000 6a21 	lui	v0,0x1
[0-9a-f]+ <[^>]*> f050 fd47 	daddiu	v0,-32697
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f000 6a21 	lui	v0,0x1
[0-9a-f]+ <[^>]*> f050 3a4f 	ld	v0,-32689\(v0\)
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
