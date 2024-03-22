@localsize 1, 1, 1
@buf 4  ; g[0]
@const(c0.x)  0.0, 0.0, 0.0, 0.0
@const(c1.x)  1.0, 2.0, 3.0, 4.0
@wgid(r48.x)        ; r48.xyz
@invocationid(r0.x) ; r0.xyz
@numwg(c2.x)        ; c2.xyz
mov.f32f32 r2.x, c0.y
mov.u32u32 r0.x, 0x12345678
mov.u32u32 r0.y, 0x12345678
mov.u32u32 r0.z, 0x12345678
add.u r2.x, c0.x, r2.x
mov.u32u32 r0.w, 0x12345678
mov.u32u32 r1.x, 0x12345678
mov.u32u32 r1.y, 0x12345678
cov.u32s16 hr4.x, r2.x
mov.u32u32 r1.z, 0x12345678
mov.u32u32 r1.w, 0x12345678
nop
mova a0.x, hr4.x
(rpt5)nop
(ul)mov.u32u32 r0.x, r<a0.x>
mov.u32u32 r0.y, 0x00000000
(rpt5)nop
stib.b.untyped.1d.u32.1.imm r0.x, r0.y, 0
end
nop

