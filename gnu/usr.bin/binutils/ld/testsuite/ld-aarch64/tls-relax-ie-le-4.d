# We already test that we relax an access to a local symbol, this testcase
# checks that we relax an access to a global-binding symbol if the static linker
# knows that the symbol will resolve to the executable local value.
#
# The access should be relaxed to a LE access.
#ld:
#objdump: -d

.*:     file format .*


Disassembly of section \.text:

[0-9a-f]+ <_start>:
 +[0-9a-f]+:	d2800000 	mov	x0, #0x0                   	// #0
 +[0-9a-f]+:	d53bd041 	mrs	x1, tpidr_el0
 +[0-9a-f]+:	d2a00000 	movz	x0, #0x0, lsl #16
 +[0-9a-f]+:	f2800200 	movk	x0, #0x10
 +[0-9a-f]+:	b8606820 	ldr	w0, \[x1, x0\]
 +[0-9a-f]+:	d65f03c0 	ret
