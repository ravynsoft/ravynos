 .text

_start:
	rex/fxsave (%rax)
	rex64/fxsave (%rax)
	rex/fxsave (%r8)
	rex64/fxsave (%r8)
	rex/fxsave (,%r8)
	rex64/fxsave (,%r8)
	rex/fxsave (%r8,%r8)
	rex64/fxsave (%r8,%r8)

	rex.w add	(%rax,%rax), %eax
	rex.r add	(%rax,%rax), %eax
	rex.b add	(%rax,%rax), %eax
	rex.x add	(%rax,%rax), %eax

	rex.w add	(%r8,%rax), %eax
	rex.r add	(%rax,%r8), %eax
	rex.b add	(%rax,%rax), %r8d
	rex.x add	(%rax,%rax), %rax

	rex nop
	rex.w nop
	rex.r nop
	rex.x nop
	rex.b nop

	.byte 0x41,0x9b,0xdd,0x30
	fsave (%r8)

	.byte 0x40
	vmovapd (%rax),%xmm0

# Test prefixes family.
	rex
	rex.B
	rex.X
	rex.XB
	rex.R
	rex.RB
	rex.RX
	rex.RXB
	rex.W
	rex.WB
	rex.WX
	rex.WXB
	rex.WR
	rex.WRB
	rex.WRX
	rex.WRXB
# Make sure that the above rex prefix won't become the rex prefix for
# the padding.
	rex
