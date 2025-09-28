# Check VEX non-LIG instructions with with -mavxscalar=256

	.allow_index_reg
	.text
_start:
	       vmovd	%eax, %xmm0
	       vmovd	(%eax), %xmm0
	{vex3} vmovd	%eax, %xmm0
	{vex3} vmovd	(%eax), %xmm0

	       vmovd	%xmm0, %eax
	       vmovd	%xmm0, (%eax)
	{vex3} vmovd	%xmm0, %eax
	{vex3} vmovd	%xmm0, (%eax)

	       vmovq	%xmm0, %xmm0
	       vmovq	(%eax), %xmm0
	{vex3} vmovq	%xmm0, %xmm0
	{vex3} vmovq	(%eax), %xmm0

	{store} vmovq	%xmm0, %xmm0
	        vmovq	%xmm0, (%eax)
	{vex3} {store} vmovq %xmm0, %xmm0
	{vex3} vmovq	%xmm0, (%eax)

	       vextractps $0, %xmm0, %eax
	       vextractps $0, %xmm0, (%eax)

	       vpextrb $0, %xmm0, %eax
	       vpextrb $0, %xmm0, (%eax)

	       vpextrw $0, %xmm0, %eax
	{vex3} vpextrw $0, %xmm0, %eax
	{store} vpextrw $0, %xmm0, %eax
	       vpextrw $0, %xmm0, (%eax)

	       vpextrd $0, %xmm0, %eax
	       vpextrd $0, %xmm0, (%eax)

	       vinsertps $0, %xmm0, %xmm0, %xmm0
	       vinsertps $0, (%eax), %xmm0, %xmm0

	       vpinsrb $0, %eax, %xmm0, %xmm0
	       vpinsrb $0, (%eax), %xmm0, %xmm0

	       vpinsrw $0, %eax, %xmm0, %xmm0
	       vpinsrw $0, (%eax), %xmm0, %xmm0
	{vex3} vpinsrw $0, %eax, %xmm0, %xmm0
	{vex3} vpinsrw $0, (%eax), %xmm0, %xmm0

	       vpinsrd $0, %eax, %xmm0, %xmm0
	       vpinsrd $0, (%eax), %xmm0, %xmm0

	       vldmxcsr (%eax)
	       vstmxcsr (%eax)
	{vex3} vldmxcsr (%eax)
	{vex3} vstmxcsr (%eax)

	andn	(%eax), %eax, %eax
	bextr	%eax, (%eax), %eax
	blsi	(%eax), %eax
	blsmsk	(%eax), %eax
	blsr	(%eax), %eax

	bzhi	%eax, (%eax), %eax
	mulx	(%eax), %eax, %eax
	pdep	(%eax), %eax, %eax
	pext	(%eax), %eax, %eax
	rorx	$0, (%eax), %eax
	sarx	%eax, (%eax), %eax
	shlx	%eax, (%eax), %eax
	shrx	%eax, (%eax), %eax

	bextr	$0, (%eax), %eax
	blcfill	(%eax), %eax
	blci	(%eax), %eax
	blcic	(%eax), %eax
	blcmsk	(%eax), %eax
	blcs	(%eax), %eax
	blsfill	(%eax), %eax
	blsic	(%eax), %eax
	t1mskc	(%eax), %eax
	tzmsk	(%eax), %eax
