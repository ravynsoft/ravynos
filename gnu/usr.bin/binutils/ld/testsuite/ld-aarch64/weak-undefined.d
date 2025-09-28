#source: weak-undefined.s
#ld: -Ttext 0xF0000000 -T relocs.ld -e0 --emit-relocs
#objdump: -d
#...
 +f0000000:	54000001 	b\.ne	f0000000 <main>  // .*
 +f0000004:	54000000 	b\.eq	f0000004 <main\+0x4>  // .*
 +f0000008:	54000002 	b\.cs	f0000008 <main\+0x8>  // .*
 +f000000c:	54000003 	b\.cc	f000000c <main\+0xc>  // .*
 +f0000010:	5400000c 	b\.gt	f0000010 <main\+0x10>
 +f0000014:	5400000a 	b\.ge	f0000014 <main\+0x14>  // .*
 +f0000018:	5400000b 	b\.lt	f0000018 <main\+0x18>  // .*
 +f000001c:	5400000d 	b\.le	f000001c <main\+0x1c>
 +f0000020:	d503201f 	nop
 +f0000024:	d503201f 	nop
 +f0000028:	58000000 	ldr	x0, f0000028 <main\+0x28>
 +f000002c:	10000000 	adr	x0, f000002c <main\+0x2c>
 +f0000030:	90000000 	adrp	x0, f0000000 <main>
 +f0000034:	91000000 	add	x0, x0, #0x0
