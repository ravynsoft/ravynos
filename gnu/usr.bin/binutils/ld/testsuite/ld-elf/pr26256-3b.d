#source: pr26256-3.s
#ld: -e _start -T pr26256-3b.t
#readelf: -x .rodata -x .text

Hex dump of section \'.rodata\':
  0x[a-f0-9]+ +00020301 +040907 +.+

Hex dump of section \'.text\':
  0x[a-f0-9]+ +22222222 +22222222 +22222222 +.+
  0x[a-f0-9]+ +11111111 +11111111 +11111111 +.+
  0x[a-f0-9]+ +33333333 +33333333 +33333333 +.+
