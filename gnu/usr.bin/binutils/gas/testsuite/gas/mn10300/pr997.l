GAS LISTING .*/pr997.s.*page 1


   1.*.data
   2.*
   3 0000 68656C6C.*msg:.*.asciz  "hello world.\\n"
   3      6F20776F 
   3      726C642E 
   3      0A00
   4.*msglen = .-msg-1
   5.*msglen=msglen & 0xff

.*GAS LISTING.*/pr997.s.*page 2


DEFINED SYMBOLS
.*/pr997.s:3.*.data:0+00 msg
.*/pr997.s:4.*\*ABS\*:0+0d msglen

NO UNDEFINED SYMBOLS
