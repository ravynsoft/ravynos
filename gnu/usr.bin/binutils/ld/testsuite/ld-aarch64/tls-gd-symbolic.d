# Testcase to show that -Bsymbolic does not trigger any relaxation from general
# dynamic or initial exec for global symbols.
#target: [check_shared_lib_support]
#ld: -shared -Bsymbolic
#objdump: -d -j .text

.*:     file format .*


Disassembly of section \.text:

[0-9a-f]+ <_test_tls_desc>:
 +[0-9a-f]+:	........ 	adrp	x0, .*
 +[0-9a-f]+:	........ 	ldr	x1, \[x0, #.*\]
 +[0-9a-f]+:	........ 	add	x0, x0, .*
 +[0-9a-f]+:	d63f0020 	blr	x1

[0-9a-f]+ <_test_tls_desc2>:
 +[0-9a-f]+:	........ 	adrp	x0, .*
 +[0-9a-f]+:	........ 	ldr	x0, \[x0, #.*\]
