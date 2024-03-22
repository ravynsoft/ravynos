@localsize 16, 1, 1
@buf 16  ; g[0]
@invocationid(r0.x) ; r0.xyz
@branchstack 1
cmps.u.gt p0.x, r0.x, 0
mov.u32u32 r1.x, 0x87654321
(rpt5)nop
br !p0.x, #endif
mov.u32u32 r1.x, 0x12345678
endif:
(jp)(rpt5)nop
stib.b.untyped.1d.u32.1.imm r1.x, r0.x, 0
end
nop

