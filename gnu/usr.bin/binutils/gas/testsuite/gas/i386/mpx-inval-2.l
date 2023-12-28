.*: Assembler messages:
.*:5: Error: 16-bit address isn't allowed in MPX instructions
.*:6: Error: 16-bit address isn't allowed in MPX instructions
.*:7: Error: 16-bit address isn't allowed in MPX instructions
.*:8: Error: 16-bit address isn't allowed in MPX instructions
.*:9: Error: 16-bit address isn't allowed in MPX instructions
.*:10: Error: 16-bit address isn't allowed in MPX instructions
.*:11: Error: 16-bit address isn't allowed in MPX instructions
.*:12: Error: 16-bit address isn't allowed in MPX instructions
.*:14: Error: .* `bndcl'
.*:15: Error: .* `bndcn'
.*:16: Error: .* `bndcu'
.*:19: Error: 16-bit address isn't allowed in MPX instructions
.*:20: Error: 16-bit address isn't allowed in MPX instructions
.*:21: Error: 16-bit address isn't allowed in MPX instructions
.*:22: Error: 16-bit address isn't allowed in MPX instructions
.*:23: Error: 16-bit address isn't allowed in MPX instructions
.*:24: Error: 16-bit address isn't allowed in MPX instructions
.*:25: Error: 16-bit address isn't allowed in MPX instructions
.*:26: Error: 16-bit address isn't allowed in MPX instructions
.*:28: Error: .* `bndcl'
.*:29: Error: .* `bndcn'
.*:30: Error: .* `bndcu'
.*:35: Error: 16-bit address isn't allowed in MPX instructions
.*:36: Error: 16-bit address isn't allowed in MPX instructions
.*:37: Error: 16-bit address isn't allowed in MPX instructions
.*:38: Error: 16-bit address isn't allowed in MPX instructions
.*:39: Error: 16-bit address isn't allowed in MPX instructions
.*:40: Error: 16-bit address isn't allowed in MPX instructions
.*:41: Error: 16-bit address isn't allowed in MPX instructions
.*:42: Error: 16-bit address isn't allowed in MPX instructions
.*:44: Error: .* `bndcl'
.*:45: Error: .* `bndcn'
.*:46: Error: .* `bndcu'
.*:49: Error: 16-bit address isn't allowed in MPX instructions
.*:50: Error: 16-bit address isn't allowed in MPX instructions
.*:51: Error: 16-bit address isn't allowed in MPX instructions
.*:52: Error: 16-bit address isn't allowed in MPX instructions
.*:53: Error: 16-bit address isn't allowed in MPX instructions
.*:54: Error: 16-bit address isn't allowed in MPX instructions
.*:55: Error: 16-bit address isn't allowed in MPX instructions
.*:56: Error: 16-bit address isn't allowed in MPX instructions
.*:58: Error: .* `bndcl'
.*:59: Error: .* `bndcn'
.*:60: Error: .* `bndcu'
GAS LISTING .*
#...
[ 	]*[1-9][0-9]*[ 	]+mpx32:
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcl	\(%bx,%si\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcn	\(%bx,%di\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcu	\(%bp,%si\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndldx	\(%bp,%di\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmk	\(%bx\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmov	\(%bp\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmov	%bnd0, \(%si\)
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndstx	%bnd0, \(%di\)
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcl	bnd0, \[bx\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcn	bnd0, \[bp\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcu	bnd0, \[si\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndldx	bnd0, \[di\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmk	bnd0, \[bx\+si\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmov	bnd0, \[bx\+di\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmov	\[bp\+si\], bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndstx	\[bp\+di\], bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+mpx16:
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcl	\(%bx,%si\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcn	\(%bx,%di\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcu	\(%bp,%si\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndldx	\(%bp,%di\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmk	\(%bx\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmov	\(%bp\), %bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmov	%bnd0, \(%si\)
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndstx	%bnd0, \(%di\)
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcl	bnd0, \[bx\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcn	bnd0, \[bp\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndcu	bnd0, \[si\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndldx	bnd0, \[di\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmk	bnd0, \[bx\+si\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmov	bnd0, \[bx\+di\]
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndmov	\[bp\+si\], bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
[ 	]*[1-9][0-9]*[ 	]+[?0-9A-F 	]*bndstx	\[bp\+di\], bnd0
.*  Error: 16-bit address isn't allowed in MPX instructions
#...
