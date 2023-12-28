	.section .text.t1
	.global _start
	.global end
_start:
	bl	end

	.section .text.t2
	.zero 64 * 1024 * 1024

	.section .text.t3
	.zero 64 * 1024 * 1024

	.section .text.t4
end:
	bl _start
