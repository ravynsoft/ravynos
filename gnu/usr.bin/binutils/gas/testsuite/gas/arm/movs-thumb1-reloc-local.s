.arch armv6s-m
.text
.syntax unified
.thumb
foo:
movs r0, #:upper8_15:#bar
movs r0, #:upper0_7:#bar
movs r0, #:lower8_15:#bar
movs r0, #:lower0_7:#bar

.space 0x10000

bar:
