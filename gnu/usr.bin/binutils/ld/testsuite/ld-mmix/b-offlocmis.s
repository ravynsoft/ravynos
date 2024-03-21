% The .text contents is supposed to be linked --oformat binary with
% b-post1.s and b-goodmain.s.  The code below will provide a LOP_LOC
% with a 64-bit address (0x789abcdef012345b) then 16 bytes of % random
% data.  Note that the address is misaligned and the contents should
% be handled as at 0x789abcdef0123458.  After that, there's another
% LOP_LOC, about 32 bytes further on, also at a misaligned address:
% this time the data (0x12345677) is entered with a LOP_QUOTE.

 .text
 .byte 0x98,1,0,2
 .8byte 0x789abcdef012345b
 .byte 0xb0,0x45,0x19,0x7d,0x2c,0x1b,0x3,0xb2
 .byte 0xe4,0xdb,0xf8,0x77,0xf,0xc7,0x66,0xfb
 .byte 0x98,1,0,2
 .8byte 0x789abcdef012347a
 .byte 0x98,0,0,1
 .byte 0x12,0x34,0x56,0x77
