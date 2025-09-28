# Check 64bit SSE instructions without AVX equivalent

	.text
_start:
 cmpxchg16b (%rax)
 crc32           %cl,%ebx
 cvtpd2pi	%xmm3,%mm2
 cvtpi2pd	%mm3,%xmm2
 cvtpi2ps	%mm3,%xmm2
 cvtps2pi	%xmm7,%mm6
 cvttpd2pi	%xmm4,%mm3
 cvttps2pi	%xmm4,%mm3
 fisttps (%rax)
 fisttpl (%rax)
 fisttpll (%rax)
 lfence
 maskmovq	%mm7,%mm0
 mfence
 monitor
 movdq2q	%xmm0, %mm1
 movnti %eax, (%rax)
 movntq		%mm2,(%rax)
 movq2dq	%mm0, %xmm1
 mwait
 pabsb           %mm1,%mm0
 pabsd           %mm1,%mm0
 pabsw           %mm1,%mm0
 paddq           %mm1,%mm0
 palignr         $0x2,%mm1,%mm0
 pavgb		%mm1,%mm0
 pavgw		%mm3,%mm2
 pextrw		$0x0,%mm1,%eax
 phaddd          %mm1,%mm0
 phaddsw         %mm1,%mm0
 phaddw          %mm1,%mm0
 phsubd          %mm1,%mm0
 phsubsw         %mm1,%mm0
 phsubw          %mm1,%mm0
 pinsrw		$0x2,%edx,%mm2
 pmaddubsw       %mm1,%mm0
 pmaxsw		%mm1,%mm0
 pmaxub		%mm2,%mm2
 pminsw		%mm5,%mm4
 pminub		%mm7,%mm6
 pmovmskb	%mm5,%eax
 pmulhrsw        %mm1,%mm0
 pmulhuw	%mm5,%mm4
 pmuludq         %mm0, %mm1
 popcnt %ebx,%ecx
 prefetchnta	(%rax)
 prefetcht0	(%rax)
 prefetcht1	(%rax)
 prefetcht2	(%rax)
 psadbw		%mm7,%mm6
 pshufb         %mm1,%mm0
 pshufw		$0x1,%mm2,%mm3
 psignb          %mm1,%mm0
 psignd          %mm1,%mm0
 psignw          %mm1,%mm0
 psubq %mm1,%mm0
 sfence
