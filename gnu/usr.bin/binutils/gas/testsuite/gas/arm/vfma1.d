#objdump: -dr --prefix-addresses --show-raw-insn
#name: VFMA decoding
#as: -mcpu=arm7m
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

# Test VFMA instruction disassembly

.*: *file format .*arm.*


Disassembly of section .text:
00000000 <[^>]*> ee000a00 	vmla.f32	s0, s0, s0
00000004 <[^>]*> ee000b00 	vmla.f64	d0, d0, d0
00000008 <[^>]*> f2000d10 	vmla.f32	d0, d0, d0
0000000c <[^>]*> f2000d50 	vmla.f32	q0, q0, q0
00000010 <[^>]*> eea00a00 	vfma.f32	s0, s0, s0
00000014 <[^>]*> eea00b00 	vfma.f64	d0, d0, d0
00000018 <[^>]*> f2000c10 	vfma.f32	d0, d0, d0
0000001c <[^>]*> f2000c50 	vfma.f32	q0, q0, q0
00000020 <[^>]*> ee000a40 	vmls.f32	s0, s0, s0
00000024 <[^>]*> ee000b40 	vmls.f64	d0, d0, d0
00000028 <[^>]*> f2200d10 	vmls.f32	d0, d0, d0
0000002c <[^>]*> f2200d50 	vmls.f32	q0, q0, q0
00000030 <[^>]*> eea00a40 	vfms.f32	s0, s0, s0
00000034 <[^>]*> eea00b40 	vfms.f64	d0, d0, d0
00000038 <[^>]*> f2200c10 	vfms.f32	d0, d0, d0
0000003c <[^>]*> f2200c50 	vfms.f32	q0, q0, q0
00000040 <[^>]*> ee100a40 	vnmla.f32	s0, s0, s0
00000044 <[^>]*> ee100b40 	vnmla.f64	d0, d0, d0
00000048 <[^>]*> ee900a40 	vfnma.f32	s0, s0, s0
0000004c <[^>]*> ee900b40 	vfnma.f64	d0, d0, d0
00000050 <[^>]*> ee100a00 	vnmls.f32	s0, s0, s0
00000054 <[^>]*> ee100b00 	vnmls.f64	d0, d0, d0
00000058 <[^>]*> ee900a00 	vfnms.f32	s0, s0, s0
0000005c <[^>]*> ee900b00 	vfnms.f64	d0, d0, d0
