#objdump: -dw
#name: 32-bit SVME

.*: +file format .*

Disassembly of section .text:

0+000 <common>:
[	 ]*[0-9a-f]+:[	 ]+0f 01 dd[	 ]+clgi
[	 ]*[0-9a-f]+:[	 ]+0f 01 df[	 ]+invlpga
[	 ]*[0-9a-f]+:[	 ]+0f 01 de[	 ]+skinit
[	 ]*[0-9a-f]+:[	 ]+0f 01 dc[	 ]+stgi
[	 ]*[0-9a-f]+:[	 ]+0f 01 da[	 ]+vmload
[	 ]*[0-9a-f]+:[	 ]+0f 01 d9[	 ]+vmmcall
[	 ]*[0-9a-f]+:[	 ]+0f 01 d8[	 ]+vmrun
[	 ]*[0-9a-f]+:[	 ]+0f 01 db[	 ]+vmsave
[0-9a-f]+ <att32>:
[	 ]*[0-9a-f]+:[	 ]+0f 01 de[	 ]+skinit
[	 ]*[0-9a-f]+:[	 ]+0f 01 df[	 ]+invlpga
[	 ]*[0-9a-f]+:[	 ]+0f 01 da[	 ]+vmload
[	 ]*[0-9a-f]+:[	 ]+0f 01 d8[	 ]+vmrun
[	 ]*[0-9a-f]+:[	 ]+0f 01 db[	 ]+vmsave
[0-9a-f]+ <att16>:
[	 ]*[0-9a-f]+:[	 ]+67 0f 01 df[	 ]+addr16 invlpga
[	 ]*[0-9a-f]+:[	 ]+67 0f 01 da[	 ]+addr16 vmload
[	 ]*[0-9a-f]+:[	 ]+67 0f 01 d8[	 ]+addr16 vmrun
[	 ]*[0-9a-f]+:[	 ]+67 0f 01 db[	 ]+addr16 vmsave
[0-9a-f]+ <intel32>:
[	 ]*[0-9a-f]+:[	 ]+0f 01 de[	 ]+skinit
[	 ]*[0-9a-f]+:[	 ]+0f 01 df[	 ]+invlpga
[	 ]*[0-9a-f]+:[	 ]+0f 01 da[	 ]+vmload
[	 ]*[0-9a-f]+:[	 ]+0f 01 d8[	 ]+vmrun
[	 ]*[0-9a-f]+:[	 ]+0f 01 db[	 ]+vmsave
[0-9a-f]+ <intel16>:
[	 ]*[0-9a-f]+:[	 ]+67 0f 01 df[	 ]+addr16 invlpga
[	 ]*[0-9a-f]+:[	 ]+67 0f 01 da[	 ]+addr16 vmload
[	 ]*[0-9a-f]+:[	 ]+67 0f 01 d8[	 ]+addr16 vmrun
[	 ]*[0-9a-f]+:[	 ]+67 0f 01 db[	 ]+addr16 vmsave
#pass
