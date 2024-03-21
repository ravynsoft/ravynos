#as: --32 -mbranches-within-32B-boundaries
#ld: -melf_i386
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	85 d2                	test   %edx,%edx
 +[a-f0-9]+:	74 20                	je     +[a-f0-9]+ <_start\+0x24>
 +[a-f0-9]+:	85 d2                	test   %edx,%edx
 +[a-f0-9]+:	74 1c                	je     +[a-f0-9]+ <_start\+0x24>
 +[a-f0-9]+:	85 ff                	test   %edi,%edi
 +[a-f0-9]+:	74 18                	je     +[a-f0-9]+ <_start\+0x24>
 +[a-f0-9]+:	65 a1 00 00 00 00    	mov    %gs:0x0,%eax
 +[a-f0-9]+:	90                   	nop
 +[a-f0-9]+:	8d 74 26 00          	lea    0x0\(%esi,%eiz,1\),%esi
 +[a-f0-9]+:	3e 3e 3e 8b 90 fc ff ff ff 	ds ds mov %ds:-0x4\(%eax\),%edx
 +[a-f0-9]+:	85 d2                	test   %edx,%edx
 +[a-f0-9]+:	74 00                	je     +[a-f0-9]+ <_start\+0x24>
 +[a-f0-9]+:	c3                   	ret
#pass
