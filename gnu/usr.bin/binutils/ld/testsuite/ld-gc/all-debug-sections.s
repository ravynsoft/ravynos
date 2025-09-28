 .section .text.useless,"ax",%progbits
 .globl	useless
 .type useless, %function
useless:
 .long 1

 .section .text.main,"ax",%progbits
 .globl _start
_start:
 .globl main
 .type main, %function
main:
 .long 2

 .section .text,"ax",%progbits
 .long main

 .section .debug_info.text.main,"",%progbits
debug_info_main:
 .long 0x3c
 .long main

 .section .debug_info.text.useless,"",%progbits
debug_info_useless:
 .long 0x38
 .long useless

 .section .debug_info,"",%progbits
 .long 0x49

 .section .debug_aranges,"",%progbits
 .long 0x3c

 .section .debug_aranges.text.main,"",%progbits
debug_aranges_main:
 .long 0x2c
 .long main

 .section .debug_aranges.text.useless,"",%progbits
debug_aranges_useless:
 .long 0x2c
 .long useless

 .section .debug_line,"",%progbits
 .long 0x3c

 .section .debug_line.text.main,"",%progbits
debug_line_main:
 .long 0x2c
 .long main

 .section .debug_line.text.useless,"",%progbits
debug_line_useless:
 .long 0x2c
 .long useless

