#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*


Disassembly of section .text:
0x[0-9a-f]+\s+c0c2\s+leave_s	\[r13\]
0x[0-9a-f]+\s+c0c2\s+leave_s	\[r13\]
0x[0-9a-f]+\s+c4dc\s+leave_s	\[r13-gp,pcl\]
0x[0-9a-f]+\s+c1dc\s+leave_s	\[r13-gp,fp\]
0x[0-9a-f]+\s+c2dc\s+leave_s	\[r13-gp,blink\]
0x[0-9a-f]+\s+c3dc\s+leave_s	\[r13-gp,fp,blink\]
0x[0-9a-f]+\s+c5dc\s+leave_s	\[r13-gp,fp,pcl\]
0x[0-9a-f]+\s+c6dc\s+leave_s	\[r13-gp,blink,pcl\]
0x[0-9a-f]+\s+c7dc\s+leave_s	\[r13-gp,fp,blink,pcl\]
0x[0-9a-f]+\s+c6c2\s+leave_s	\[r13,blink,pcl\]
0x[0-9a-f]+\s+c6c0\s+leave_s	\[blink,pcl\]
0x[0-9a-f]+\s+c1c0\s+leave_s	\[fp\]
0x[0-9a-f]+\s+c2c0\s+leave_s	\[blink\]
0x[0-9a-f]+\s+c4c0\s+leave_s	\[pcl\]
0x[0-9a-f]+\s+1100 0000\s+ld	r0,\[r1\]
0x[0-9a-f]+\s+c0e2\s+enter_s	\[r13\]
0x[0-9a-f]+\s+c0e2\s+enter_s	\[r13\]
0x[0-9a-f]+\s+c1fc\s+enter_s	\[r13-gp,fp\]
0x[0-9a-f]+\s+c2fc\s+enter_s	\[r13-gp,blink\]
0x[0-9a-f]+\s+c3fc\s+enter_s	\[r13-gp,fp,blink\]
0x[0-9a-f]+\s+c2e2\s+enter_s	\[r13,blink]
0x[0-9a-f]+\s+c2e0\s+enter_s	\[blink\]
0x[0-9a-f]+\s+c3e0\s+enter_s	\[fp,blink\]
0x[0-9a-f]+\s+c1e0\s+enter_s	\[fp\]
