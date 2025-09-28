# name: rgn-over7
# source: rgn-over.s
# ld: -T rgn-over7.t -Map tmpdir/rgn-over7.map
# error: \A[^ \n]*?ld[^:\n]*?: [^\n]*?section `\.text' will not fit in region `r1'\n[^ \n]*?ld[^:\n]*?: section \.data LMA \[0+1008,0+1013\] overlaps section \.text LMA \[0+1000,0+100b\]\n[^ \n]*?ld[^:\n]*?: region `r1' overflowed by 4 bytes\Z

#...
Discarded input sections
#...
Memory\s+Configuration

Name\s+Origin\s+Length\s+Attributes
bss\s+0x0+0000\s+0x0+0000\s+xrw
r1\s+0x0+1000\s+0x0+0008\s+xrw
r2\s+0x0+1008\s+0x0+000c\s+xrw
\*default\*\s+0x0+0000\s+0xf+ffff

Linker\s+script\s+and\s+memory\s+map

\s*0x0+1000\s+_start\s+=\s+0x1000

\s*\.bss\s+0x0+0000\s+0x0
\s*\*\(\.bss\)
\s*\.bss\s+0x0+0000\s+0x0\s+.*?

\s*\.text\s+0x0+1000\s+0xc
\s*\*\(\.txt\)
\s*\.txt\s+0x0+1000\s+0xc\s+.*?

\s*\.data\s+0x0+1008\s+0xc
\s*\*\(\.dat\)
\s*\.dat\s+0x0+1008\s+0xc\s+.*?

/DISCARD/
 \*\(\*\)
LOAD\s+.*?
OUTPUT\(.*?\)
#pass
