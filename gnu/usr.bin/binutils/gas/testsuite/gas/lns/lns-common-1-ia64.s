	.file 1 "foo.c"
	.loc 1 1
	.explicit
	{ .mii; nop 0; nop 0; nop 0 ;; }
	.loc 1 2 3
	{ .mii; nop 0; nop 0; nop 0 ;; }
	.loc 1 3 prologue_end
	{ .mii; nop 0; nop 0; nop 0 ;; }
	.loc 1 4 0 epilogue_begin
	{ .mii; nop 0; nop 0; nop 0 ;; }
	.loc 1 5 isa 1 basic_block
	{ .mii; nop 0; nop 0; nop 0 ;; }
	.loc 1 6 is_stmt 0
	{ .mii; nop 0; nop 0; nop 0 ;; }
	.loc 1 7 is_stmt 1
	{ .mii; nop 0; nop 0; nop 0 ;; }
	.loc 1 7 discriminator 1
	{ .mii; nop 0; nop 0; nop 0 ;; }
