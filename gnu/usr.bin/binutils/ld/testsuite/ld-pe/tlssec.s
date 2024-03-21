.global _tls_used
.global __tls_used
.global _start
.global start
.global _mainCRTStartup
.global mainCRTStartup

.text
_start:
mainCRTStartup:
_mainCRTStartup:
        .byte 1

.section .tls
_tls_used:
__tls_used:
.long 1,2,3,4,5,6,7,8,9,10
.long 11,12,13,14,15,16,17,18,19,20
.long 21,22,23,24,25,26,27,28,29,30

