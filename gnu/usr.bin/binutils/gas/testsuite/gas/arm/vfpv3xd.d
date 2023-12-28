#objdump: -dr --prefix-addresses --show-raw-insn
#name: VFP Double-precision load/store
#as: -mfpu=vfpv3xd

# Test the ARM VFP Double Precision load/store on single precision FPU

.*: +file format .*arm.*

Disassembly of section .text:
0+[0-9a-f]* <[^>]*> ed900b00 	vldr	d0, \[r0\]
0+[0-9a-f]* <[^>]*> ed800b00 	vstr	d0, \[r0\]
0+[0-9a-f]* <[^>]*> ec900b02 	vldmia	r0, {d0}
0+[0-9a-f]* <[^>]*> ec900b02 	vldmia	r0, {d0}
0+[0-9a-f]* <[^>]*> ecb00b02 	vldmia	r0!, {d0}
0+[0-9a-f]* <[^>]*> ecb00b02 	vldmia	r0!, {d0}
0+[0-9a-f]* <[^>]*> ed300b02 	vldmdb	r0!, {d0}
0+[0-9a-f]* <[^>]*> ed300b02 	vldmdb	r0!, {d0}
0+[0-9a-f]* <[^>]*> ec800b02 	vstmia	r0, {d0}
0+[0-9a-f]* <[^>]*> ec800b02 	vstmia	r0, {d0}
0+[0-9a-f]* <[^>]*> eca00b02 	vstmia	r0!, {d0}
0+[0-9a-f]* <[^>]*> eca00b02 	vstmia	r0!, {d0}
0+[0-9a-f]* <[^>]*> ed200b02 	vstmdb	r0!, {d0}
0+[0-9a-f]* <[^>]*> ed200b02 	vstmdb	r0!, {d0}
