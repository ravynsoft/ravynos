#objdump: -dw
#name: i386 TBM

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	8f ea 78 10 1c f2 67 00 00 00 	bextr  \$0x67,\(%edx,%esi,8\),%ebx
[ 	]*[a-f0-9]+:	8f ea 78 10 c6 00 00 00 00 	bextr  \$0x0,%esi,%eax
[ 	]*[a-f0-9]+:	8f ea 78 10 f8 ff ff ff 7f 	bextr  \$0x7fffffff,%eax,%edi
[ 	]*[a-f0-9]+:	8f ea 78 10 26 b2 35 00 00 	bextr  \$0x35b2,\(%esi\),%esp
[ 	]*[a-f0-9]+:	8f ea 78 10 ef 86 9c 00 00 	bextr  \$0x9c86,%edi,%ebp
[ 	]*[a-f0-9]+:	8f ea 78 10 c9 03 00 00 00 	bextr  \$0x3,%ecx,%ecx
[ 	]*[a-f0-9]+:	8f ea 78 10 74 43 fd ee 00 00 00 	bextr  \$0xee,-0x3\(%ebx,%eax,2\),%esi
[ 	]*[a-f0-9]+:	8f ea 78 10 23 55 00 00 00 	bextr  \$0x55,\(%ebx\),%esp
[ 	]*[a-f0-9]+:	8f ea 78 10 12 e8 4e 00 00 	bextr  \$0x4ee8,\(%edx\),%edx
[ 	]*[a-f0-9]+:	8f ea 78 10 fb 00 00 00 00 	bextr  \$0x0,%ebx,%edi
[ 	]*[a-f0-9]+:	8f ea 78 10 f4 dc 00 00 00 	bextr  \$0xdc,%esp,%esi
[ 	]*[a-f0-9]+:	8f ea 78 10 00 a9 00 00 00 	bextr  \$0xa9,\(%eax\),%eax
[ 	]*[a-f0-9]+:	8f ea 78 10 ea 89 01 00 00 	bextr  \$0x189,%edx,%ebp
[ 	]*[a-f0-9]+:	8f ea 78 10 0c 41 84 00 00 00 	bextr  \$0x84,\(%ecx,%eax,2\),%ecx
[ 	]*[a-f0-9]+:	8f ea 78 10 04 01 fe ca 00 00 	bextr  \$0xcafe,\(%ecx,%eax,1\),%eax
[ 	]*[a-f0-9]+:	8f ea 78 10 bc 3e 09 71 00 00 ad de 00 00 	bextr  \$0xdead,0x7109\(%esi,%edi,1\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 09       	blcfill \(%ecx\),%eax
[ 	]*[a-f0-9]+:	8f e9 40 01 ce       	blcfill %esi,%edi
[ 	]*[a-f0-9]+:	8f e9 70 01 c8       	blcfill %eax,%ecx
[ 	]*[a-f0-9]+:	8f e9 48 01 cf       	blcfill %edi,%esi
[ 	]*[a-f0-9]+:	8f e9 58 01 0e       	blcfill \(%esi\),%esp
[ 	]*[a-f0-9]+:	8f e9 50 01 0b       	blcfill \(%ebx\),%ebp
[ 	]*[a-f0-9]+:	8f e9 68 01 8c 03 95 1a 00 00 	blcfill 0x1a95\(%ebx,%eax,1\),%edx
[ 	]*[a-f0-9]+:	8f e9 40 01 0a       	blcfill \(%edx\),%edi
[ 	]*[a-f0-9]+:	8f e9 40 01 cb       	blcfill %ebx,%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 8c 30 ce 00 00 00 	blcfill 0xce\(%eax,%esi,1\),%eax
[ 	]*[a-f0-9]+:	8f e9 78 01 0c 1d 02 35 ff ff 	blcfill -0xcafe\(,%ebx,1\),%eax
[ 	]*[a-f0-9]+:	8f e9 60 01 0c 05 a1 51 ff ff 	blcfill -0xae5f\(,%eax,1\),%ebx
[ 	]*[a-f0-9]+:	8f e9 40 01 c9       	blcfill %ecx,%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 cc       	blcfill %esp,%eax
[ 	]*[a-f0-9]+:	8f e9 40 01 cd       	blcfill %ebp,%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 0c 4e    	blcfill \(%esi,%ecx,2\),%eax
[ 	]*[a-f0-9]+:	8f e9 70 02 f0       	blci   %eax,%ecx
[ 	]*[a-f0-9]+:	8f e9 60 02 f1       	blci   %ecx,%ebx
[ 	]*[a-f0-9]+:	8f e9 78 02 34 45 b0 12 00 00 	blci   0x12b0\(,%eax,2\),%eax
[ 	]*[a-f0-9]+:	8f e9 40 02 30       	blci   \(%eax\),%edi
[ 	]*[a-f0-9]+:	8f e9 48 02 f7       	blci   %edi,%esi
[ 	]*[a-f0-9]+:	8f e9 68 02 f4       	blci   %esp,%edx
[ 	]*[a-f0-9]+:	8f e9 50 02 f6       	blci   %esi,%ebp
[ 	]*[a-f0-9]+:	8f e9 78 02 f2       	blci   %edx,%eax
[ 	]*[a-f0-9]+:	8f e9 58 02 b4 83 57 8d ff ff 	blci   -0x72a9\(%ebx,%eax,4\),%esp
[ 	]*[a-f0-9]+:	8f e9 60 02 36       	blci   \(%esi\),%ebx
[ 	]*[a-f0-9]+:	8f e9 78 02 34 73    	blci   \(%ebx,%esi,2\),%eax
[ 	]*[a-f0-9]+:	8f e9 68 02 33       	blci   \(%ebx\),%edx
[ 	]*[a-f0-9]+:	8f e9 78 02 f3       	blci   %ebx,%eax
[ 	]*[a-f0-9]+:	8f e9 70 02 b4 93 a2 e0 00 00 	blci   0xe0a2\(%ebx,%edx,4\),%ecx
[ 	]*[a-f0-9]+:	8f e9 40 02 37       	blci   \(%edi\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 02 34 45 ff ff ff 3f 	blci   0x3fffffff\(,%eax,2\),%eax
[ 	]*[a-f0-9]+:	8f e9 70 01 ef       	blcic  %edi,%ecx
[ 	]*[a-f0-9]+:	8f e9 40 01 e8       	blcic  %eax,%edi
[ 	]*[a-f0-9]+:	8f e9 60 01 28       	blcic  \(%eax\),%ebx
[ 	]*[a-f0-9]+:	8f e9 68 01 e9       	blcic  %ecx,%edx
[ 	]*[a-f0-9]+:	8f e9 58 01 ee       	blcic  %esi,%esp
[ 	]*[a-f0-9]+:	8f e9 50 01 2c 1d 02 35 ff ff 	blcic  -0xcafe\(,%ebx,1\),%ebp
[ 	]*[a-f0-9]+:	8f e9 78 01 ed       	blcic  %ebp,%eax
[ 	]*[a-f0-9]+:	8f e9 48 01 2e       	blcic  \(%esi\),%esi
[ 	]*[a-f0-9]+:	8f e9 60 01 ec       	blcic  %esp,%ebx
[ 	]*[a-f0-9]+:	8f e9 48 01 2c 3f    	blcic  \(%edi,%edi,1\),%esi
[ 	]*[a-f0-9]+:	8f e9 50 01 2c 35 01 00 00 c0 	blcic  -0x3fffffff\(,%esi,1\),%ebp
[ 	]*[a-f0-9]+:	8f e9 40 01 2b       	blcic  \(%ebx\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 6c c7 08 	blcic  0x8\(%edi,%eax,8\),%eax
[ 	]*[a-f0-9]+:	8f e9 40 01 a9 d1 4a 57 3a 	blcic  0x3a574ad1\(%ecx\),%edi
[ 	]*[a-f0-9]+:	8f e9 40 01 ec       	blcic  %esp,%edi
[ 	]*[a-f0-9]+:	8f e9 40 01 ea       	blcic  %edx,%edi
[ 	]*[a-f0-9]+:	8f e9 40 02 48 0c    	blcmsk 0xc\(%eax\),%edi
[ 	]*[a-f0-9]+:	8f e9 50 02 0c 16    	blcmsk \(%esi,%edx,1\),%ebp
[ 	]*[a-f0-9]+:	8f e9 70 02 8f 00 22 3d e2 	blcmsk -0x1dc2de00\(%edi\),%ecx
[ 	]*[a-f0-9]+:	8f e9 58 02 c8       	blcmsk %eax,%esp
[ 	]*[a-f0-9]+:	8f e9 78 02 0c 57    	blcmsk \(%edi,%edx,2\),%eax
[ 	]*[a-f0-9]+:	8f e9 68 02 0b       	blcmsk \(%ebx\),%edx
[ 	]*[a-f0-9]+:	8f e9 40 02 0a       	blcmsk \(%edx\),%edi
[ 	]*[a-f0-9]+:	8f e9 48 02 ce       	blcmsk %esi,%esi
[ 	]*[a-f0-9]+:	8f e9 40 02 cc       	blcmsk %esp,%edi
[ 	]*[a-f0-9]+:	8f e9 58 02 cf       	blcmsk %edi,%esp
[ 	]*[a-f0-9]+:	8f e9 60 02 0c c3    	blcmsk \(%ebx,%eax,8\),%ebx
[ 	]*[a-f0-9]+:	8f e9 78 02 0f       	blcmsk \(%edi\),%eax
[ 	]*[a-f0-9]+:	8f e9 78 02 ca       	blcmsk %edx,%eax
[ 	]*[a-f0-9]+:	8f e9 40 02 4c 3b 67 	blcmsk 0x67\(%ebx,%edi,1\),%edi
[ 	]*[a-f0-9]+:	8f e9 40 02 0c 05 a0 d8 12 aa 	blcmsk -0x55ed2760\(,%eax,1\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 02 0c 05 01 00 00 00 	blcmsk 0x1\(,%eax,1\),%eax
[ 	]*[a-f0-9]+:	8f e9 48 01 da       	blcs   %edx,%esi
[ 	]*[a-f0-9]+:	8f e9 78 01 1b       	blcs   \(%ebx\),%eax
[ 	]*[a-f0-9]+:	8f e9 40 01 d8       	blcs   %eax,%edi
[ 	]*[a-f0-9]+:	8f e9 58 01 9c 01 fe ca 00 00 	blcs   0xcafe\(%ecx,%eax,1\),%esp
[ 	]*[a-f0-9]+:	8f e9 50 01 df       	blcs   %edi,%ebp
[ 	]*[a-f0-9]+:	8f e9 70 01 1a       	blcs   \(%edx\),%ecx
[ 	]*[a-f0-9]+:	8f e9 40 01 1f       	blcs   \(%edi\),%edi
[ 	]*[a-f0-9]+:	8f e9 60 01 9b 02 35 ff ff 	blcs   -0xcafe\(%ebx\),%ebx
[ 	]*[a-f0-9]+:	8f e9 70 01 dc       	blcs   %esp,%ecx
[ 	]*[a-f0-9]+:	8f e9 68 01 de       	blcs   %esi,%edx
[ 	]*[a-f0-9]+:	8f e9 40 01 18       	blcs   \(%eax\),%edi
[ 	]*[a-f0-9]+:	8f e9 40 01 1c 0d 01 00 00 00 	blcs   0x1\(,%ecx,1\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 d9       	blcs   %ecx,%eax
[ 	]*[a-f0-9]+:	8f e9 40 01 1c 13    	blcs   \(%ebx,%edx,1\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 9c 00 53 21 ff ff 	blcs   -0xdead\(%eax,%eax,1\),%eax
[ 	]*[a-f0-9]+:	8f e9 40 01 1c 13    	blcs   \(%ebx,%edx,1\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 d0       	blsfill %eax,%eax
[ 	]*[a-f0-9]+:	8f e9 48 01 d1       	blsfill %ecx,%esi
[ 	]*[a-f0-9]+:	8f e9 40 01 10       	blsfill \(%eax\),%edi
[ 	]*[a-f0-9]+:	8f e9 58 01 d3       	blsfill %ebx,%esp
[ 	]*[a-f0-9]+:	8f e9 68 01 d2       	blsfill %edx,%edx
[ 	]*[a-f0-9]+:	8f e9 70 01 11       	blsfill \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	8f e9 40 01 d7       	blsfill %edi,%edi
[ 	]*[a-f0-9]+:	8f e9 50 01 d5       	blsfill %ebp,%ebp
[ 	]*[a-f0-9]+:	8f e9 40 01 17       	blsfill \(%edi\),%edi
[ 	]*[a-f0-9]+:	8f e9 60 01 13       	blsfill \(%ebx\),%ebx
[ 	]*[a-f0-9]+:	8f e9 78 01 16       	blsfill \(%esi\),%eax
[ 	]*[a-f0-9]+:	8f e9 78 01 14 80    	blsfill \(%eax,%eax,4\),%eax
[ 	]*[a-f0-9]+:	8f e9 40 01 d6       	blsfill %esi,%edi
[ 	]*[a-f0-9]+:	8f e9 40 01 94 18 21 a2 00 00 	blsfill 0xa221\(%eax,%ebx,1\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 14 00    	blsfill \(%eax,%eax,1\),%eax
[ 	]*[a-f0-9]+:	8f e9 70 01 14 5d f8 ff ff ff 	blsfill -0x8\(,%ebx,2\),%ecx
[ 	]*[a-f0-9]+:	8f e9 40 01 f0       	blsic  %eax,%edi
[ 	]*[a-f0-9]+:	8f e9 60 01 36       	blsic  \(%esi\),%ebx
[ 	]*[a-f0-9]+:	8f e9 50 01 34 5d 00 00 00 00 	blsic  0x0\(,%ebx,2\),%ebp
[ 	]*[a-f0-9]+:	8f e9 78 01 34 41    	blsic  \(%ecx,%eax,2\),%eax
[ 	]*[a-f0-9]+:	8f e9 58 01 37       	blsic  \(%edi\),%esp
[ 	]*[a-f0-9]+:	8f e9 78 01 33       	blsic  \(%ebx\),%eax
[ 	]*[a-f0-9]+:	8f e9 70 01 f7       	blsic  %edi,%ecx
[ 	]*[a-f0-9]+:	8f e9 40 01 74 18 51 	blsic  0x51\(%eax,%ebx,1\),%edi
[ 	]*[a-f0-9]+:	8f e9 68 01 f4       	blsic  %esp,%edx
[ 	]*[a-f0-9]+:	8f e9 68 01 74 3e 99 	blsic  -0x67\(%esi,%edi,1\),%edx
[ 	]*[a-f0-9]+:	8f e9 40 01 31       	blsic  \(%ecx\),%edi
[ 	]*[a-f0-9]+:	8f e9 48 01 74 8e 67 	blsic  0x67\(%esi,%ecx,4\),%esi
[ 	]*[a-f0-9]+:	8f e9 40 01 b4 d3 81 00 00 00 	blsic  0x81\(%ebx,%edx,8\),%edi
[ 	]*[a-f0-9]+:	8f e9 40 01 74 11 0e 	blsic  0xe\(%ecx,%edx,1\),%edi
[ 	]*[a-f0-9]+:	8f e9 58 01 70 3b    	blsic  0x3b\(%eax\),%esp
[ 	]*[a-f0-9]+:	8f e9 40 01 f1       	blsic  %ecx,%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 f8       	t1mskc %eax,%eax
[ 	]*[a-f0-9]+:	8f e9 40 01 ff       	t1mskc %edi,%edi
[ 	]*[a-f0-9]+:	8f e9 70 01 39       	t1mskc \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	8f e9 48 01 3c 33    	t1mskc \(%ebx,%esi,1\),%esi
[ 	]*[a-f0-9]+:	8f e9 50 01 fa       	t1mskc %edx,%ebp
[ 	]*[a-f0-9]+:	8f e9 68 01 3c 0d 00 00 00 00 	t1mskc 0x0\(,%ecx,1\),%edx
[ 	]*[a-f0-9]+:	8f e9 58 01 3c b5 00 00 00 00 	t1mskc 0x0\(,%esi,4\),%esp
[ 	]*[a-f0-9]+:	8f e9 70 01 fb       	t1mskc %ebx,%ecx
[ 	]*[a-f0-9]+:	8f e9 60 01 3b       	t1mskc \(%ebx\),%ebx
[ 	]*[a-f0-9]+:	8f e9 40 01 fc       	t1mskc %esp,%edi
[ 	]*[a-f0-9]+:	8f e9 40 01 38       	t1mskc \(%eax\),%edi
[ 	]*[a-f0-9]+:	8f e9 78 01 f9       	t1mskc %ecx,%eax
[ 	]*[a-f0-9]+:	8f e9 40 01 b8 ad de 00 00 	t1mskc 0xdead\(%eax\),%edi
[ 	]*[a-f0-9]+:	8f e9 68 01 f9       	t1mskc %ecx,%edx
[ 	]*[a-f0-9]+:	8f e9 60 01 3c 15 ad de 00 00 	t1mskc 0xdead\(,%edx,1\),%ebx
[ 	]*[a-f0-9]+:	8f e9 40 01 3a       	t1mskc \(%edx\),%edi
[ 	]*[a-f0-9]+:	8f e9 58 01 23       	tzmsk  \(%ebx\),%esp
[ 	]*[a-f0-9]+:	8f e9 78 01 e7       	tzmsk  %edi,%eax
[ 	]*[a-f0-9]+:	8f e9 48 01 a7 02 35 ff ff 	tzmsk  -0xcafe\(%edi\),%esi
[ 	]*[a-f0-9]+:	8f e9 68 01 24 3d 00 00 00 00 	tzmsk  0x0\(,%edi,1\),%edx
[ 	]*[a-f0-9]+:	8f e9 50 01 e0       	tzmsk  %eax,%ebp
[ 	]*[a-f0-9]+:	8f e9 60 01 e5       	tzmsk  %ebp,%ebx
[ 	]*[a-f0-9]+:	8f e9 40 01 26       	tzmsk  \(%esi\),%edi
[ 	]*[a-f0-9]+:	8f e9 70 01 21       	tzmsk  \(%ecx\),%ecx
[ 	]*[a-f0-9]+:	8f e9 40 01 24 45 00 00 00 00 	tzmsk  0x0\(,%eax,2\),%edi
[ 	]*[a-f0-9]+:	8f e9 40 01 e7       	tzmsk  %edi,%edi
[ 	]*[a-f0-9]+:	8f e9 68 01 e4       	tzmsk  %esp,%edx
[ 	]*[a-f0-9]+:	8f e9 70 01 20       	tzmsk  \(%eax\),%ecx
[ 	]*[a-f0-9]+:	8f e9 78 01 24 3a    	tzmsk  \(%edx,%edi,1\),%eax
[ 	]*[a-f0-9]+:	8f e9 78 01 23       	tzmsk  \(%ebx\),%eax
[ 	]*[a-f0-9]+:	8f e9 78 01 a3 d9 c6 2a 2a 	tzmsk  0x2a2ac6d9\(%ebx\),%eax
[ 	]*[a-f0-9]+:	8f e9 70 01 a4 01 47 e9 ff ff 	tzmsk  -0x16b9\(%ecx,%eax,1\),%ecx
#pass
