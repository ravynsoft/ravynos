.globl cc

.text
cc:
	addi.d	$r3,$r3,-16
	st.d	$r1,$r3,8
	la.local $r5,.LC0
	addi.w $r4,$r0,0
	addi.w $r6, $r0,12
	addi.w $r11, $r0, 64
	syscall 0
	ld.d	$r1,$r3,8
	addi.d	$r3,$r3,16
	jirl $r0, $r1, 0
.LC0:
	.ascii	"hello world\012\000"
	.text
	.align	2
	.globl	world
	.type	world, @function
