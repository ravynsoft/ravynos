@localsize 32, 1, 1
@buf 32  ; g[0]
@const(c0.x)  0.0, 0.0, 0.0, 0.0
@const(c1.x)  1.0, 2.0, 3.0, 4.0
@wgid(r48.x)        ; r48.xyz
@invocationid(r0.x) ; r0.xyz
@numwg(c2.x)        ; c2.xyz
mov.u32u32 r0.y, r0.x
(rpt5)nop
stib.b.untyped.1d.u32.1.imm r0.x, r0.y, 0
end
nop

