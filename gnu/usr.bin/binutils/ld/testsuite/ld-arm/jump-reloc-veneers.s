	.text
	.syntax unified
	.thumb_func
	.global _start
	.type _start,%function
_start:
	b.w dest

	.section destsect, "x"
	.thumb_func
dest:
	b.n dest
