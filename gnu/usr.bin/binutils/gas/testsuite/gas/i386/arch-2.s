# Test .arch .sse4.2
.arch generic32
.arch .sse4.2
ptest		%xmm1,%xmm0
roundpd		$0,%xmm1,%xmm0
roundps		$0,%xmm1,%xmm0
roundsd		$0,%xmm1,%xmm0
roundss		$0,%xmm1,%xmm0
crc32		%ecx,%ebx
