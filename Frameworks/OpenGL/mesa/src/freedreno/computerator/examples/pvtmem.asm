@localsize 1, 1, 1
@buf 4  ; g[0]
@pvtmem 4
mov.u32u32 r1.x, 0x12345678
mov.u32u32 r0.x, 0
(rpt5)nop
stp.u32 p[r0.x + 0], r1.x, 1
ldp.u32 r0.x, p[r0.x + 0], 1
mov.u32u32 r0.y, 0x00000000
(sy)(rpt5)nop
stib.b.untyped.1d.u32.1.imm r0.x, r0.y, 0
end
nop

