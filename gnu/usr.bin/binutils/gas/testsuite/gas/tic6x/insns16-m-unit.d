#objdump: -dr --prefix-addresses --show-raw-insn
#name:
#as: -march=c64x+ -mlittle-endian

.*: *file format elf32-tic6x-le


Disassembly of section .text:
[ \t]*\.\.\.
[0-9a-f]+[02468ace] <[^>]*> 231e[ \t]+mpy \.M1 a1,a6,a0
[0-9a-f]+[02468ace] <[^>]*> 469f[ \t]+mpy \.M2 b2,b5,b2
[0-9a-f]+[02468ace] <[^>]*> 799e[ \t]+mpy \.M1X a3,b3,a4
[0-9a-f]+[02468ace] <[^>]*> 9c1f[ \t]+mpy \.M2X b4,a0,b6
[0-9a-f]+[02468ace] <[^>]*> a71e[ \t]+mpy \.M1 a5,a6,a2
[0-9a-f]+[02468ace] <[^>]*> ca9f[ \t]+mpy \.M2 b6,b5,b4
[0-9a-f]+[02468ace] <[^>]*> fd9e[ \t]+mpy \.M1X a7,b3,a6
[0-9a-f]+[02468ace] <[^>]*> 213e[ \t]+mpyh \.M1 a1,a2,a0
[0-9a-f]+[02468ace] <[^>]*> 46bf[ \t]+mpyh \.M2 b2,b5,b2
[0-9a-f]+[02468ace] <[^>]*> 7bbe[ \t]+mpyh \.M1X a3,b7,a4
[0-9a-f]+[02468ace] <[^>]*> 9c3f[ \t]+mpyh \.M2X b4,a0,b6
[0-9a-f]+[02468ace] <[^>]*> a53e[ \t]+mpyh \.M1 a5,a2,a2
[0-9a-f]+[02468ace] <[^>]*> cabf[ \t]+mpyh \.M2 b6,b5,b4
[0-9a-f]+[02468ace] <[^>]*> ffbe[ \t]+mpyh \.M1X a7,b7,a6
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 225e[ \t]+mpylh \.M1 a17,a20,a16
[0-9a-f]+[02468ace] <[^>]*> 47df[ \t]+mpylh \.M2 b18,b23,b18
[0-9a-f]+[02468ace] <[^>]*> 78de[ \t]+mpylh \.M1X a19,b17,a20
[0-9a-f]+[02468ace] <[^>]*> 9d5f[ \t]+mpylh \.M2X b20,a18,b22
[0-9a-f]+[02468ace] <[^>]*> a6de[ \t]+mpylh \.M1 a21,a21,a18
[0-9a-f]+[02468ace] <[^>]*> cbdf[ \t]+mpylh \.M2 b22,b23,b20
[0-9a-f]+[02468ace] <[^>]*> fc5e[ \t]+mpylh \.M1X a23,b16,a22
[0-9a-f]+[02468ace] <[^>]*> 207e[ \t]+mpyhl \.M1 a17,a16,a16
[0-9a-f]+[02468ace] <[^>]*> 45ff[ \t]+mpyhl \.M2 b18,b19,b18
[0-9a-f]+[02468ace] <[^>]*> 7afe[ \t]+mpyhl \.M1X a19,b21,a20
[0-9a-f]+[02468ace] <[^>]*> 9f7f[ \t]+mpyhl \.M2X b20,a22,b22
[0-9a-f]+[02468ace] <[^>]*> a47e[ \t]+mpyhl \.M1 a21,a16,a18
[0-9a-f]+[02468ace] <[^>]*> c9ff[ \t]+mpyhl \.M2 b22,b19,b20
[0-9a-f]+[02468ace] <[^>]*> fefe[ \t]+mpyhl \.M1X a23,b21,a22
[0-9a-f]+[02468ace] <[^>]*> efe80000[ \t]+<fetch packet header 0xefe80000>
[0-9a-f]+[02468ace] <[^>]*> 231e[ \t]+smpy \.M1 a17,a22,a16
[0-9a-f]+[02468ace] <[^>]*> 469f[ \t]+smpy \.M2 b18,b21,b18
[0-9a-f]+[02468ace] <[^>]*> 799e[ \t]+smpy \.M1X a19,b19,a20
[0-9a-f]+[02468ace] <[^>]*> 9c1f[ \t]+smpy \.M2X b20,a16,b22
[0-9a-f]+[02468ace] <[^>]*> a71e[ \t]+smpy \.M1 a21,a22,a18
[0-9a-f]+[02468ace] <[^>]*> ca9f[ \t]+smpy \.M2 b22,b21,b20
[0-9a-f]+[02468ace] <[^>]*> fd9e[ \t]+smpy \.M1X a23,b19,a22
[0-9a-f]+[02468ace] <[^>]*> 213e[ \t]+smpyh \.M1 a17,a18,a16
[0-9a-f]+[02468ace] <[^>]*> 46bf[ \t]+smpyh \.M2 b18,b21,b18
[0-9a-f]+[02468ace] <[^>]*> 7bbe[ \t]+smpyh \.M1X a19,b23,a20
[0-9a-f]+[02468ace] <[^>]*> 9c3f[ \t]+smpyh \.M2X b20,a16,b22
[0-9a-f]+[02468ace] <[^>]*> a53e[ \t]+smpyh \.M1 a21,a18,a18
[0-9a-f]+[02468ace] <[^>]*> cabf[ \t]+smpyh \.M2 b22,b21,b20
[0-9a-f]+[02468ace] <[^>]*> ffbe[ \t]+smpyh \.M1X a23,b23,a22
[0-9a-f]+[02468ace] <[^>]*> efe84000[ \t]+<fetch packet header 0xefe84000>
[0-9a-f]+[02468ace] <[^>]*> 225e[ \t]+smpylh \.M1 a1,a4,a0
[0-9a-f]+[02468ace] <[^>]*> 47df[ \t]+smpylh \.M2 b2,b7,b2
[0-9a-f]+[02468ace] <[^>]*> 78de[ \t]+smpylh \.M1X a3,b1,a4
[0-9a-f]+[02468ace] <[^>]*> 9d5f[ \t]+smpylh \.M2X b4,a2,b6
[0-9a-f]+[02468ace] <[^>]*> a6de[ \t]+smpylh \.M1 a5,a5,a2
[0-9a-f]+[02468ace] <[^>]*> cbdf[ \t]+smpylh \.M2 b6,b7,b4
[0-9a-f]+[02468ace] <[^>]*> fc5e[ \t]+smpylh \.M1X a7,b0,a6
[0-9a-f]+[02468ace] <[^>]*> 207e[ \t]+smpyhl \.M1 a1,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 45ff[ \t]+smpyhl \.M2 b2,b3,b2
[0-9a-f]+[02468ace] <[^>]*> 7afe[ \t]+smpyhl \.M1X a3,b5,a4
[0-9a-f]+[02468ace] <[^>]*> 9f7f[ \t]+smpyhl \.M2X b4,a6,b6
[0-9a-f]+[02468ace] <[^>]*> a47e[ \t]+smpyhl \.M1 a5,a0,a2
[0-9a-f]+[02468ace] <[^>]*> c9ff[ \t]+smpyhl \.M2 b6,b3,b4
[0-9a-f]+[02468ace] <[^>]*> fefe[ \t]+smpyhl \.M1X a7,b5,a6
[0-9a-f]+[02468ace] <[^>]*> efe04000[ \t]+<fetch packet header 0xefe04000>
[ \t]*\.\.\.
