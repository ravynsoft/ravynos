# Test .arch .sse4.2
.arch generic32
.arch .sse4.2
popcnt	%ecx,%ebx
crc32	%ecx,%ebx
