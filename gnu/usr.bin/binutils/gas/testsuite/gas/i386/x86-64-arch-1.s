# Test .arch .sse4.1
.arch generic64
.arch .sse4.1
ptest		%xmm1,%xmm0
roundpd		$0,%xmm1,%xmm0
roundps		$0,%xmm1,%xmm0
roundsd		$0,%xmm1,%xmm0
roundss		$0,%xmm1,%xmm0
phminposuw	%xmm1,%xmm3
