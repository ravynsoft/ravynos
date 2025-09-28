#name: i386 32-bit wrapping calculations (text)
#source: wrap32.s
#objdump: -dwr

.*: +file format .*

Disassembly of section .text:

0+ <wrap>:
[ 	]*[0-9a-f]+:[ 	]+b8 f4 00 00 00       	mov    \$0xf4,%eax
[ 	]*[0-9a-f]+:[ 	]+ba f4 00 00 00       	mov    \$0xf4,%edx
[ 	]*[0-9a-f]+:[ 	]+b8 90 00 00 00       	mov    \$0x90,%eax
[ 	]*[0-9a-f]+:[ 	]+ba 90 00 00 00       	mov    \$0x90,%edx
[ 	]*[0-9a-f]+:[ 	]+b8 00 ff ff ff       	mov    \$0xffffff00,%eax[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+ba 00 ff ff ff       	mov    \$0xffffff00,%edx[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+b8 f4 00 00 00       	mov    \$0xf4,%eax[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+ba f4 00 00 00       	mov    \$0xf4,%edx[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+c7 00 f4 00 00 00    	movl   \$0xf4,\(%eax\)
[ 	]*[0-9a-f]+:[ 	]+c7 02 f4 00 00 00    	movl   \$0xf4,\(%edx\)
[ 	]*[0-9a-f]+:[ 	]+c7 00 90 00 00 00    	movl   \$0x90,\(%eax\)
[ 	]*[0-9a-f]+:[ 	]+c7 02 90 00 00 00    	movl   \$0x90,\(%edx\)
[ 	]*[0-9a-f]+:[ 	]+c7 00 00 ff ff ff    	movl   \$0xffffff00,\(%eax\)[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+c7 02 00 ff ff ff    	movl   \$0xffffff00,\(%edx\)[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+c7 00 f4 00 00 00    	movl   \$0xf4,\(%eax\)[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+c7 02 f4 00 00 00    	movl   \$0xf4,\(%edx\)[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+81 c1 f4 00 00 00    	add    \$0xf4,%ecx
[ 	]*[0-9a-f]+:[ 	]+81 c2 f4 00 00 00    	add    \$0xf4,%edx
[ 	]*[0-9a-f]+:[ 	]+81 c1 90 00 00 00    	add    \$0x90,%ecx
[ 	]*[0-9a-f]+:[ 	]+81 c2 90 00 00 00    	add    \$0x90,%edx
[ 	]*[0-9a-f]+:[ 	]+81 c1 00 ff ff ff    	add    \$0xffffff00,%ecx[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+81 c2 00 ff ff ff    	add    \$0xffffff00,%edx[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+81 c1 f4 00 00 00    	add    \$0xf4,%ecx[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+81 c2 f4 00 00 00    	add    \$0xf4,%edx[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+81 00 f4 00 00 00    	addl   \$0xf4,\(%eax\)
[ 	]*[0-9a-f]+:[ 	]+81 02 f4 00 00 00    	addl   \$0xf4,\(%edx\)
[ 	]*[0-9a-f]+:[ 	]+81 00 90 00 00 00    	addl   \$0x90,\(%eax\)
[ 	]*[0-9a-f]+:[ 	]+81 02 90 00 00 00    	addl   \$0x90,\(%edx\)
[ 	]*[0-9a-f]+:[ 	]+81 00 00 ff ff ff    	addl   \$0xffffff00,\(%eax\)[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+81 02 00 ff ff ff    	addl   \$0xffffff00,\(%edx\)[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+81 00 f4 00 00 00    	addl   \$0xf4,\(%eax\)[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+81 02 f4 00 00 00    	addl   \$0xf4,\(%edx\)[ 	]+[0-9a-f]+: (R_386_|dir)?32[ 	]+sym
[ 	]*[0-9a-f]+:[ 	]+c3                   	ret
#pass
