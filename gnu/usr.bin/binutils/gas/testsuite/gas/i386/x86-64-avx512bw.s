# Check 64bit AVX512BW instructions

	.allow_index_reg
	.text
_start:
	vpabsb	%zmm29, %zmm30	 # AVX512BW
	vpabsb	%zmm29, %zmm30{%k7}	 # AVX512BW
	vpabsb	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpabsb	(%rcx), %zmm30	 # AVX512BW
	vpabsb	0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpabsb	8128(%rdx), %zmm30	 # AVX512BW Disp8
	vpabsb	8192(%rdx), %zmm30	 # AVX512BW
	vpabsb	-8192(%rdx), %zmm30	 # AVX512BW Disp8
	vpabsb	-8256(%rdx), %zmm30	 # AVX512BW
	vpabsw	%zmm29, %zmm30	 # AVX512BW
	vpabsw	%zmm29, %zmm30{%k7}	 # AVX512BW
	vpabsw	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpabsw	(%rcx), %zmm30	 # AVX512BW
	vpabsw	0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpabsw	8128(%rdx), %zmm30	 # AVX512BW Disp8
	vpabsw	8192(%rdx), %zmm30	 # AVX512BW
	vpabsw	-8192(%rdx), %zmm30	 # AVX512BW Disp8
	vpabsw	-8256(%rdx), %zmm30	 # AVX512BW
	vpackssdw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpackssdw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpackssdw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpackssdw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpackssdw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpackssdw	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512BW
	vpackssdw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpackssdw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpackssdw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpackssdw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpackssdw	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512BW Disp8
	vpackssdw	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512BW
	vpackssdw	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512BW Disp8
	vpackssdw	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512BW
	vpacksswb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpacksswb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpacksswb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpacksswb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpacksswb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpacksswb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpacksswb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpacksswb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpacksswb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpackusdw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpackusdw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpackusdw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpackusdw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpackusdw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpackusdw	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512BW
	vpackusdw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpackusdw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpackusdw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpackusdw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpackusdw	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512BW Disp8
	vpackusdw	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512BW
	vpackusdw	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512BW Disp8
	vpackusdw	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512BW
	vpackuswb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpackuswb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpackuswb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpackuswb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpackuswb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpackuswb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpackuswb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpackuswb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpackuswb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpaddb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpaddb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpaddb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpaddb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpaddb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddsb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpaddsb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpaddsb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpaddsb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpaddsb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpaddsb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddsb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddsb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddsb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddsw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpaddsw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpaddsw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpaddsw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpaddsw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpaddsw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddsw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddsw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddsw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddusb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpaddusb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpaddusb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpaddusb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpaddusb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpaddusb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddusb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddusb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddusb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddusw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpaddusw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpaddusw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpaddusw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpaddusw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpaddusw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddusw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddusw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddusw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpaddw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpaddw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpaddw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpaddw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpaddw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpaddw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpaddw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpalignr	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512BW
	vpalignr	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpalignr	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpalignr	$123, %zmm28, %zmm29, %zmm30	 # AVX512BW
	vpalignr	$123, (%rcx), %zmm29, %zmm30	 # AVX512BW
	vpalignr	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpalignr	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpalignr	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpalignr	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpalignr	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpavgb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpavgb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpavgb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpavgb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpavgb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpavgb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpavgb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpavgb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpavgb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpavgw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpavgw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpavgw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpavgw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpavgw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpavgw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpavgw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpavgw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpavgw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpblendmb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpblendmb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpblendmb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpblendmb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpblendmb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpblendmb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpblendmb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpblendmb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpblendmb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpbroadcastb	%xmm29, %zmm30	 # AVX512BW
	vpbroadcastb	%xmm29, %zmm30{%k7}	 # AVX512BW
	vpbroadcastb	%xmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpbroadcastb	(%rcx), %zmm30	 # AVX512BW
	vpbroadcastb	0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpbroadcastb	127(%rdx), %zmm30	 # AVX512BW Disp8
	vpbroadcastb	128(%rdx), %zmm30	 # AVX512BW
	vpbroadcastb	-128(%rdx), %zmm30	 # AVX512BW Disp8
	vpbroadcastb	-129(%rdx), %zmm30	 # AVX512BW
	vpbroadcastb	%eax, %zmm30	 # AVX512BW
	vpbroadcastb	%eax, %zmm30{%k7}	 # AVX512BW
	vpbroadcastb	%eax, %zmm30{%k7}{z}	 # AVX512BW
	vpbroadcastw	%xmm29, %zmm30	 # AVX512BW
	vpbroadcastw	%xmm29, %zmm30{%k7}	 # AVX512BW
	vpbroadcastw	%xmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpbroadcastw	(%rcx), %zmm30	 # AVX512BW
	vpbroadcastw	0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpbroadcastw	254(%rdx), %zmm30	 # AVX512BW Disp8
	vpbroadcastw	256(%rdx), %zmm30	 # AVX512BW
	vpbroadcastw	-256(%rdx), %zmm30	 # AVX512BW Disp8
	vpbroadcastw	-258(%rdx), %zmm30	 # AVX512BW
	vpbroadcastw	%eax, %zmm30	 # AVX512BW
	vpbroadcastw	%eax, %zmm30{%k7}	 # AVX512BW
	vpbroadcastw	%eax, %zmm30{%k7}{z}	 # AVX512BW
	vpcmpeqb	%zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpeqb	%zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vpcmpeqb	(%rcx), %zmm30, %k5	 # AVX512BW
	vpcmpeqb	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vpcmpeqb	8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpeqb	8192(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpeqb	-8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpeqb	-8256(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpeqw	%zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpeqw	%zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vpcmpeqw	(%rcx), %zmm30, %k5	 # AVX512BW
	vpcmpeqw	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vpcmpeqw	8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpeqw	8192(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpeqw	-8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpeqw	-8256(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpgtb	%zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpgtb	%zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vpcmpgtb	(%rcx), %zmm30, %k5	 # AVX512BW
	vpcmpgtb	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vpcmpgtb	8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpgtb	8192(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpgtb	-8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpgtb	-8256(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpgtw	%zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpgtw	%zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vpcmpgtw	(%rcx), %zmm30, %k5	 # AVX512BW
	vpcmpgtw	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vpcmpgtw	8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpgtw	8192(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpgtw	-8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpgtw	-8256(%rdx), %zmm30, %k5	 # AVX512BW
	vpblendmw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpblendmw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpblendmw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpblendmw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpblendmw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpblendmw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpblendmw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpblendmw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpblendmw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpextrb	$0xab, %xmm29, %eax	 # AVX512BW
	vpextrb	$123, %xmm29, %rax	 # AVX512BW
	vpextrb	$123, %xmm29, %r8	 # AVX512BW
	vpextrb	$123, %xmm29, (%rcx)	 # AVX512BW
	vpextrb	$123, %xmm29, 0x123(%rax,%r14,8)	 # AVX512BW
	vpextrb	$123, %xmm29, 127(%rdx)	 # AVX512BW Disp8
	vpextrb	$123, %xmm29, 128(%rdx)	 # AVX512BW
	vpextrb	$123, %xmm29, -128(%rdx)	 # AVX512BW Disp8
	vpextrb	$123, %xmm29, -129(%rdx)	 # AVX512BW
	vpextrw	$123, %xmm29, (%rcx)	 # AVX512BW
	vpextrw	$123, %xmm29, 0x123(%rax,%r14,8)	 # AVX512BW
	vpextrw	$123, %xmm29, 254(%rdx)	 # AVX512BW Disp8
	vpextrw	$123, %xmm29, 256(%rdx)	 # AVX512BW
	vpextrw	$123, %xmm29, -256(%rdx)	 # AVX512BW Disp8
	vpextrw	$123, %xmm29, -258(%rdx)	 # AVX512BW
	vpextrw	$0xab, %xmm30, %eax	 # AVX512BW
	vpextrw	$123, %xmm30, %rax	 # AVX512BW
	vpextrw	$123, %xmm30, %r8	 # AVX512BW
	vpinsrb	$0xab, %eax, %xmm29, %xmm30	 # AVX512BW
	vpinsrb	$123, %rax, %xmm29, %xmm30	 # AVX512BW
	vpinsrb	$123, %ebp, %xmm29, %xmm30	 # AVX512BW
	vpinsrb	$123, %r13, %xmm29, %xmm30	 # AVX512BW
	vpinsrb	$123, (%rcx), %xmm29, %xmm30	 # AVX512BW
	vpinsrb	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512BW
	vpinsrb	$123, 127(%rdx), %xmm29, %xmm30	 # AVX512BW Disp8
	vpinsrb	$123, 128(%rdx), %xmm29, %xmm30	 # AVX512BW
	vpinsrb	$123, -128(%rdx), %xmm29, %xmm30	 # AVX512BW Disp8
	vpinsrb	$123, -129(%rdx), %xmm29, %xmm30	 # AVX512BW
	vpinsrw	$0xab, %eax, %xmm29, %xmm30	 # AVX512BW
	vpinsrw	$123, %rax, %xmm29, %xmm30	 # AVX512BW
	vpinsrw	$123, %ebp, %xmm29, %xmm30	 # AVX512BW
	vpinsrw	$123, %r13, %xmm29, %xmm30	 # AVX512BW
	vpinsrw	$123, (%rcx), %xmm29, %xmm30	 # AVX512BW
	vpinsrw	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512BW
	vpinsrw	$123, 254(%rdx), %xmm29, %xmm30	 # AVX512BW Disp8
	vpinsrw	$123, 256(%rdx), %xmm29, %xmm30	 # AVX512BW
	vpinsrw	$123, -256(%rdx), %xmm29, %xmm30	 # AVX512BW Disp8
	vpinsrw	$123, -258(%rdx), %xmm29, %xmm30	 # AVX512BW
	vpmaddubsw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmaddubsw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmaddubsw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmaddubsw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmaddubsw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmaddubsw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaddubsw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaddubsw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaddubsw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaddwd	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmaddwd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmaddwd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmaddwd	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmaddwd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmaddwd	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaddwd	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaddwd	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaddwd	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaxsb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmaxsb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmaxsb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmaxsb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmaxsb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmaxsb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaxsb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaxsb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaxsb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaxsw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmaxsw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmaxsw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmaxsw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmaxsw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmaxsw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaxsw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaxsw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaxsw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaxub	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmaxub	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmaxub	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmaxub	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmaxub	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmaxub	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaxub	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaxub	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaxub	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaxuw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmaxuw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmaxuw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmaxuw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmaxuw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmaxuw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaxuw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmaxuw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmaxuw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpminsb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpminsb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpminsb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpminsb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpminsb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpminsb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpminsb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpminsb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpminsb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpminsw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpminsw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpminsw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpminsw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpminsw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpminsw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpminsw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpminsw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpminsw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpminub	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpminub	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpminub	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpminub	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpminub	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpminub	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpminub	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpminub	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpminub	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpminuw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpminuw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpminuw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpminuw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpminuw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpminuw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpminuw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpminuw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpminuw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmovsxbw	%ymm29, %zmm30	 # AVX512BW
	vpmovsxbw	%ymm29, %zmm30{%k7}	 # AVX512BW
	vpmovsxbw	%ymm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmovsxbw	(%rcx), %zmm30	 # AVX512BW
	vpmovsxbw	0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpmovsxbw	4064(%rdx), %zmm30	 # AVX512BW Disp8
	vpmovsxbw	4096(%rdx), %zmm30	 # AVX512BW
	vpmovsxbw	-4096(%rdx), %zmm30	 # AVX512BW Disp8
	vpmovsxbw	-4128(%rdx), %zmm30	 # AVX512BW
	vpmovzxbw	%ymm29, %zmm30	 # AVX512BW
	vpmovzxbw	%ymm29, %zmm30{%k7}	 # AVX512BW
	vpmovzxbw	%ymm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmovzxbw	(%rcx), %zmm30	 # AVX512BW
	vpmovzxbw	0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpmovzxbw	4064(%rdx), %zmm30	 # AVX512BW Disp8
	vpmovzxbw	4096(%rdx), %zmm30	 # AVX512BW
	vpmovzxbw	-4096(%rdx), %zmm30	 # AVX512BW Disp8
	vpmovzxbw	-4128(%rdx), %zmm30	 # AVX512BW
	vpmulhrsw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmulhrsw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmulhrsw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmulhrsw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmulhrsw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmulhrsw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmulhrsw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmulhrsw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmulhrsw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmulhuw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmulhuw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmulhuw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmulhuw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmulhuw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmulhuw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmulhuw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmulhuw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmulhuw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmulhw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmulhw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmulhw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmulhw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmulhw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmulhw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmulhw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmulhw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmulhw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmullw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpmullw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpmullw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpmullw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpmullw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpmullw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmullw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmullw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpmullw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsadbw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsadbw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsadbw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsadbw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsadbw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsadbw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsadbw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpshufb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpshufb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpshufb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpshufb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpshufb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpshufb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpshufb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpshufb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpshufb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpshufhw	$0xab, %zmm29, %zmm30	 # AVX512BW
	vpshufhw	$0xab, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpshufhw	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpshufhw	$123, %zmm29, %zmm30	 # AVX512BW
	vpshufhw	$123, (%rcx), %zmm30	 # AVX512BW
	vpshufhw	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpshufhw	$123, 8128(%rdx), %zmm30	 # AVX512BW Disp8
	vpshufhw	$123, 8192(%rdx), %zmm30	 # AVX512BW
	vpshufhw	$123, -8192(%rdx), %zmm30	 # AVX512BW Disp8
	vpshufhw	$123, -8256(%rdx), %zmm30	 # AVX512BW
	vpshuflw	$0xab, %zmm29, %zmm30	 # AVX512BW
	vpshuflw	$0xab, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpshuflw	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpshuflw	$123, %zmm29, %zmm30	 # AVX512BW
	vpshuflw	$123, (%rcx), %zmm30	 # AVX512BW
	vpshuflw	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpshuflw	$123, 8128(%rdx), %zmm30	 # AVX512BW Disp8
	vpshuflw	$123, 8192(%rdx), %zmm30	 # AVX512BW
	vpshuflw	$123, -8192(%rdx), %zmm30	 # AVX512BW Disp8
	vpshuflw	$123, -8256(%rdx), %zmm30	 # AVX512BW
	vpsllw	%xmm28, %zmm29, %zmm30	 # AVX512BW
	vpsllw	%xmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsllw	%xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsllw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsllw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsllw	2032(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsllw	2048(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsllw	-2048(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsllw	-2064(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsraw	%xmm28, %zmm29, %zmm30	 # AVX512BW
	vpsraw	%xmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsraw	%xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsraw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsraw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsraw	2032(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsraw	2048(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsraw	-2048(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsraw	-2064(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsrlw	%xmm28, %zmm29, %zmm30	 # AVX512BW
	vpsrlw	%xmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsrlw	%xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsrlw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsrlw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsrlw	2032(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsrlw	2048(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsrlw	-2048(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsrlw	-2064(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsrldq	$0xab, %zmm29, %zmm30	 # AVX512BW
	vpsrldq	$123, %zmm29, %zmm30	 # AVX512BW
	vpsrldq	$123, (%rcx), %zmm30	 # AVX512BW
	vpsrldq	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpsrldq	$123, 8128(%rdx), %zmm30	 # AVX512BW Disp8
	vpsrldq	$123, 8192(%rdx), %zmm30	 # AVX512BW
	vpsrldq	$123, -8192(%rdx), %zmm30	 # AVX512BW Disp8
	vpsrldq	$123, -8256(%rdx), %zmm30	 # AVX512BW
	vpsrlw	$0xab, %zmm29, %zmm30	 # AVX512BW
	vpsrlw	$0xab, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsrlw	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsrlw	$123, %zmm29, %zmm30	 # AVX512BW
	vpsrlw	$123, (%rcx), %zmm30	 # AVX512BW
	vpsrlw	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpsrlw	$123, 8128(%rdx), %zmm30	 # AVX512BW Disp8
	vpsrlw	$123, 8192(%rdx), %zmm30	 # AVX512BW
	vpsrlw	$123, -8192(%rdx), %zmm30	 # AVX512BW Disp8
	vpsrlw	$123, -8256(%rdx), %zmm30	 # AVX512BW
	vpsraw	$0xab, %zmm29, %zmm30	 # AVX512BW
	vpsraw	$0xab, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsraw	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsraw	$123, %zmm29, %zmm30	 # AVX512BW
	vpsraw	$123, (%rcx), %zmm30	 # AVX512BW
	vpsraw	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpsraw	$123, 8128(%rdx), %zmm30	 # AVX512BW Disp8
	vpsraw	$123, 8192(%rdx), %zmm30	 # AVX512BW
	vpsraw	$123, -8192(%rdx), %zmm30	 # AVX512BW Disp8
	vpsraw	$123, -8256(%rdx), %zmm30	 # AVX512BW
	vpsrlvw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsrlvw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsrlvw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsrlvw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsrlvw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsrlvw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsrlvw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsrlvw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsrlvw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsravw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsravw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsravw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsravw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsravw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsravw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsravw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsravw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsravw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsubb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsubb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsubb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsubb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsubb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubsb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsubsb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsubsb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsubsb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsubsb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsubsb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubsb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubsb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubsb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubsw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsubsw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsubsw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsubsw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsubsw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsubsw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubsw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubsw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubsw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubusb	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsubusb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsubusb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsubusb	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsubusb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsubusb	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubusb	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubusb	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubusb	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubusw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsubusw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsubusw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsubusw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsubusw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsubusw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubusw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubusw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubusw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsubw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsubw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsubw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsubw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsubw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsubw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsubw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpunpckhbw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpunpckhbw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpunpckhbw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpunpckhbw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpunpckhbw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpunpckhbw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpunpckhbw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpunpckhbw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpunpckhbw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpunpckhwd	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpunpckhwd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpunpckhwd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpunpckhwd	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpunpckhwd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpunpckhwd	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpunpckhwd	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpunpckhwd	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpunpckhwd	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpunpcklbw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpunpcklbw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpunpcklbw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpunpcklbw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpunpcklbw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpunpcklbw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpunpcklbw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpunpcklbw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpunpcklbw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpunpcklwd	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpunpcklwd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpunpcklwd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpunpcklwd	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpunpcklwd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpunpcklwd	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpunpcklwd	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpunpcklwd	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpunpcklwd	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpmovwb	%zmm29, %ymm30	 # AVX512BW
	vpmovwb	%zmm29, %ymm30{%k7}	 # AVX512BW
	vpmovwb	%zmm29, %ymm30{%k7}{z}	 # AVX512BW
	vpmovswb	%zmm29, %ymm30	 # AVX512BW
	vpmovswb	%zmm29, %ymm30{%k7}	 # AVX512BW
	vpmovswb	%zmm29, %ymm30{%k7}{z}	 # AVX512BW
	vpmovuswb	%zmm29, %ymm30	 # AVX512BW
	vpmovuswb	%zmm29, %ymm30{%k7}	 # AVX512BW
	vpmovuswb	%zmm29, %ymm30{%k7}{z}	 # AVX512BW
	vdbpsadbw	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512BW
	vdbpsadbw	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vdbpsadbw	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vdbpsadbw	$123, %zmm28, %zmm29, %zmm30	 # AVX512BW
	vdbpsadbw	$123, (%rcx), %zmm29, %zmm30	 # AVX512BW
	vdbpsadbw	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vdbpsadbw	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vdbpsadbw	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vdbpsadbw	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vdbpsadbw	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpermw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpermw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpermw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpermw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpermw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpermw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpermw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpermw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpermw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpermt2w	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpermt2w	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpermt2w	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpermt2w	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpermt2w	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpermt2w	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpermt2w	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpermt2w	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpermt2w	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpslldq	$0xab, %zmm29, %zmm30	 # AVX512BW
	vpslldq	$123, %zmm29, %zmm30	 # AVX512BW
	vpslldq	$123, (%rcx), %zmm30	 # AVX512BW
	vpslldq	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpslldq	$123, 8128(%rdx), %zmm30	 # AVX512BW Disp8
	vpslldq	$123, 8192(%rdx), %zmm30	 # AVX512BW
	vpslldq	$123, -8192(%rdx), %zmm30	 # AVX512BW Disp8
	vpslldq	$123, -8256(%rdx), %zmm30	 # AVX512BW
	vpsllw	$0xab, %zmm29, %zmm30	 # AVX512BW
	vpsllw	$0xab, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsllw	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsllw	$123, %zmm29, %zmm30	 # AVX512BW
	vpsllw	$123, (%rcx), %zmm30	 # AVX512BW
	vpsllw	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vpsllw	$123, 8128(%rdx), %zmm30	 # AVX512BW Disp8
	vpsllw	$123, 8192(%rdx), %zmm30	 # AVX512BW
	vpsllw	$123, -8192(%rdx), %zmm30	 # AVX512BW Disp8
	vpsllw	$123, -8256(%rdx), %zmm30	 # AVX512BW
	vpsllvw	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpsllvw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpsllvw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpsllvw	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpsllvw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpsllvw	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsllvw	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpsllvw	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpsllvw	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vmovdqu8	%zmm29, %zmm30	 # AVX512BW
	vmovdqu8	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu8	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vmovdqu8	(%rcx), %zmm30	 # AVX512BW
	vmovdqu8	0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vmovdqu8	8128(%rdx), %zmm30	 # AVX512BW Disp8
	vmovdqu8	8192(%rdx), %zmm30	 # AVX512BW
	vmovdqu8	-8192(%rdx), %zmm30	 # AVX512BW Disp8
	vmovdqu8	-8256(%rdx), %zmm30	 # AVX512BW
	vmovdqu16	%zmm29, %zmm30	 # AVX512BW
	vmovdqu16	%zmm29, %zmm30{%k7}	 # AVX512BW
	vmovdqu16	%zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vmovdqu16	(%rcx), %zmm30	 # AVX512BW
	vmovdqu16	0x123(%rax,%r14,8), %zmm30	 # AVX512BW
	vmovdqu16	8128(%rdx), %zmm30	 # AVX512BW Disp8
	vmovdqu16	8192(%rdx), %zmm30	 # AVX512BW
	vmovdqu16	-8192(%rdx), %zmm30	 # AVX512BW Disp8
	vmovdqu16	-8256(%rdx), %zmm30	 # AVX512BW
	kandq	%k7, %k6, %k5	 # AVX512BW
	kandd	%k7, %k6, %k5	 # AVX512BW
	kandnq	%k7, %k6, %k5	 # AVX512BW
	kandnd	%k7, %k6, %k5	 # AVX512BW
	korq	%k7, %k6, %k5	 # AVX512BW
	kord	%k7, %k6, %k5	 # AVX512BW
	kxnorq	%k7, %k6, %k5	 # AVX512BW
	kxnord	%k7, %k6, %k5	 # AVX512BW
	kxorq	%k7, %k6, %k5	 # AVX512BW
	kxord	%k7, %k6, %k5	 # AVX512BW
	knotq	%k6, %k5	 # AVX512BW
	knotd	%k6, %k5	 # AVX512BW
	kortestq	%k6, %k5	 # AVX512BW
	kortestd	%k6, %k5	 # AVX512BW
	ktestq	%k6, %k5	 # AVX512BW
	ktestd	%k6, %k5	 # AVX512BW
	kshiftrq	$0xab, %k6, %k5	 # AVX512BW
	kshiftrq	$123, %k6, %k5	 # AVX512BW
	kshiftrd	$0xab, %k6, %k5	 # AVX512BW
	kshiftrd	$123, %k6, %k5	 # AVX512BW
	kshiftlq	$0xab, %k6, %k5	 # AVX512BW
	kshiftlq	$123, %k6, %k5	 # AVX512BW
	kshiftld	$0xab, %k6, %k5	 # AVX512BW
	kshiftld	$123, %k6, %k5	 # AVX512BW
	kmovq	%k6, %k5	 # AVX512BW
	kmovq	(%rcx), %k5	 # AVX512BW
	kmovq	0x123(%rax,%r14,8), %k5	 # AVX512BW
	kmovd	%k6, %k5	 # AVX512BW
	kmovd	(%rcx), %k5	 # AVX512BW
	kmovd	0x123(%rax,%r14,8), %k5	 # AVX512BW
	kmovq	%k5, (%rcx)	 # AVX512BW
	kmovq	%k5, 0x123(%rax,%r14,8)	 # AVX512BW
	kmovd	%k5, (%rcx)	 # AVX512BW
	kmovd	%k5, 0x123(%rax,%r14,8)	 # AVX512BW
	kmovq	%rax, %k5	 # AVX512BW
	kmovq	%r8, %k5	 # AVX512BW
	kmovd	%eax, %k5	 # AVX512BW
	kmovd	%ebp, %k5	 # AVX512BW
	kmovd	%r13d, %k5	 # AVX512BW
	kmovq	%k5, %rax	 # AVX512BW
	kmovq	%k5, %r8	 # AVX512BW
	kmovd	%k5, %eax	 # AVX512BW
	kmovd	%k5, %ebp	 # AVX512BW
	kmovd	%k5, %r13d	 # AVX512BW
	kaddq	%k7, %k6, %k5	 # AVX512BW
	kaddd	%k7, %k6, %k5	 # AVX512BW
	kunpckwd	%k7, %k6, %k5	 # AVX512BW
	kunpckdq	%k7, %k6, %k5	 # AVX512BW
	vpmovwb	%zmm30, (%rcx)	 # AVX512BW
	vpmovwb	%zmm30, (%rcx){%k7}	 # AVX512BW
	vpmovwb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512BW
	vpmovwb	%zmm30, 4064(%rdx)	 # AVX512BW Disp8
	vpmovwb	%zmm30, 4096(%rdx)	 # AVX512BW
	vpmovwb	%zmm30, -4096(%rdx)	 # AVX512BW Disp8
	vpmovwb	%zmm30, -4128(%rdx)	 # AVX512BW
	vpmovswb	%zmm30, (%rcx)	 # AVX512BW
	vpmovswb	%zmm30, (%rcx){%k7}	 # AVX512BW
	vpmovswb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512BW
	vpmovswb	%zmm30, 4064(%rdx)	 # AVX512BW Disp8
	vpmovswb	%zmm30, 4096(%rdx)	 # AVX512BW
	vpmovswb	%zmm30, -4096(%rdx)	 # AVX512BW Disp8
	vpmovswb	%zmm30, -4128(%rdx)	 # AVX512BW
	vpmovuswb	%zmm30, (%rcx)	 # AVX512BW
	vpmovuswb	%zmm30, (%rcx){%k7}	 # AVX512BW
	vpmovuswb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512BW
	vpmovuswb	%zmm30, 4064(%rdx)	 # AVX512BW Disp8
	vpmovuswb	%zmm30, 4096(%rdx)	 # AVX512BW
	vpmovuswb	%zmm30, -4096(%rdx)	 # AVX512BW Disp8
	vpmovuswb	%zmm30, -4128(%rdx)	 # AVX512BW
	vmovdqu8	%zmm30, (%rcx)	 # AVX512BW
	vmovdqu8	%zmm30, (%rcx){%k7}	 # AVX512BW
	vmovdqu8	%zmm30, 0x123(%rax,%r14,8)	 # AVX512BW
	vmovdqu8	%zmm30, 8128(%rdx)	 # AVX512BW Disp8
	vmovdqu8	%zmm30, 8192(%rdx)	 # AVX512BW
	vmovdqu8	%zmm30, -8192(%rdx)	 # AVX512BW Disp8
	vmovdqu8	%zmm30, -8256(%rdx)	 # AVX512BW
	vmovdqu16	%zmm30, (%rcx)	 # AVX512BW
	vmovdqu16	%zmm30, (%rcx){%k7}	 # AVX512BW
	vmovdqu16	%zmm30, 0x123(%rax,%r14,8)	 # AVX512BW
	vmovdqu16	%zmm30, 8128(%rdx)	 # AVX512BW Disp8
	vmovdqu16	%zmm30, 8192(%rdx)	 # AVX512BW
	vmovdqu16	%zmm30, -8192(%rdx)	 # AVX512BW Disp8
	vmovdqu16	%zmm30, -8256(%rdx)	 # AVX512BW
	vpermi2w	%zmm28, %zmm29, %zmm30	 # AVX512BW
	vpermi2w	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512BW
	vpermi2w	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512BW
	vpermi2w	(%rcx), %zmm29, %zmm30	 # AVX512BW
	vpermi2w	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512BW
	vpermi2w	8128(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpermi2w	8192(%rdx), %zmm29, %zmm30	 # AVX512BW
	vpermi2w	-8192(%rdx), %zmm29, %zmm30	 # AVX512BW Disp8
	vpermi2w	-8256(%rdx), %zmm29, %zmm30	 # AVX512BW
	vptestmb	%zmm29, %zmm30, %k5	 # AVX512BW
	vptestmb	%zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vptestmb	(%rcx), %zmm30, %k5	 # AVX512BW
	vptestmb	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vptestmb	8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vptestmb	8192(%rdx), %zmm30, %k5	 # AVX512BW
	vptestmb	-8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vptestmb	-8256(%rdx), %zmm30, %k5	 # AVX512BW
	vptestmw	%zmm29, %zmm30, %k5	 # AVX512BW
	vptestmw	%zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vptestmw	(%rcx), %zmm30, %k5	 # AVX512BW
	vptestmw	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vptestmw	8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vptestmw	8192(%rdx), %zmm30, %k5	 # AVX512BW
	vptestmw	-8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vptestmw	-8256(%rdx), %zmm30, %k5	 # AVX512BW
	vpmovb2m	%zmm30, %k5	 # AVX512BW
	vpmovw2m	%zmm30, %k5	 # AVX512BW
	vpmovm2b	%k5, %zmm30	 # AVX512BW
	vpmovm2w	%k5, %zmm30	 # AVX512BW
	vptestnmb	%zmm28, %zmm29, %k5	 # AVX512BW
	vptestnmb	%zmm28, %zmm29, %k5{%k7}	 # AVX512BW
	vptestnmb	(%rcx), %zmm29, %k5	 # AVX512BW
	vptestnmb	0x123(%rax,%r14,8), %zmm29, %k5	 # AVX512BW
	vptestnmb	8128(%rdx), %zmm29, %k5	 # AVX512BW Disp8
	vptestnmb	8192(%rdx), %zmm29, %k5	 # AVX512BW
	vptestnmb	-8192(%rdx), %zmm29, %k5	 # AVX512BW Disp8
	vptestnmb	-8256(%rdx), %zmm29, %k5	 # AVX512BW
	vptestnmw	%zmm28, %zmm29, %k5	 # AVX512BW
	vptestnmw	%zmm28, %zmm29, %k5{%k7}	 # AVX512BW
	vptestnmw	(%rcx), %zmm29, %k5	 # AVX512BW
	vptestnmw	0x123(%rax,%r14,8), %zmm29, %k5	 # AVX512BW
	vptestnmw	8128(%rdx), %zmm29, %k5	 # AVX512BW Disp8
	vptestnmw	8192(%rdx), %zmm29, %k5	 # AVX512BW
	vptestnmw	-8192(%rdx), %zmm29, %k5	 # AVX512BW Disp8
	vptestnmw	-8256(%rdx), %zmm29, %k5	 # AVX512BW
	vpcmpb	$0xab, %zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpb	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vpcmpb	$123, %zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpb	$123, (%rcx), %zmm30, %k5	 # AVX512BW
	vpcmpb	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vpcmpb	$123, 8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpb	$123, 8192(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpb	$123, -8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpb	$123, -8256(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpw	$0xab, %zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpw	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vpcmpw	$123, %zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpw	$123, (%rcx), %zmm30, %k5	 # AVX512BW
	vpcmpw	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vpcmpw	$123, 8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpw	$123, 8192(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpw	$123, -8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpw	$123, -8256(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpub	$0xab, %zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpub	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vpcmpub	$123, %zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpub	$123, (%rcx), %zmm30, %k5	 # AVX512BW
	vpcmpub	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vpcmpub	$123, 8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpub	$123, 8192(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpub	$123, -8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpub	$123, -8256(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpuw	$0xab, %zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpuw	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512BW
	vpcmpuw	$123, %zmm29, %zmm30, %k5	 # AVX512BW
	vpcmpuw	$123, (%rcx), %zmm30, %k5	 # AVX512BW
	vpcmpuw	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512BW
	vpcmpuw	$123, 8128(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpuw	$123, 8192(%rdx), %zmm30, %k5	 # AVX512BW
	vpcmpuw	$123, -8192(%rdx), %zmm30, %k5	 # AVX512BW Disp8
	vpcmpuw	$123, -8256(%rdx), %zmm30, %k5	 # AVX512BW

	.intel_syntax noprefix
	vpabsb	zmm30, zmm29	 # AVX512BW
	vpabsb	zmm30{k7}, zmm29	 # AVX512BW
	vpabsb	zmm30{k7}{z}, zmm29	 # AVX512BW
	vpabsb	zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vpabsb	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpabsb	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpabsb	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpabsb	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpabsb	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpabsw	zmm30, zmm29	 # AVX512BW
	vpabsw	zmm30{k7}, zmm29	 # AVX512BW
	vpabsw	zmm30{k7}{z}, zmm29	 # AVX512BW
	vpabsw	zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vpabsw	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpabsw	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpabsw	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpabsw	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpabsw	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpackssdw	zmm30, zmm29, zmm28	 # AVX512BW
	vpackssdw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpackssdw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpackssdw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpackssdw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpackssdw	zmm30, zmm29, [rcx]{1to16}	 # AVX512BW
	vpackssdw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpackssdw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpackssdw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpackssdw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpackssdw	zmm30, zmm29, [rdx+508]{1to16}	 # AVX512BW Disp8
	vpackssdw	zmm30, zmm29, [rdx+512]{1to16}	 # AVX512BW
	vpackssdw	zmm30, zmm29, [rdx-512]{1to16}	 # AVX512BW Disp8
	vpackssdw	zmm30, zmm29, [rdx-516]{1to16}	 # AVX512BW
	vpacksswb	zmm30, zmm29, zmm28	 # AVX512BW
	vpacksswb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpacksswb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpacksswb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpacksswb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpacksswb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpacksswb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpacksswb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpacksswb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpackusdw	zmm30, zmm29, zmm28	 # AVX512BW
	vpackusdw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpackusdw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpackusdw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpackusdw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpackusdw	zmm30, zmm29, [rcx]{1to16}	 # AVX512BW
	vpackusdw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpackusdw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpackusdw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpackusdw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpackusdw	zmm30, zmm29, [rdx+508]{1to16}	 # AVX512BW Disp8
	vpackusdw	zmm30, zmm29, [rdx+512]{1to16}	 # AVX512BW
	vpackusdw	zmm30, zmm29, [rdx-512]{1to16}	 # AVX512BW Disp8
	vpackusdw	zmm30, zmm29, [rdx-516]{1to16}	 # AVX512BW
	vpackuswb	zmm30, zmm29, zmm28	 # AVX512BW
	vpackuswb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpackuswb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpackuswb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpackuswb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpackuswb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpackuswb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpackuswb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpackuswb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpaddb	zmm30, zmm29, zmm28	 # AVX512BW
	vpaddb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpaddb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpaddb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpaddb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpaddb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpaddb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpaddb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpaddb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpaddsb	zmm30, zmm29, zmm28	 # AVX512BW
	vpaddsb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpaddsb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpaddsb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpaddsb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpaddsb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpaddsb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpaddsb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpaddsb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpaddsw	zmm30, zmm29, zmm28	 # AVX512BW
	vpaddsw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpaddsw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpaddsw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpaddsw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpaddsw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpaddsw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpaddsw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpaddsw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpaddusb	zmm30, zmm29, zmm28	 # AVX512BW
	vpaddusb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpaddusb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpaddusb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpaddusb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpaddusb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpaddusb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpaddusb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpaddusb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpaddusw	zmm30, zmm29, zmm28	 # AVX512BW
	vpaddusw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpaddusw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpaddusw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpaddusw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpaddusw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpaddusw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpaddusw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpaddusw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpaddw	zmm30, zmm29, zmm28	 # AVX512BW
	vpaddw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpaddw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpaddw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpaddw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpaddw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpaddw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpaddw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpaddw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpalignr	zmm30, zmm29, zmm28, 0xab	 # AVX512BW
	vpalignr	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512BW
	vpalignr	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512BW
	vpalignr	zmm30, zmm29, zmm28, 123	 # AVX512BW
	vpalignr	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpalignr	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpalignr	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpalignr	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpalignr	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpalignr	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpavgb	zmm30, zmm29, zmm28	 # AVX512BW
	vpavgb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpavgb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpavgb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpavgb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpavgb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpavgb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpavgb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpavgb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpavgw	zmm30, zmm29, zmm28	 # AVX512BW
	vpavgw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpavgw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpavgw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpavgw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpavgw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpavgw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpavgw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpavgw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpblendmb	zmm30, zmm29, zmm28	 # AVX512BW
	vpblendmb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpblendmb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpblendmb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpblendmb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpblendmb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpblendmb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpblendmb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpblendmb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpbroadcastb	zmm30, xmm29	 # AVX512BW
	vpbroadcastb	zmm30{k7}, xmm29	 # AVX512BW
	vpbroadcastb	zmm30{k7}{z}, xmm29	 # AVX512BW
	vpbroadcastb	zmm30, BYTE PTR [rcx]	 # AVX512BW
	vpbroadcastb	zmm30, BYTE PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpbroadcastb	zmm30, BYTE PTR [rdx+127]	 # AVX512BW Disp8
	vpbroadcastb	zmm30, BYTE PTR [rdx+128]	 # AVX512BW
	vpbroadcastb	zmm30, BYTE PTR [rdx-128]	 # AVX512BW Disp8
	vpbroadcastb	zmm30, BYTE PTR [rdx-129]	 # AVX512BW
	vpbroadcastb	zmm30, eax	 # AVX512BW
	vpbroadcastb	zmm30{k7}, eax	 # AVX512BW
	vpbroadcastb	zmm30{k7}{z}, eax	 # AVX512BW
	vpbroadcastw	zmm30, xmm29	 # AVX512BW
	vpbroadcastw	zmm30{k7}, xmm29	 # AVX512BW
	vpbroadcastw	zmm30{k7}{z}, xmm29	 # AVX512BW
	vpbroadcastw	zmm30, WORD PTR [rcx]	 # AVX512BW
	vpbroadcastw	zmm30, WORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpbroadcastw	zmm30, WORD PTR [rdx+254]	 # AVX512BW Disp8
	vpbroadcastw	zmm30, WORD PTR [rdx+256]	 # AVX512BW
	vpbroadcastw	zmm30, WORD PTR [rdx-256]	 # AVX512BW Disp8
	vpbroadcastw	zmm30, WORD PTR [rdx-258]	 # AVX512BW
	vpbroadcastw	zmm30, eax	 # AVX512BW
	vpbroadcastw	zmm30{k7}, eax	 # AVX512BW
	vpbroadcastw	zmm30{k7}{z}, eax	 # AVX512BW
	vpcmpeqb	k5, zmm30, zmm29	 # AVX512BW
	vpcmpeqb	k5{k7}, zmm30, zmm29	 # AVX512BW
	vpcmpeqb	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vpcmpeqb	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpcmpeqb	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpcmpeqb	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpcmpeqb	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpcmpeqb	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpcmpeqw	k5, zmm30, zmm29	 # AVX512BW
	vpcmpeqw	k5{k7}, zmm30, zmm29	 # AVX512BW
	vpcmpeqw	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vpcmpeqw	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpcmpeqw	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpcmpeqw	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpcmpeqw	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpcmpeqw	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpcmpgtb	k5, zmm30, zmm29	 # AVX512BW
	vpcmpgtb	k5{k7}, zmm30, zmm29	 # AVX512BW
	vpcmpgtb	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vpcmpgtb	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpcmpgtb	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpcmpgtb	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpcmpgtb	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpcmpgtb	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpcmpgtw	k5, zmm30, zmm29	 # AVX512BW
	vpcmpgtw	k5{k7}, zmm30, zmm29	 # AVX512BW
	vpcmpgtw	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vpcmpgtw	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpcmpgtw	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpcmpgtw	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpcmpgtw	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpcmpgtw	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpblendmw	zmm30, zmm29, zmm28	 # AVX512BW
	vpblendmw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpblendmw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpblendmw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpblendmw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpblendmw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpblendmw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpblendmw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpblendmw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpextrb	eax, xmm29, 0xab	 # AVX512BW
	vpextrb	rax, xmm29, 123	 # AVX512BW
	vpextrb	r8, xmm29, 123	 # AVX512BW
	vpextrb	BYTE PTR [rcx], xmm29, 123	 # AVX512BW
	vpextrb	BYTE PTR [rax+r14*8+0x1234], xmm29, 123	 # AVX512BW
	vpextrb	BYTE PTR [rdx+127], xmm29, 123	 # AVX512BW Disp8
	vpextrb	BYTE PTR [rdx+128], xmm29, 123	 # AVX512BW
	vpextrb	BYTE PTR [rdx-128], xmm29, 123	 # AVX512BW Disp8
	vpextrb	BYTE PTR [rdx-129], xmm29, 123	 # AVX512BW
	vpextrw	WORD PTR [rcx], xmm29, 123	 # AVX512BW
	vpextrw	WORD PTR [rax+r14*8+0x1234], xmm29, 123	 # AVX512BW
	vpextrw	WORD PTR [rdx+254], xmm29, 123	 # AVX512BW Disp8
	vpextrw	WORD PTR [rdx+256], xmm29, 123	 # AVX512BW
	vpextrw	WORD PTR [rdx-256], xmm29, 123	 # AVX512BW Disp8
	vpextrw	WORD PTR [rdx-258], xmm29, 123	 # AVX512BW
	vpextrw	eax, xmm30, 0xab	 # AVX512BW
	vpextrw	rax, xmm30, 123	 # AVX512BW
	vpextrw	r8, xmm30, 123	 # AVX512BW
	vpinsrb	xmm30, xmm29, eax, 0xab	 # AVX512BW
	vpinsrb	xmm30, xmm29, rax, 123	 # AVX512BW
	vpinsrb	xmm30, xmm29, ebp, 123	 # AVX512BW
	vpinsrb	xmm30, xmm29, r13, 123	 # AVX512BW
	vpinsrb	xmm30, xmm29, BYTE PTR [rcx], 123	 # AVX512BW
	vpinsrb	xmm30, xmm29, BYTE PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpinsrb	xmm30, xmm29, BYTE PTR [rdx+127], 123	 # AVX512BW Disp8
	vpinsrb	xmm30, xmm29, BYTE PTR [rdx+128], 123	 # AVX512BW
	vpinsrb	xmm30, xmm29, BYTE PTR [rdx-128], 123	 # AVX512BW Disp8
	vpinsrb	xmm30, xmm29, BYTE PTR [rdx-129], 123	 # AVX512BW
	vpinsrw	xmm30, xmm29, eax, 0xab	 # AVX512BW
	vpinsrw	xmm30, xmm29, rax, 123	 # AVX512BW
	vpinsrw	xmm30, xmm29, ebp, 123	 # AVX512BW
	vpinsrw	xmm30, xmm29, r13, 123	 # AVX512BW
	vpinsrw	xmm30, xmm29, WORD PTR [rcx], 123	 # AVX512BW
	vpinsrw	xmm30, xmm29, WORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpinsrw	xmm30, xmm29, WORD PTR [rdx+254], 123	 # AVX512BW Disp8
	vpinsrw	xmm30, xmm29, WORD PTR [rdx+256], 123	 # AVX512BW
	vpinsrw	xmm30, xmm29, WORD PTR [rdx-256], 123	 # AVX512BW Disp8
	vpinsrw	xmm30, xmm29, WORD PTR [rdx-258], 123	 # AVX512BW
	vpmaddubsw	zmm30, zmm29, zmm28	 # AVX512BW
	vpmaddubsw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmaddubsw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmaddubsw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmaddubsw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmaddubsw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmaddubsw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmaddubsw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmaddubsw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmaddwd	zmm30, zmm29, zmm28	 # AVX512BW
	vpmaddwd	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmaddwd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmaddwd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmaddwd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmaddwd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmaddwd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmaddwd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmaddwd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmaxsb	zmm30, zmm29, zmm28	 # AVX512BW
	vpmaxsb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmaxsb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmaxsb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmaxsb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmaxsb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmaxsb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmaxsb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmaxsb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmaxsw	zmm30, zmm29, zmm28	 # AVX512BW
	vpmaxsw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmaxsw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmaxsw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmaxsw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmaxsw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmaxsw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmaxsw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmaxsw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmaxub	zmm30, zmm29, zmm28	 # AVX512BW
	vpmaxub	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmaxub	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmaxub	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmaxub	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmaxub	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmaxub	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmaxub	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmaxub	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmaxuw	zmm30, zmm29, zmm28	 # AVX512BW
	vpmaxuw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmaxuw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmaxuw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmaxuw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmaxuw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmaxuw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmaxuw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmaxuw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpminsb	zmm30, zmm29, zmm28	 # AVX512BW
	vpminsb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpminsb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpminsb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpminsb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpminsb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpminsb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpminsb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpminsb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpminsw	zmm30, zmm29, zmm28	 # AVX512BW
	vpminsw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpminsw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpminsw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpminsw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpminsw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpminsw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpminsw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpminsw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpminub	zmm30, zmm29, zmm28	 # AVX512BW
	vpminub	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpminub	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpminub	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpminub	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpminub	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpminub	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpminub	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpminub	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpminuw	zmm30, zmm29, zmm28	 # AVX512BW
	vpminuw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpminuw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpminuw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpminuw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpminuw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpminuw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpminuw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpminuw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmovsxbw	zmm30, ymm29	 # AVX512BW
	vpmovsxbw	zmm30{k7}, ymm29	 # AVX512BW
	vpmovsxbw	zmm30{k7}{z}, ymm29	 # AVX512BW
	vpmovsxbw	zmm30, YMMWORD PTR [rcx]	 # AVX512BW
	vpmovsxbw	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmovsxbw	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512BW Disp8
	vpmovsxbw	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512BW
	vpmovsxbw	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512BW Disp8
	vpmovsxbw	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512BW
	vpmovzxbw	zmm30, ymm29	 # AVX512BW
	vpmovzxbw	zmm30{k7}, ymm29	 # AVX512BW
	vpmovzxbw	zmm30{k7}{z}, ymm29	 # AVX512BW
	vpmovzxbw	zmm30, YMMWORD PTR [rcx]	 # AVX512BW
	vpmovzxbw	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmovzxbw	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512BW Disp8
	vpmovzxbw	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512BW
	vpmovzxbw	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512BW Disp8
	vpmovzxbw	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512BW
	vpmulhrsw	zmm30, zmm29, zmm28	 # AVX512BW
	vpmulhrsw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmulhrsw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmulhrsw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmulhrsw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmulhrsw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmulhrsw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmulhrsw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmulhrsw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmulhuw	zmm30, zmm29, zmm28	 # AVX512BW
	vpmulhuw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmulhuw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmulhuw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmulhuw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmulhuw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmulhuw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmulhuw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmulhuw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmulhw	zmm30, zmm29, zmm28	 # AVX512BW
	vpmulhw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmulhw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmulhw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmulhw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmulhw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmulhw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmulhw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmulhw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmullw	zmm30, zmm29, zmm28	 # AVX512BW
	vpmullw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpmullw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpmullw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpmullw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpmullw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpmullw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpmullw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpmullw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpsadbw	zmm30, zmm29, zmm28	 # AVX512BW
	vpsadbw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsadbw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsadbw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsadbw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsadbw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsadbw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpshufb	zmm30, zmm29, zmm28	 # AVX512BW
	vpshufb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpshufb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpshufb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpshufb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpshufb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpshufb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpshufb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpshufb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpshufhw	zmm30, zmm29, 0xab	 # AVX512BW
	vpshufhw	zmm30{k7}, zmm29, 0xab	 # AVX512BW
	vpshufhw	zmm30{k7}{z}, zmm29, 0xab	 # AVX512BW
	vpshufhw	zmm30, zmm29, 123	 # AVX512BW
	vpshufhw	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpshufhw	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpshufhw	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpshufhw	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpshufhw	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpshufhw	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpshuflw	zmm30, zmm29, 0xab	 # AVX512BW
	vpshuflw	zmm30{k7}, zmm29, 0xab	 # AVX512BW
	vpshuflw	zmm30{k7}{z}, zmm29, 0xab	 # AVX512BW
	vpshuflw	zmm30, zmm29, 123	 # AVX512BW
	vpshuflw	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpshuflw	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpshuflw	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpshuflw	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpshuflw	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpshuflw	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpsllw	zmm30, zmm29, xmm28	 # AVX512BW
	vpsllw	zmm30{k7}, zmm29, xmm28	 # AVX512BW
	vpsllw	zmm30{k7}{z}, zmm29, xmm28	 # AVX512BW
	vpsllw	zmm30, zmm29, XMMWORD PTR [rcx]	 # AVX512BW
	vpsllw	zmm30, zmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsllw	zmm30, zmm29, XMMWORD PTR [rdx+2032]	 # AVX512BW Disp8
	vpsllw	zmm30, zmm29, XMMWORD PTR [rdx+2048]	 # AVX512BW
	vpsllw	zmm30, zmm29, XMMWORD PTR [rdx-2048]	 # AVX512BW Disp8
	vpsllw	zmm30, zmm29, XMMWORD PTR [rdx-2064]	 # AVX512BW
	vpsraw	zmm30, zmm29, xmm28	 # AVX512BW
	vpsraw	zmm30{k7}, zmm29, xmm28	 # AVX512BW
	vpsraw	zmm30{k7}{z}, zmm29, xmm28	 # AVX512BW
	vpsraw	zmm30, zmm29, XMMWORD PTR [rcx]	 # AVX512BW
	vpsraw	zmm30, zmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsraw	zmm30, zmm29, XMMWORD PTR [rdx+2032]	 # AVX512BW Disp8
	vpsraw	zmm30, zmm29, XMMWORD PTR [rdx+2048]	 # AVX512BW
	vpsraw	zmm30, zmm29, XMMWORD PTR [rdx-2048]	 # AVX512BW Disp8
	vpsraw	zmm30, zmm29, XMMWORD PTR [rdx-2064]	 # AVX512BW
	vpsrlw	zmm30, zmm29, xmm28	 # AVX512BW
	vpsrlw	zmm30{k7}, zmm29, xmm28	 # AVX512BW
	vpsrlw	zmm30{k7}{z}, zmm29, xmm28	 # AVX512BW
	vpsrlw	zmm30, zmm29, XMMWORD PTR [rcx]	 # AVX512BW
	vpsrlw	zmm30, zmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsrlw	zmm30, zmm29, XMMWORD PTR [rdx+2032]	 # AVX512BW Disp8
	vpsrlw	zmm30, zmm29, XMMWORD PTR [rdx+2048]	 # AVX512BW
	vpsrlw	zmm30, zmm29, XMMWORD PTR [rdx-2048]	 # AVX512BW Disp8
	vpsrlw	zmm30, zmm29, XMMWORD PTR [rdx-2064]	 # AVX512BW
	vpsrldq	zmm30, zmm29, 0xab	 # AVX512BW
	vpsrldq	zmm30, zmm29, 123	 # AVX512BW
	vpsrldq	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpsrldq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpsrldq	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpsrldq	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpsrldq	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpsrldq	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpsrlw	zmm30, zmm29, 0xab	 # AVX512BW
	vpsrlw	zmm30{k7}, zmm29, 0xab	 # AVX512BW
	vpsrlw	zmm30{k7}{z}, zmm29, 0xab	 # AVX512BW
	vpsrlw	zmm30, zmm29, 123	 # AVX512BW
	vpsrlw	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpsrlw	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpsrlw	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpsrlw	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpsrlw	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpsrlw	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpsraw	zmm30, zmm29, 0xab	 # AVX512BW
	vpsraw	zmm30{k7}, zmm29, 0xab	 # AVX512BW
	vpsraw	zmm30{k7}{z}, zmm29, 0xab	 # AVX512BW
	vpsraw	zmm30, zmm29, 123	 # AVX512BW
	vpsraw	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpsraw	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpsraw	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpsraw	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpsraw	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpsraw	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpsrlvw	zmm30, zmm29, zmm28	 # AVX512BW
	vpsrlvw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpsrlvw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpsrlvw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsrlvw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsrlvw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsrlvw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsrlvw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsrlvw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpsravw	zmm30, zmm29, zmm28	 # AVX512BW
	vpsravw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpsravw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpsravw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsravw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsravw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsravw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsravw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsravw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpsubb	zmm30, zmm29, zmm28	 # AVX512BW
	vpsubb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpsubb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpsubb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsubb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsubb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsubb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsubb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsubb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpsubsb	zmm30, zmm29, zmm28	 # AVX512BW
	vpsubsb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpsubsb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpsubsb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsubsb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsubsb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsubsb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsubsb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsubsb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpsubsw	zmm30, zmm29, zmm28	 # AVX512BW
	vpsubsw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpsubsw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpsubsw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsubsw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsubsw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsubsw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsubsw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsubsw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpsubusb	zmm30, zmm29, zmm28	 # AVX512BW
	vpsubusb	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpsubusb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpsubusb	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsubusb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsubusb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsubusb	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsubusb	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsubusb	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpsubusw	zmm30, zmm29, zmm28	 # AVX512BW
	vpsubusw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpsubusw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpsubusw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsubusw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsubusw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsubusw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsubusw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsubusw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpsubw	zmm30, zmm29, zmm28	 # AVX512BW
	vpsubw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpsubw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpsubw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsubw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsubw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsubw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsubw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsubw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpunpckhbw	zmm30, zmm29, zmm28	 # AVX512BW
	vpunpckhbw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpunpckhbw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpunpckhbw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpunpckhbw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpunpckhbw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpunpckhbw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpunpckhbw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpunpckhbw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpunpckhwd	zmm30, zmm29, zmm28	 # AVX512BW
	vpunpckhwd	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpunpckhwd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpunpckhwd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpunpckhwd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpunpckhwd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpunpckhwd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpunpckhwd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpunpckhwd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpunpcklbw	zmm30, zmm29, zmm28	 # AVX512BW
	vpunpcklbw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpunpcklbw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpunpcklbw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpunpcklbw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpunpcklbw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpunpcklbw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpunpcklbw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpunpcklbw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpunpcklwd	zmm30, zmm29, zmm28	 # AVX512BW
	vpunpcklwd	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpunpcklwd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpunpcklwd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpunpcklwd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpunpcklwd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpunpcklwd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpunpcklwd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpunpcklwd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmovwb	ymm30, zmm29	 # AVX512BW
	vpmovwb	ymm30{k7}, zmm29	 # AVX512BW
	vpmovwb	ymm30{k7}{z}, zmm29	 # AVX512BW
	vpmovswb	ymm30, zmm29	 # AVX512BW
	vpmovswb	ymm30{k7}, zmm29	 # AVX512BW
	vpmovswb	ymm30{k7}{z}, zmm29	 # AVX512BW
	vpmovuswb	ymm30, zmm29	 # AVX512BW
	vpmovuswb	ymm30{k7}, zmm29	 # AVX512BW
	vpmovuswb	ymm30{k7}{z}, zmm29	 # AVX512BW
	vdbpsadbw	zmm30, zmm29, zmm28, 0xab	 # AVX512BW
	vdbpsadbw	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512BW
	vdbpsadbw	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512BW
	vdbpsadbw	zmm30, zmm29, zmm28, 123	 # AVX512BW
	vdbpsadbw	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vdbpsadbw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vdbpsadbw	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vdbpsadbw	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vdbpsadbw	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vdbpsadbw	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpermw	zmm30, zmm29, zmm28	 # AVX512BW
	vpermw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpermw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpermw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpermw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpermw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpermw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpermw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpermw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpermt2w	zmm30, zmm29, zmm28	 # AVX512BW
	vpermt2w	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpermt2w	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpermt2w	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpermt2w	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpermt2w	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpermt2w	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpermt2w	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpermt2w	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpslldq	zmm30, zmm29, 0xab	 # AVX512BW
	vpslldq	zmm30, zmm29, 123	 # AVX512BW
	vpslldq	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpslldq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpslldq	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpslldq	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpslldq	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpslldq	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpsllw	zmm30, zmm29, 0xab	 # AVX512BW
	vpsllw	zmm30{k7}, zmm29, 0xab	 # AVX512BW
	vpsllw	zmm30{k7}{z}, zmm29, 0xab	 # AVX512BW
	vpsllw	zmm30, zmm29, 123	 # AVX512BW
	vpsllw	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpsllw	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpsllw	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpsllw	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpsllw	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpsllw	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpsllvw	zmm30, zmm29, zmm28	 # AVX512BW
	vpsllvw	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpsllvw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpsllvw	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpsllvw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpsllvw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpsllvw	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpsllvw	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpsllvw	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vmovdqu8	zmm30, zmm29	 # AVX512BW
	vmovdqu8	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu8	zmm30{k7}{z}, zmm29	 # AVX512BW
	vmovdqu8	zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vmovdqu8	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vmovdqu8	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vmovdqu8	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vmovdqu8	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vmovdqu8	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vmovdqu16	zmm30, zmm29	 # AVX512BW
	vmovdqu16	zmm30{k7}, zmm29	 # AVX512BW
	vmovdqu16	zmm30{k7}{z}, zmm29	 # AVX512BW
	vmovdqu16	zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vmovdqu16	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vmovdqu16	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vmovdqu16	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vmovdqu16	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vmovdqu16	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	kandq	k5, k6, k7	 # AVX512BW
	kandd	k5, k6, k7	 # AVX512BW
	kandnq	k5, k6, k7	 # AVX512BW
	kandnd	k5, k6, k7	 # AVX512BW
	korq	k5, k6, k7	 # AVX512BW
	kord	k5, k6, k7	 # AVX512BW
	kxnorq	k5, k6, k7	 # AVX512BW
	kxnord	k5, k6, k7	 # AVX512BW
	kxorq	k5, k6, k7	 # AVX512BW
	kxord	k5, k6, k7	 # AVX512BW
	knotq	k5, k6	 # AVX512BW
	knotd	k5, k6	 # AVX512BW
	kortestq	k5, k6	 # AVX512BW
	kortestd	k5, k6	 # AVX512BW
	ktestq	k5, k6	 # AVX512BW
	ktestd	k5, k6	 # AVX512BW
	kshiftrq	k5, k6, 0xab	 # AVX512BW
	kshiftrq	k5, k6, 123	 # AVX512BW
	kshiftrd	k5, k6, 0xab	 # AVX512BW
	kshiftrd	k5, k6, 123	 # AVX512BW
	kshiftlq	k5, k6, 0xab	 # AVX512BW
	kshiftlq	k5, k6, 123	 # AVX512BW
	kshiftld	k5, k6, 0xab	 # AVX512BW
	kshiftld	k5, k6, 123	 # AVX512BW
	kmovq	k5, k6	 # AVX512BW
	kmovq	k5, QWORD PTR [rcx]	 # AVX512BW
	kmovq	k5, QWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	kmovd	k5, k6	 # AVX512BW
	kmovd	k5, DWORD PTR [rcx]	 # AVX512BW
	kmovd	k5, DWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	kmovq	QWORD PTR [rcx], k5	 # AVX512BW
	kmovq	QWORD PTR [rax+r14*8+0x1234], k5	 # AVX512BW
	kmovd	DWORD PTR [rcx], k5	 # AVX512BW
	kmovd	DWORD PTR [rax+r14*8+0x1234], k5	 # AVX512BW
	kmovq	k5, rax	 # AVX512BW
	kmovq	k5, r8	 # AVX512BW
	kmovd	k5, eax	 # AVX512BW
	kmovd	k5, ebp	 # AVX512BW
	kmovd	k5, r13d	 # AVX512BW
	kmovq	rax, k5	 # AVX512BW
	kmovq	r8, k5	 # AVX512BW
	kmovd	eax, k5	 # AVX512BW
	kmovd	ebp, k5	 # AVX512BW
	kmovd	r13d, k5	 # AVX512BW
	kaddq	k5, k6, k7	 # AVX512BW
	kaddd	k5, k6, k7	 # AVX512BW
	kunpckwd	k5, k6, k7	 # AVX512BW
	kunpckdq	k5, k6, k7	 # AVX512BW
	vpmovwb	YMMWORD PTR [rcx], zmm30	 # AVX512BW
	vpmovwb	YMMWORD PTR [rcx]{k7}, zmm30	 # AVX512BW
	vpmovwb	YMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512BW
	vpmovwb	YMMWORD PTR [rdx+4064], zmm30	 # AVX512BW Disp8
	vpmovwb	YMMWORD PTR [rdx+4096], zmm30	 # AVX512BW
	vpmovwb	YMMWORD PTR [rdx-4096], zmm30	 # AVX512BW Disp8
	vpmovwb	YMMWORD PTR [rdx-4128], zmm30	 # AVX512BW
	vpmovswb	YMMWORD PTR [rcx], zmm30	 # AVX512BW
	vpmovswb	YMMWORD PTR [rcx]{k7}, zmm30	 # AVX512BW
	vpmovswb	YMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512BW
	vpmovswb	YMMWORD PTR [rdx+4064], zmm30	 # AVX512BW Disp8
	vpmovswb	YMMWORD PTR [rdx+4096], zmm30	 # AVX512BW
	vpmovswb	YMMWORD PTR [rdx-4096], zmm30	 # AVX512BW Disp8
	vpmovswb	YMMWORD PTR [rdx-4128], zmm30	 # AVX512BW
	vpmovuswb	YMMWORD PTR [rcx], zmm30	 # AVX512BW
	vpmovuswb	YMMWORD PTR [rcx]{k7}, zmm30	 # AVX512BW
	vpmovuswb	YMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512BW
	vpmovuswb	YMMWORD PTR [rdx+4064], zmm30	 # AVX512BW Disp8
	vpmovuswb	YMMWORD PTR [rdx+4096], zmm30	 # AVX512BW
	vpmovuswb	YMMWORD PTR [rdx-4096], zmm30	 # AVX512BW Disp8
	vpmovuswb	YMMWORD PTR [rdx-4128], zmm30	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [rcx], zmm30	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512BW Disp8
	vmovdqu8	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512BW Disp8
	vmovdqu8	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [rcx], zmm30	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512BW Disp8
	vmovdqu16	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512BW Disp8
	vmovdqu16	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512BW
	vpermi2w	zmm30, zmm29, zmm28	 # AVX512BW
	vpermi2w	zmm30{k7}, zmm29, zmm28	 # AVX512BW
	vpermi2w	zmm30{k7}{z}, zmm29, zmm28	 # AVX512BW
	vpermi2w	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vpermi2w	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vpermi2w	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vpermi2w	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vpermi2w	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vpermi2w	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vptestmb	k5, zmm30, zmm29	 # AVX512BW
	vptestmb	k5{k7}, zmm30, zmm29	 # AVX512BW
	vptestmb	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vptestmb	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vptestmb	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vptestmb	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vptestmb	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vptestmb	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vptestmw	k5, zmm30, zmm29	 # AVX512BW
	vptestmw	k5{k7}, zmm30, zmm29	 # AVX512BW
	vptestmw	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512BW
	vptestmw	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vptestmw	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vptestmw	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vptestmw	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vptestmw	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpmovb2m	k5, zmm30	 # AVX512BW
	vpmovw2m	k5, zmm30	 # AVX512BW
	vpmovm2b	zmm30, k5	 # AVX512BW
	vpmovm2w	zmm30, k5	 # AVX512BW
	vptestnmb	k5, zmm29, zmm28	 # AVX512BW
	vptestnmb	k5{k7}, zmm29, zmm28	 # AVX512BW
	vptestnmb	k5, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vptestnmb	k5, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vptestnmb	k5, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vptestnmb	k5, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vptestnmb	k5, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vptestnmb	k5, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vptestnmw	k5, zmm29, zmm28	 # AVX512BW
	vptestnmw	k5{k7}, zmm29, zmm28	 # AVX512BW
	vptestnmw	k5, zmm29, ZMMWORD PTR [rcx]	 # AVX512BW
	vptestnmw	k5, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512BW
	vptestnmw	k5, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512BW Disp8
	vptestnmw	k5, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512BW
	vptestnmw	k5, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512BW Disp8
	vptestnmw	k5, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512BW
	vpcmpb	k5, zmm30, zmm29, 0xab	 # AVX512BW
	vpcmpb	k5{k7}, zmm30, zmm29, 0xab	 # AVX512BW
	vpcmpb	k5, zmm30, zmm29, 123	 # AVX512BW
	vpcmpb	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpcmpb	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpcmpb	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpcmpb	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpcmpb	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpcmpb	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpcmpw	k5, zmm30, zmm29, 0xab	 # AVX512BW
	vpcmpw	k5{k7}, zmm30, zmm29, 0xab	 # AVX512BW
	vpcmpw	k5, zmm30, zmm29, 123	 # AVX512BW
	vpcmpw	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpcmpw	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpcmpw	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpcmpw	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpcmpw	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpcmpw	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpcmpub	k5, zmm30, zmm29, 0xab	 # AVX512BW
	vpcmpub	k5{k7}, zmm30, zmm29, 0xab	 # AVX512BW
	vpcmpub	k5, zmm30, zmm29, 123	 # AVX512BW
	vpcmpub	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpcmpub	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpcmpub	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpcmpub	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpcmpub	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpcmpub	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
	vpcmpuw	k5, zmm30, zmm29, 0xab	 # AVX512BW
	vpcmpuw	k5{k7}, zmm30, zmm29, 0xab	 # AVX512BW
	vpcmpuw	k5, zmm30, zmm29, 123	 # AVX512BW
	vpcmpuw	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512BW
	vpcmpuw	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512BW
	vpcmpuw	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512BW Disp8
	vpcmpuw	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512BW
	vpcmpuw	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512BW Disp8
	vpcmpuw	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512BW
