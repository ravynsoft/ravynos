@localsize 16, 1, 1
@buf 128 (c2.x)  ; c2.xy
@invocationid(r0.x) ; r0.xyz
mov.u32u32 r0.y, r0.x
mov.u32u32 r1.x, c2.x
mov.u32u32 r1.y, c2.y
mov.u32u32 r2.x, 0xff
(rpt5)nop
stg.a.u32 g[r1.x+r0.y<<4+2<<2], r2.x, 1
nop(sy)
ldg.a.u32 r4.x, g[r1.x+r0.y<<4+2<<2], 1
nop(sy)
add.u r4.x, r4.x, 1
(rpt3)nop
stg.a.u32 g[r1.x+r0.y<<4+1<<2], r4.x, 1
end
nop
