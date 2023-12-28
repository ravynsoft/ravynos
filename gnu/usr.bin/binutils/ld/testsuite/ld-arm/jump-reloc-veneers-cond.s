	.text
	.syntax unified
	.thumb_func
	.global _start
	.type _start,%function
_start:
	bne dest

	.section destsect, "x"
	.thumb_func
dest:
	bl dest
