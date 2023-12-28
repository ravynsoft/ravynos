.arch armv6s-m
.text
.syntax unified
.thumb
foo:
adds r0, #:upper8_15:#bar
adds r0, #:upper0_7:#bar
adds r0, #:lower8_15:#bar
adds r0, #:lower0_7:#bar

.space 0x10000

bar:
