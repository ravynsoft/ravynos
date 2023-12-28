# Check EVEX non-LIG instructions with with -mevexlig=256

	.allow_index_reg
	.text
_start:
	{evex} vmovd %xmm4,(%rcx)
	{evex} vmovd %xmm4,%ecx
	{evex} vmovd (%rcx),%xmm4
	{evex} vmovd %ecx,%xmm4

	{evex} vmovq %xmm4,(%rcx)
	{evex} vmovq %xmm4,%rcx
	{evex} vmovq (%rcx),%xmm4
	{evex} vmovq %rcx,%xmm4
	{evex} vmovq %xmm4,%xmm6

	{evex} vextractps $0, %xmm0, %eax
	{evex} vextractps $0, %xmm0, (%rax)

	{evex} vpextrb $0, %xmm0, %eax
	{evex} vpextrb $0, %xmm0, (%rax)

	{evex} vpextrw $0, %xmm0, %eax
	{evex} {store} vpextrw $0, %xmm0, %eax
	{evex} vpextrw $0, %xmm0, (%rax)

	{evex} vpextrd $0, %xmm0, %eax
	{evex} vpextrd $0, %xmm0, (%rax)

	{evex} vpextrq $0, %xmm0, %rax
	{evex} vpextrq $0, %xmm0, (%rax)

	{evex} vinsertps $0, %xmm0, %xmm0, %xmm0
	{evex} vinsertps $0, (%rax), %xmm0, %xmm0

	{evex} vpinsrb $0, %eax, %xmm0, %xmm0
	{evex} vpinsrb $0, (%rax), %xmm0, %xmm0

	{evex} vpinsrw $0, %eax, %xmm0, %xmm0
	{evex} vpinsrw $0, (%rax), %xmm0, %xmm0

	{evex} vpinsrd $0, %eax, %xmm0, %xmm0
	{evex} vpinsrd $0, (%rax), %xmm0, %xmm0

	{evex} vpinsrq $0, %rax, %xmm0, %xmm0
	{evex} vpinsrq $0, (%rax), %xmm0, %xmm0
