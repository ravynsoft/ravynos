.section .rodata.str1.1 ,"aMS",@progbits,0x1
_start:
.string "abcdefgh"
L001:
.string "hogehoge"
L002:
.string "fooooooo"
L003:

.text
.align  0x02
mov (L001@GOTOFF,A2),D0
mov (L004@GOTOFF,A2),D1


.section .rodata.str1.1 ,"aMS",@progbits,0x1
L006:
.string ""
.string ""
.string ""
.string "%"
.string ""
.string ""
.string "s"
.string ""
L005:
.string ""
.string ""
.string ""
.string "%"
.string ""
.string ""
.string "r"
.string ""
L004:
.string "hogehoge"
