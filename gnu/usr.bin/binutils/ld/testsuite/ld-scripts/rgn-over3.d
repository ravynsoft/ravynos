# name: rgn-over3
# source: rgn-over.s
# ld: -T rgn-over3.t -Map tmpdir/rgn-over3.map
# error: \A[^ \n]*?ld[^:\n]*?: [^\n]*?section `\.text' will not fit in region `r1'\n[^ \n]*?ld[^:\n]*?: [^\n]*?section `\.data' will not fit in region `r2'\n[^ \n]*?ld[^:\n]*?: region `r1' overflowed by 4 bytes\n[^ \n]*?ld[^:\n]*?: region `r2' overflowed by 4 bytes\Z

#...
Discarded input sections
#...
Memory\s+Configuration

Name\s+Origin\s+Length\s+Attributes
bss\s+0x0+0000\s+0x0+0000\s+xrw
r1\s+0x0+1000\s+0x0+0008\s+xrw
r2\s+0x0+2000\s+0x0+0008\s+xrw
\*default\*\s+0x0+0000\s+0xf+ffff

Linker\s+script\s+and\s+memory\s+map

\s*0x0+1000\s+_start\s+=\s+0x1000

\s*\.bss\s+0x0+0000\s+0x0
\s*\*\(\.bss\)
\s*\.bss\s+0x0+0000\s+0x0\s+.*?

\s*\.text\s+0x0+1000\s+0xc
\s*\*\(\.txt\)
\s*\.txt\s+0x0+1000\s+0xc\s+.*?

\s*\.data\s+0x0+2000\s+0xc
\s*\*\(\.dat\)
\s*\.dat\s+0x0+2000\s+0xc\s+.*?

/DISCARD/
 \*\(\*\)
LOAD\s+.*?
OUTPUT\(.*?\)
#pass
