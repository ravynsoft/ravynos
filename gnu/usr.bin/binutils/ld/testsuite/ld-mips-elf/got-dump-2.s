	.global	glob
	.ent	glob
glob:
	ld	$4,%got_page(local)($28)
	daddiu	$4,$4,%got_ofst(local)
	ld	$4,%got_disp(hidden)($28)
	ld	$4,%call16(glob)($28)
	ld	$4,%call16(extern)($28)
	.end	glob

	.data
	.type	local,%object
	.size	local,8
local:
	.dword	undef

	.globl	hidden
	.hidden	hidden
	.type	hidden,%object
	.size	hidden,8
hidden:
	.dword	0
