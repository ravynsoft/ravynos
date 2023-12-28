.arch armv7-a
.text
.syntax unified
foo:
movw r0, #:lower16: bar
movt r0, #:upper16: bar
.thumb
movw r0, #:lower16: bar
movt r0, #:upper16: bar

.space 0x10000

bar:
