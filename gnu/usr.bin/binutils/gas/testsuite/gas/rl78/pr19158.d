#objdump: -d --prefix-addresses --show-raw-insn
#name: PR19158: RL78: Show the system registers in disassembly

.*: +file format .*rl78.*

Disassembly of section .text:
0+000 <.*> 8e f8[	 ]+mov[	 ]+a, spl
0+002 <.*> 9e f8[	 ]+mov[	 ]+spl, a
0+004 <.*> ce f8 7b[	 ]+mov[	 ]+spl, #123
0+007 <.*> 61 ab f8[	 ]+xch[	 ]+a, spl
0+00a <.*> ae f8[	 ]+movw[	 ]+ax, sp
0+00c <.*> be f8[	 ]+movw[	 ]+sp, ax
0+00e <.*> cb f8 34 12[	 ]+movw[	 ]+sp, #0x1234
0+012 <.*> 61 ab fa[	 ]+xch[	 ]+a, psw
0+015 <.*> 61 ab f9[	 ]+xch[	 ]+a, sph
0+018 <.*> 61 ab fc[	 ]+xch[	 ]+a, cs
0+01b <.*> 61 ab fd[	 ]+xch[	 ]+a, es
0+01e <.*> 61 ab fe[	 ]+xch[	 ]+a, pmc
0+021 <.*> 61 ab ff[	 ]+xch[	 ]+a, mem
