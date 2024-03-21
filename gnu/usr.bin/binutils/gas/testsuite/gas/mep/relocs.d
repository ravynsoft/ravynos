
relocs.x:     file format elf32-mep-little

Contents of section .text:
 1000 00000000 00000000 00000000 00000000  ................
 1010 00000000 00000000 00000000 00000000  ................
 1020 00000000 00000000 00000000 00000000  ................
 1030 00003cc5 1210e9de ffff09e5 ecff0000  ..<.............
 1040 00003cc5 eeef49dd dfff09e5 d2ef0000  ..<...I.........
 1050 00003cc5 2c20b9de 0f0009e5 e90788dc  ..<., ..........
 1060 800018d8 0200c8df ff7f28df ff7f78df  ..........\(...x.
 1070 ff7f98dd 010098da 0f00f8db 700058da  ............p.X.
 1080 020028d8 000048d8 0000d8d8 100098d8  ..\(...H.........
 1090 100008d8 000008d9 000008d9 000008d8  ................
 10a0 000008d8 000008d9 000008d9 00000000  ................
 10b0 00000000 000008d8 00000000 00000000  ................
Contents of section .rostacktab:
 10c0 f0ff1f00                             ....            
Contents of section .data:
 11c4 2a000000                             \*...            
Disassembly of section .text:

00001000 <junk1>:
    1000:	00 00       	nop
    1002:	00 00       	nop
    1004:	00 00       	nop
    1006:	00 00       	nop
    1008:	00 00       	nop
    100a:	00 00       	nop
    100c:	00 00       	nop
    100e:	00 00       	nop
    1010:	00 00       	nop

00001012 <foo>:
    1012:	00 00       	nop
    1014:	00 00       	nop
    1016:	00 00       	nop
    1018:	00 00       	nop

0000101a <bar>:
    101a:	00 00       	nop
    101c:	00 00       	nop
    101e:	00 00       	nop
    1020:	00 00       	nop
    1022:	00 00       	nop

00001024 <junk2>:
    1024:	00 00       	nop
    1026:	00 00       	nop
    1028:	00 00       	nop
    102a:	00 00       	nop
    102c:	00 00       	nop

0000102e <main>:
    102e:	00 00       	nop
    1030:	00 00       	nop
    1032:	3c c5 12 10 	lb \$5,4114\(\$3\)
    1036:	e9 de ff ff 	bsr 1012 <foo>
    103a:	09 e5 ec ff 	repeat \$5,1012 <foo>
    103e:	00 00       	nop
    1040:	00 00       	nop
    1042:	3c c5 ee ef 	lb \$5,-4114\(\$3\)
    1046:	49 dd df ff 	bsr ffffefee <0-:s3:foo.*>
    104a:	09 e5 d2 ef 	repeat \$5,ffffefee <0-:s3:foo.*>
    104e:	00 00       	nop
    1050:	00 00       	nop
    1052:	3c c5 2c 20 	lb \$5,8236\(\$3\)
    1056:	b9 de 0f 00 	bsr 202c <\+:s3:foo:s3:bar>
    105a:	09 e5 e9 07 	repeat \$5,202c <\+:s3:foo:s3:bar>
    105e:	88 dc 80 00 	jmp 8090 <<<:s3:foo:#0+03>
    1062:	18 d8 02 00 	jmp 202 <>>:s3:foo:#0+03>
    1066:	c8 df ff 7f 	jmp 7ffff8 <&:-:s3:foo:s3:bar:#0+7fffff>
    106a:	28 df ff 7f 	jmp 7fffe4 <&:-:s3:foo:s4:main:#0+7fffff>
    106e:	78 df ff 7f 	jmp 7fffee <&:-:S5:.text:s3:foo:#0+7fffff>
    1072:	98 dd 01 00 	jmp 1b2 <&:-:S5:.data:s3:foo:#0+7fffff>
    1076:	98 da 0f 00 	jmp f52 <-:s3:foo:\+:s9:.text.end:0-:S5:.text>
    107a:	f8 db 70 00 	jmp 707e <\*:s3:foo:#0+07>
    107e:	58 da 02 00 	jmp 24a <>>:s3:foo:#0+03\+0x48>
    1082:	28 d8 00 00 	jmp 4 <__assert_based_size\+0x3>
    1086:	48 d8 00 00 	jmp 8 <\^:s3:foo:s3:bar>
    108a:	d8 d8 10 00 	jmp 101a <|:s3:foo:s3:bar>
    108e:	98 d8 10 00 	jmp 1012 <foo>
    1092:	08 d8 00 00 	jmp 0 <<<:==:s3:foo:s3:bar:#0+05>
    1096:	08 d9 00 00 	jmp 20 <<<:&&:s3:foo:s3:bar:#0+05>
    109a:	08 d9 00 00 	jmp 20 <<<:&&:s3:foo:s3:bar:#0+05>
    109e:	08 d8 00 00 	jmp 0 <<<:==:s3:foo:s3:bar:#0+05>
    10a2:	08 d8 00 00 	jmp 0 <<<:==:s3:foo:s3:bar:#0+05>
    10a6:	08 d9 00 00 	jmp 20 <<<:&&:s3:foo:s3:bar:#0+05>
    10aa:	08 d9 00 00 	jmp 20 <<<:&&:s3:foo:s3:bar:#0+05>
    10ae:	00 00       	nop
    10b0:	00 00       	nop
    10b2:	00 00       	nop
    10b4:	00 00       	nop
    10b6:	08 d8 00 00 	jmp 0 <<<:==:s3:foo:s3:bar:#0+05>
    10ba:	00 00       	nop
    10bc:	00 00       	nop
    10be:	00 00       	nop
#pass
