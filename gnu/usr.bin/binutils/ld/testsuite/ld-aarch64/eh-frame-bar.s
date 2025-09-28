__longjmp:
 .cfi_startproc
 .cfi_def_cfa x0, 0
 .cfi_offset x19, 16
 .cfi_offset x20, 16
 .cfi_offset x21, 16
 .cfi_offset x22, 16
 .cfi_offset x23, 24
 .cfi_offset x24, 24
 .cfi_offset x25, 24
 .cfi_offset x26, 24
 .cfi_offset x27, 24
 .cfi_offset x28, 32
 .cfi_offset x29, 32
 .cfi_offset x30, 40
 .cfi_offset d9, 8
# This eh frame data differs from eh-frame-bar.s here, see the comment
# in eh-frame-foo.s
 .cfi_offset d11, 8

 ldp x19, x20, [x0, #16]
 ldp x21, x22, [x0, #16]
 ldp x23, x24, [x0, #24]
 ldp x25, x26, [x0, #24]
 ldp x27, x28, [x0, #24]
 ldp x29, x30, [x0, #32]

 ldp d8, d9, [x0, #8]
 ldp d10, d11, [x0, #8]
 ldp d12, d13, [x0, #8]
 ldp d14, d15, [x0, #8]
 ldr x5, [x0, #48]
 mov sp, x5
 cmp x1, #0
 mov x0, #1
 csel x0, x1, x0, ne
 br x30
 .cfi_endproc
