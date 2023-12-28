# Test .arch .sse4
.arch generic32
.arch .sse4
popcnt	%ecx,%ebx
crc32	%ecx,%ebx
