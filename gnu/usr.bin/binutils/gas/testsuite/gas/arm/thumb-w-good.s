.thumb
.syntax unified
.arch armv4t
bl.w foo
.arch armv6-m
mrs.w r0, apsr

.arch armv6t2
push.w {r0}
push.w {r0, lr}
pop.w {r0}
pop.w {r0, pc}
