# The linker recognises that if we have one IE access to a TLS symbol then all
# accesses to that symbol could be IE.  Here we are also interested to check
# the linker does not also decide that a second access to that symbol could be
# LE.
#target: [check_shared_lib_support]
#ld: -shared
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

[0-9a-f]+ <foo>:
 +[0-9a-f]+:	d2800000 	mov	x0, #0x0                   	// #0
 +[0-9a-f]+:	d53bd041 	mrs	x1, tpidr_el0
 +[0-9a-f]+:	.* 	adrp	x0, .*
 +[0-9a-f]+:	.* 	ldr	x0, \[x0, #.*\]
 +[0-9a-f]+:	b8606820 	ldr	w0, \[x1, x0\]
 +[0-9a-f]+:	d53bd041 	mrs	x1, tpidr_el0
 +[0-9a-f]+:	910003fd 	mov	x29, sp
 +[0-9a-f]+:	.* 	adrp	x0, .*
 +[0-9a-f]+:	.* 	ldr	x0, \[x0, #.*\]
 +[0-9a-f]+:	d503201f 	nop
 +[0-9a-f]+:	d503201f 	nop
 +[0-9a-f]+:	b8606820 	ldr	w0, \[x1, x0\]
 +[0-9a-f]+:	d65f03c0 	ret
