#objdump: -dr --prefix-addresses --show-raw-insn
#name:
#as: -march=c64x+ -mlittle-endian

.*: *file format elf32-tic6x-le


Disassembly of section .text:
[ \t]*\.\.\.
[0-9a-f]+[02468ace] <[^>]*> 000a[ \t]+add \.S1 a0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 201b[ \t]+add \.S2 b1,b0,b1
[0-9a-f]+[02468ace] <[^>]*> 512a[ \t]+add \.S1X a2,b2,a2
[0-9a-f]+[02468ace] <[^>]*> 713b[ \t]+add \.S2X b3,a2,b3
[0-9a-f]+[02468ace] <[^>]*> 824a[ \t]+add \.S1 a4,a4,a4
[0-9a-f]+[02468ace] <[^>]*> a25b[ \t]+add \.S2 b5,b4,b5
[0-9a-f]+[02468ace] <[^>]*> d36a[ \t]+add \.S1X a6,b6,a6
[0-9a-f]+[02468ace] <[^>]*> f37b[ \t]+add \.S2X b7,a6,b7
[0-9a-f]+[02468ace] <[^>]*> e28a[ \t]+add \.S1 a7,a5,a0
[0-9a-f]+[02468ace] <[^>]*> 0a9b[ \t]+sub \.S2 b0,b5,b1
[0-9a-f]+[02468ace] <[^>]*> 39aa[ \t]+sub \.S1X a1,b3,a2
[0-9a-f]+[02468ace] <[^>]*> 59bb[ \t]+sub \.S2X b2,a3,b3
[0-9a-f]+[02468ace] <[^>]*> 68ca[ \t]+sub \.S1 a3,a1,a4
[0-9a-f]+[02468ace] <[^>]*> 88db[ \t]+sub \.S2 b4,b1,b5
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 21ea[ \t]+sadd \.S1 a1,a3,a6
[0-9a-f]+[02468ace] <[^>]*> 41fb[ \t]+sadd \.S2 b2,b3,b7
[0-9a-f]+[02468ace] <[^>]*> 720a[ \t]+sadd \.S1X a3,b4,a0
[0-9a-f]+[02468ace] <[^>]*> 921b[ \t]+sadd \.S2X b4,a4,b1
[0-9a-f]+[02468ace] <[^>]*> c32a[ \t]+sadd \.S1 a6,a6,a2
[0-9a-f]+[02468ace] <[^>]*> e33b[ \t]+sadd \.S2 b7,b6,b3
[0-9a-f]+[02468ace] <[^>]*> f24a[ \t]+sadd \.S1X a7,b4,a4
[0-9a-f]+[02468ace] <[^>]*> b25b[ \t]+sadd \.S2X b5,a4,b5
[0-9a-f]+[02468ace] <[^>]*> 816a[ \t]+sadd \.S1 a4,a2,a6
[0-9a-f]+[02468ace] <[^>]*> a87b[ \t]+sub \.S2 b5,b0,b7
[0-9a-f]+[02468ace] <[^>]*> d88a[ \t]+sub \.S1X a6,b1,a0
[0-9a-f]+[02468ace] <[^>]*> fa9b[ \t]+sub \.S2X b7,a5,b1
[0-9a-f]+[02468ace] <[^>]*> eaaa[ \t]+sub \.S1 a7,a5,a2
[0-9a-f]+[02468ace] <[^>]*> 7bbb[ \t]+sub \.S2X b3,a7,b3
[0-9a-f]+[02468ace] <[^>]*> efe04000[ \t]+<fetch packet header 0xefe04000>
[0-9a-f]+[02468ace] <[^>]*> 040a[ \t]+shl \.S1 a0,16,a0
[0-9a-f]+[02468ace] <[^>]*> 251b[ \t]+shl \.S2 b2,1,b1
[0-9a-f]+[02468ace] <[^>]*> 362a[ \t]+shl \.S1X b4,1,a2
[0-9a-f]+[02468ace] <[^>]*> 573b[ \t]+shl \.S2X a6,2,b3
[0-9a-f]+[02468ace] <[^>]*> 444a[ \t]+shl \.S1 a0,2,a4
[0-9a-f]+[02468ace] <[^>]*> 655b[ \t]+shl \.S2 b2,3,b5
[0-9a-f]+[02468ace] <[^>]*> 766a[ \t]+shl \.S1X b4,3,a6
[0-9a-f]+[02468ace] <[^>]*> 9ffb[ \t]+shr \.S2X a7,4,b7
[0-9a-f]+[02468ace] <[^>]*> 8cea[ \t]+shr \.S1 a1,4,a6
[0-9a-f]+[02468ace] <[^>]*> addb[ \t]+shr \.S2 b3,5,b5
[0-9a-f]+[02468ace] <[^>]*> beca[ \t]+shr \.S1X b5,5,a4
[0-9a-f]+[02468ace] <[^>]*> dfbb[ \t]+shr \.S2X a7,6,b3
[0-9a-f]+[02468ace] <[^>]*> ccaa[ \t]+shr \.S1 a1,6,a2
[0-9a-f]+[02468ace] <[^>]*> ed9b[ \t]+shr \.S2 b3,8,b1
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 0012[ \t]+mvk \.S1 0,a0
[0-9a-f]+[02468ace] <[^>]*> 1113[ \t]+mvk \.S2 16,b2
[0-9a-f]+[02468ace] <[^>]*> 2232[ \t]+mvk \.S1 33,a4
[0-9a-f]+[02468ace] <[^>]*> 3333[ \t]+mvk \.S2 49,b6
[0-9a-f]+[02468ace] <[^>]*> 4752[ \t]+mvk \.S1 194,a6
[0-9a-f]+[02468ace] <[^>]*> 5653[ \t]+mvk \.S2 210,b4
[0-9a-f]+[02468ace] <[^>]*> 6572[ \t]+mvk \.S1 227,a2
[0-9a-f]+[02468ace] <[^>]*> 78f3[ \t]+mvk \.S2 123,b1
[0-9a-f]+[02468ace] <[^>]*> 8992[ \t]+mvk \.S1 12,a3
[0-9a-f]+[02468ace] <[^>]*> 9a93[ \t]+mvk \.S2 28,b5
[0-9a-f]+[02468ace] <[^>]*> abb2[ \t]+mvk \.S1 45,a7
[0-9a-f]+[02468ace] <[^>]*> bed2[ \t]+mvk \.S1 221,a5
[0-9a-f]+[02468ace] <[^>]*> cdf3[ \t]+mvk \.S2 238,b3
[0-9a-f]+[02468ace] <[^>]*> fc92[ \t]+mvk \.S1 159,a1
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 0402[ \t]+shl \.S1 a0,0,a0
[0-9a-f]+[02468ace] <[^>]*> 1503[ \t]+shl \.S2 b2,16,b2
[0-9a-f]+[02468ace] <[^>]*> 2682[ \t]+shl \.S1 a5,1,a5
[0-9a-f]+[02468ace] <[^>]*> 3783[ \t]+shl \.S2 b7,17,b7
[0-9a-f]+[02468ace] <[^>]*> 4c22[ \t]+shr \.S1 a0,10,a0
[0-9a-f]+[02468ace] <[^>]*> 5d23[ \t]+shr \.S2 b2,26,b2
[0-9a-f]+[02468ace] <[^>]*> 6ea2[ \t]+shr \.S1 a5,11,a5
[0-9a-f]+[02468ace] <[^>]*> 7fa3[ \t]+shr \.S2 b7,27,b7
[0-9a-f]+[02468ace] <[^>]*> 8442[ \t]+shru \.S1 a0,4,a0
[0-9a-f]+[02468ace] <[^>]*> 9543[ \t]+shru \.S2 b2,20,b2
[0-9a-f]+[02468ace] <[^>]*> a6c2[ \t]+shru \.S1 a5,5,a5
[0-9a-f]+[02468ace] <[^>]*> b7c3[ \t]+shru \.S2 b7,21,b7
[0-9a-f]+[02468ace] <[^>]*> cc42[ \t]+shru \.S1 a0,14,a0
[0-9a-f]+[02468ace] <[^>]*> dd43[ \t]+shru \.S2 b2,30,b2
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> ec02[ \t]+shl \.S1 a0,15,a0
[0-9a-f]+[02468ace] <[^>]*> fd03[ \t]+shl \.S2 b2,31,b2
[0-9a-f]+[02468ace] <[^>]*> e682[ \t]+shl \.S1 a5,7,a5
[0-9a-f]+[02468ace] <[^>]*> d783[ \t]+shl \.S2 b7,22,b7
[0-9a-f]+[02468ace] <[^>]*> c422[ \t]+shr \.S1 a0,6,a0
[0-9a-f]+[02468ace] <[^>]*> b523[ \t]+shr \.S2 b2,21,b2
[0-9a-f]+[02468ace] <[^>]*> aea2[ \t]+shr \.S1 a5,13,a5
[0-9a-f]+[02468ace] <[^>]*> 9fa3[ \t]+shr \.S2 b7,28,b7
[0-9a-f]+[02468ace] <[^>]*> 8c42[ \t]+sshl \.S1 a0,12,a0
[0-9a-f]+[02468ace] <[^>]*> 7d43[ \t]+sshl \.S2 b2,27,b2
[0-9a-f]+[02468ace] <[^>]*> 66c2[ \t]+sshl \.S1 a5,3,a5
[0-9a-f]+[02468ace] <[^>]*> 57c3[ \t]+sshl \.S2 b7,18,b7
[0-9a-f]+[02468ace] <[^>]*> 4442[ \t]+sshl \.S1 a0,2,a0
[0-9a-f]+[02468ace] <[^>]*> 3543[ \t]+sshl \.S2 b2,17,b2
[0-9a-f]+[02468ace] <[^>]*> efe04000[ \t]+<fetch packet header 0xefe04000>
[0-9a-f]+[02468ace] <[^>]*> 0462[ \t]+shl \.S1 a0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 2563[ \t]+shl \.S2 b2,b1,b2
[0-9a-f]+[02468ace] <[^>]*> 4662[ \t]+shl \.S1 a4,a2,a4
[0-9a-f]+[02468ace] <[^>]*> 6f63[ \t]+shr \.S2 b6,b3,b6
[0-9a-f]+[02468ace] <[^>]*> 8c62[ \t]+shr \.S1 a0,a4,a0
[0-9a-f]+[02468ace] <[^>]*> ad63[ \t]+shr \.S2 b2,b5,b2
[0-9a-f]+[02468ace] <[^>]*> ce62[ \t]+shr \.S1 a4,a6,a4
[0-9a-f]+[02468ace] <[^>]*> f7e3[ \t]+shru \.S2 b7,b7,b7
[0-9a-f]+[02468ace] <[^>]*> d4e2[ \t]+shru \.S1 a1,a6,a1
[0-9a-f]+[02468ace] <[^>]*> b5e3[ \t]+shru \.S2 b3,b5,b3
[0-9a-f]+[02468ace] <[^>]*> 96e2[ \t]+shru \.S1 a5,a4,a5
[0-9a-f]+[02468ace] <[^>]*> 7fe3[ \t]+sshl \.S2 b7,b3,b7
[0-9a-f]+[02468ace] <[^>]*> 5ce2[ \t]+sshl \.S1 a1,a2,a1
[0-9a-f]+[02468ace] <[^>]*> 3de3[ \t]+sshl \.S2 b3,b1,b3
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 0002[ \t]+extu \.S1 a0,0,31,a0
[0-9a-f]+[02468ace] <[^>]*> 1103[ \t]+extu \.S2 b2,16,31,b0
[0-9a-f]+[02468ace] <[^>]*> 2202[ \t]+extu \.S1 a4,1,31,a0
[0-9a-f]+[02468ace] <[^>]*> 3303[ \t]+extu \.S2 b6,17,31,b0
[0-9a-f]+[02468ace] <[^>]*> 4a22[ \t]+set \.S1 a4,10,10,a4
[0-9a-f]+[02468ace] <[^>]*> 5923[ \t]+set \.S2 b2,26,26,b2
[0-9a-f]+[02468ace] <[^>]*> 6822[ \t]+set \.S1 a0,11,11,a0
[0-9a-f]+[02468ace] <[^>]*> 71a3[ \t]+set \.S2 b3,19,19,b3
[0-9a-f]+[02468ace] <[^>]*> 82a2[ \t]+set \.S1 a5,4,4,a5
[0-9a-f]+[02468ace] <[^>]*> 93c3[ \t]+clr \.S2 b7,20,20,b7
[0-9a-f]+[02468ace] <[^>]*> a2c2[ \t]+clr \.S1 a5,5,5,a5
[0-9a-f]+[02468ace] <[^>]*> b9c3[ \t]+clr \.S2 b3,29,29,b3
[0-9a-f]+[02468ace] <[^>]*> c8c2[ \t]+clr \.S1 a1,14,14,a1
[0-9a-f]+[02468ace] <[^>]*> f9c3[ \t]+clr \.S2 b3,31,31,b3
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 0062[ \t]+ext \.S1 a0,16,16,a0
[0-9a-f]+[02468ace] <[^>]*> 2163[ \t]+ext \.S2 b2,16,16,b1
[0-9a-f]+[02468ace] <[^>]*> 4262[ \t]+ext \.S1 a4,16,16,a2
[0-9a-f]+[02468ace] <[^>]*> 6b63[ \t]+ext \.S2 b6,24,24,b3
[0-9a-f]+[02468ace] <[^>]*> 8862[ \t]+ext \.S1 a0,24,24,a4
[0-9a-f]+[02468ace] <[^>]*> a963[ \t]+ext \.S2 b2,24,24,b5
[0-9a-f]+[02468ace] <[^>]*> ca62[ \t]+ext \.S1 a4,24,24,a6
[0-9a-f]+[02468ace] <[^>]*> f3e3[ \t]+extu \.S2 b7,16,16,b7
[0-9a-f]+[02468ace] <[^>]*> d0e2[ \t]+extu \.S1 a1,16,16,a6
[0-9a-f]+[02468ace] <[^>]*> b1e3[ \t]+extu \.S2 b3,16,16,b5
[0-9a-f]+[02468ace] <[^>]*> 9ae2[ \t]+extu \.S1 a5,24,24,a4
[0-9a-f]+[02468ace] <[^>]*> 7be3[ \t]+extu \.S2 b7,24,24,b3
[0-9a-f]+[02468ace] <[^>]*> 58e2[ \t]+extu \.S1 a1,24,24,a2
[0-9a-f]+[02468ace] <[^>]*> 39e3[ \t]+extu \.S2 b3,24,24,b1
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 002e[ \t]+add \.S1 a0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 212f[ \t]+add \.S2 b1,b2,b1
[0-9a-f]+[02468ace] <[^>]*> 522e[ \t]+add \.S1X a2,b4,a2
[0-9a-f]+[02468ace] <[^>]*> 732f[ \t]+add \.S2X b3,a6,b3
[0-9a-f]+[02468ace] <[^>]*> 802e[ \t]+add \.S1 a4,a0,a4
[0-9a-f]+[02468ace] <[^>]*> a12f[ \t]+add \.S2 b5,b2,b5
[0-9a-f]+[02468ace] <[^>]*> d22e[ \t]+add \.S1X a6,b4,a6
[0-9a-f]+[02468ace] <[^>]*> fb2f[ \t]+sub \.S2X b7,a6,b7
[0-9a-f]+[02468ace] <[^>]*> 082e[ \t]+sub \.S1 a0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 292f[ \t]+sub \.S2 b1,b2,b1
[0-9a-f]+[02468ace] <[^>]*> 5a2e[ \t]+sub \.S1X a2,b4,a2
[0-9a-f]+[02468ace] <[^>]*> 7b2f[ \t]+sub \.S2X b3,a6,b3
[0-9a-f]+[02468ace] <[^>]*> 882e[ \t]+sub \.S1 a4,a0,a4
[0-9a-f]+[02468ace] <[^>]*> a92f[ \t]+sub \.S2 b5,b2,b5
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 042e[ \t]+addk \.S1 0,a0
[0-9a-f]+[02468ace] <[^>]*> 152f[ \t]+addk \.S2 16,b2
[0-9a-f]+[02468ace] <[^>]*> 262e[ \t]+addk \.S1 1,a4
[0-9a-f]+[02468ace] <[^>]*> 372f[ \t]+addk \.S2 17,b6
[0-9a-f]+[02468ace] <[^>]*> 4c2e[ \t]+addk \.S1 10,a0
[0-9a-f]+[02468ace] <[^>]*> 5d2f[ \t]+addk \.S2 26,b2
[0-9a-f]+[02468ace] <[^>]*> 6e2e[ \t]+addk \.S1 11,a4
[0-9a-f]+[02468ace] <[^>]*> 77af[ \t]+addk \.S2 19,b7
[0-9a-f]+[02468ace] <[^>]*> 84ae[ \t]+addk \.S1 4,a1
[0-9a-f]+[02468ace] <[^>]*> 95af[ \t]+addk \.S2 20,b3
[0-9a-f]+[02468ace] <[^>]*> aeae[ \t]+addk \.S1 13,a5
[0-9a-f]+[02468ace] <[^>]*> bfaf[ \t]+addk \.S2 29,b7
[0-9a-f]+[02468ace] <[^>]*> ccae[ \t]+addk \.S1 14,a1
[0-9a-f]+[02468ace] <[^>]*> fdaf[ \t]+addk \.S2 31,b3
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 586e[ \t]+sub \.S1 0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 596f[ \t]+sub \.S2 0,b2,b2
[0-9a-f]+[02468ace] <[^>]*> 5a6e[ \t]+sub \.S1 0,a4,a4
[0-9a-f]+[02468ace] <[^>]*> 5b6f[ \t]+sub \.S2 0,b6,b6
[0-9a-f]+[02468ace] <[^>]*> 586e[ \t]+sub \.S1 0,a0,a0
[0-9a-f]+[02468ace] <[^>]*> 796f[ \t]+add \.S2 -1,b2,b2
[0-9a-f]+[02468ace] <[^>]*> 7a6e[ \t]+add \.S1 -1,a4,a4
[0-9a-f]+[02468ace] <[^>]*> 7bef[ \t]+add \.S2 -1,b7,b7
[0-9a-f]+[02468ace] <[^>]*> 78ee[ \t]+add \.S1 -1,a1,a1
[0-9a-f]+[02468ace] <[^>]*> 79ef[ \t]+add \.S2 -1,b3,b3
[0-9a-f]+[02468ace] <[^>]*> daee[ \t]+mvc \.S1 b5,ilc
[0-9a-f]+[02468ace] <[^>]*> dbef[ \t]+mvc \.S2 b7,ilc
[0-9a-f]+[02468ace] <[^>]*> d8ee[ \t]+mvc \.S1 b1,ilc
[0-9a-f]+[02468ace] <[^>]*> d9ef[ \t]+mvc \.S2 b3,ilc
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 586e[ \t]+sub \.S1 0,a16,a16
[0-9a-f]+[02468ace] <[^>]*> 596f[ \t]+sub \.S2 0,b18,b18
[0-9a-f]+[02468ace] <[^>]*> 5a6e[ \t]+sub \.S1 0,a20,a20
[0-9a-f]+[02468ace] <[^>]*> 5b6f[ \t]+sub \.S2 0,b22,b22
[0-9a-f]+[02468ace] <[^>]*> 586e[ \t]+sub \.S1 0,a16,a16
[0-9a-f]+[02468ace] <[^>]*> 796f[ \t]+add \.S2 -1,b18,b18
[0-9a-f]+[02468ace] <[^>]*> 7a6e[ \t]+add \.S1 -1,a20,a20
[0-9a-f]+[02468ace] <[^>]*> 7bef[ \t]+add \.S2 -1,b23,b23
[0-9a-f]+[02468ace] <[^>]*> 78ee[ \t]+add \.S1 -1,a17,a17
[0-9a-f]+[02468ace] <[^>]*> 79ef[ \t]+add \.S2 -1,b19,b19
[0-9a-f]+[02468ace] <[^>]*> daee[ \t]+mvc \.S1 b21,ilc
[0-9a-f]+[02468ace] <[^>]*> dbef[ \t]+mvc \.S2 b23,ilc
[0-9a-f]+[02468ace] <[^>]*> d8ee[ \t]+mvc \.S1 b17,ilc
[0-9a-f]+[02468ace] <[^>]*> d9ef[ \t]+mvc \.S2 b19,ilc
[0-9a-f]+[02468ace] <[^>]*> efe80000[ \t]+<fetch packet header 0xefe80000>
[0-9a-f]+[02468ace] <[^>]*> 006e[ \t]+bnop \.S1 b0,0
[0-9a-f]+[02468ace] <[^>]*> 216f[ \t]+bnop \.S2 b2,1
[0-9a-f]+[02468ace] <[^>]*> 22ee[ \t]+bnop \.S1 b5,1
[0-9a-f]+[02468ace] <[^>]*> 43ef[ \t]+bnop \.S2 b7,2
[0-9a-f]+[02468ace] <[^>]*> 446e[ \t]+bnop \.S1 b8,2
[0-9a-f]+[02468ace] <[^>]*> 656f[ \t]+bnop \.S2 b10,3
[0-9a-f]+[02468ace] <[^>]*> 66ee[ \t]+bnop \.S1 b13,3
[0-9a-f]+[02468ace] <[^>]*> 87ef[ \t]+bnop \.S2 b15,4
[0-9a-f]+[02468ace] <[^>]*> 866e[ \t]+bnop \.S1 b12,4
[0-9a-f]+[02468ace] <[^>]*> a56f[ \t]+bnop \.S2 b10,5
[0-9a-f]+[02468ace] <[^>]*> a4ee[ \t]+bnop \.S1 b9,5
[0-9a-f]+[02468ace] <[^>]*> c3ef[ \t]+bnop \.S2 b7,6
[0-9a-f]+[02468ace] <[^>]*> c26e[ \t]+bnop \.S1 b4,6
[0-9a-f]+[02468ace] <[^>]*> e16f[ \t]+bnop \.S2 b2,7
[0-9a-f]+[02468ace] <[^>]*> efe00000[ \t]+<fetch packet header 0xefe00000>
[0-9a-f]+[02468ace] <[^>]*> 006e[ \t]+bnop \.S1 b0,0
[0-9a-f]+[02468ace] <[^>]*> 216f[ \t]+bnop \.S2 b2,1
[0-9a-f]+[02468ace] <[^>]*> 22ee[ \t]+bnop \.S1 b5,1
[0-9a-f]+[02468ace] <[^>]*> 43ef[ \t]+bnop \.S2 b7,2
[0-9a-f]+[02468ace] <[^>]*> 446e[ \t]+bnop \.S1 b8,2
[0-9a-f]+[02468ace] <[^>]*> 656f[ \t]+bnop \.S2 b10,3
[0-9a-f]+[02468ace] <[^>]*> 66ee[ \t]+bnop \.S1 b13,3
[0-9a-f]+[02468ace] <[^>]*> 87ef[ \t]+bnop \.S2 b15,4
[0-9a-f]+[02468ace] <[^>]*> 866e[ \t]+bnop \.S1 b12,4
[0-9a-f]+[02468ace] <[^>]*> a56f[ \t]+bnop \.S2 b10,5
[0-9a-f]+[02468ace] <[^>]*> a4ee[ \t]+bnop \.S1 b9,5
[0-9a-f]+[02468ace] <[^>]*> c3ef[ \t]+bnop \.S2 b7,6
[0-9a-f]+[02468ace] <[^>]*> c26e[ \t]+bnop \.S1 b4,6
[0-9a-f]+[02468ace] <[^>]*> e16f[ \t]+bnop \.S2 b2,7
[0-9a-f]+[02468ace] <[^>]*> efe80000[ \t]+<fetch packet header 0xefe80000>
[ \t]*\.\.\.
