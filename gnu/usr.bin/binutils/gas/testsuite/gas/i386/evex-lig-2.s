# Check EVEX non-LIG instructions with with -mevexlig=256

	.allow_index_reg
	.text
_start:
	{evex} vmovd %xmm4,(%ecx)
	{evex} vmovd %xmm4,%ecx
	{evex} vmovd (%ecx),%xmm4
	{evex} vmovd %ecx,%xmm4

	{evex} vmovq %xmm4,(%ecx)
	{evex} vmovq (%ecx),%xmm4

	{evex} vmovq %xmm4,%xmm6

	{evex} vextractps $0, %xmm0, %eax
	{evex} vextractps $0, %xmm0, (%eax)

	{evex} vpextrb $0, %xmm0, %eax
	{evex} vpextrb $0, %xmm0, (%eax)

	{evex} vpextrw $0, %xmm0, %eax
	{evex} {store} vpextrw $0, %xmm0, %eax
	{evex} vpextrw $0, %xmm0, (%eax)

	{evex} vpextrd $0, %xmm0, %eax
	{evex} vpextrd $0, %xmm0, (%eax)

	{evex} vinsertps $0, %xmm0, %xmm0, %xmm0
	{evex} vinsertps $0, (%eax), %xmm0, %xmm0

	{evex} vpinsrb $0, %eax, %xmm0, %xmm0
	{evex} vpinsrb $0, (%eax), %xmm0, %xmm0

	{evex} vpinsrw $0, %eax, %xmm0, %xmm0
	{evex} vpinsrw $0, (%eax), %xmm0, %xmm0

	{evex} vpinsrd $0, %eax, %xmm0, %xmm0
	{evex} vpinsrd $0, (%eax), %xmm0, %xmm0
