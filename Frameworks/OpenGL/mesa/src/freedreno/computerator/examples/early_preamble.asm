@localsize 1, 1, 1
@buf 4  ; g[0]
@invocationid(r0.x) ; r0.xyz
@const(c0.x)  0.0, 0.0, 0.0, 0.0
@earlypreamble

shps #l_preamble_end
getone #l_preamble_end

mov.u32u32 r48.x, 1
mov.u32u32 r48.y, 2
mov.u32u32 r48.z, 3
mov.u32u32 r48.w, 4
(rpt5)nop
stc.u32 c[0], r48.x, 4

(sy)(ss)shpe

l_preamble_end:
(jp)nop

(rpt3)mov.u32u32 r1.x, (r)c0.x
(rpt5)nop
stib.b.untyped.1d.u32.4.imm r1.x, r0.x, 0
end
