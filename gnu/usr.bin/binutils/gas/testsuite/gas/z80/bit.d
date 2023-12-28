#objdump: -d
#name: bit manipulations

.*: .*

Disassembly of section .text:

0+ <.text>:

[ 	]+[0-9a-f]+:[ 	]+cb 47[ 	]+bit 0,a
[ 	]+[0-9a-f]+:[ 	]+cb 40[ 	]+bit 0,b
[ 	]+[0-9a-f]+:[ 	]+cb 41[ 	]+bit 0,c
[ 	]+[0-9a-f]+:[ 	]+cb 42[ 	]+bit 0,d
[ 	]+[0-9a-f]+:[ 	]+cb 43[ 	]+bit 0,e
[ 	]+[0-9a-f]+:[ 	]+cb 44[ 	]+bit 0,h
[ 	]+[0-9a-f]+:[ 	]+cb 45[ 	]+bit 0,l
[ 	]+[0-9a-f]+:[ 	]+cb 46[ 	]+bit 0,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 46[ 	]+bit 0,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 46[ 	]+bit 0,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 4f[ 	]+bit 1,a
[ 	]+[0-9a-f]+:[ 	]+cb 48[ 	]+bit 1,b
[ 	]+[0-9a-f]+:[ 	]+cb 49[ 	]+bit 1,c
[ 	]+[0-9a-f]+:[ 	]+cb 4a[ 	]+bit 1,d
[ 	]+[0-9a-f]+:[ 	]+cb 4b[ 	]+bit 1,e
[ 	]+[0-9a-f]+:[ 	]+cb 4c[ 	]+bit 1,h
[ 	]+[0-9a-f]+:[ 	]+cb 4d[ 	]+bit 1,l
[ 	]+[0-9a-f]+:[ 	]+cb 4e[ 	]+bit 1,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 4e[ 	]+bit 1,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 4e[ 	]+bit 1,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 57[ 	]+bit 2,a
[ 	]+[0-9a-f]+:[ 	]+cb 50[ 	]+bit 2,b
[ 	]+[0-9a-f]+:[ 	]+cb 51[ 	]+bit 2,c
[ 	]+[0-9a-f]+:[ 	]+cb 52[ 	]+bit 2,d
[ 	]+[0-9a-f]+:[ 	]+cb 53[ 	]+bit 2,e
[ 	]+[0-9a-f]+:[ 	]+cb 54[ 	]+bit 2,h
[ 	]+[0-9a-f]+:[ 	]+cb 55[ 	]+bit 2,l
[ 	]+[0-9a-f]+:[ 	]+cb 56[ 	]+bit 2,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 56[ 	]+bit 2,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 56[ 	]+bit 2,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 5f[ 	]+bit 3,a
[ 	]+[0-9a-f]+:[ 	]+cb 58[ 	]+bit 3,b
[ 	]+[0-9a-f]+:[ 	]+cb 59[ 	]+bit 3,c
[ 	]+[0-9a-f]+:[ 	]+cb 5a[ 	]+bit 3,d
[ 	]+[0-9a-f]+:[ 	]+cb 5b[ 	]+bit 3,e
[ 	]+[0-9a-f]+:[ 	]+cb 5c[ 	]+bit 3,h
[ 	]+[0-9a-f]+:[ 	]+cb 5d[ 	]+bit 3,l
[ 	]+[0-9a-f]+:[ 	]+cb 5e[ 	]+bit 3,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 5e[ 	]+bit 3,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 5e[ 	]+bit 3,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 67[ 	]+bit 4,a
[ 	]+[0-9a-f]+:[ 	]+cb 60[ 	]+bit 4,b
[ 	]+[0-9a-f]+:[ 	]+cb 61[ 	]+bit 4,c
[ 	]+[0-9a-f]+:[ 	]+cb 62[ 	]+bit 4,d
[ 	]+[0-9a-f]+:[ 	]+cb 63[ 	]+bit 4,e
[ 	]+[0-9a-f]+:[ 	]+cb 64[ 	]+bit 4,h
[ 	]+[0-9a-f]+:[ 	]+cb 65[ 	]+bit 4,l
[ 	]+[0-9a-f]+:[ 	]+cb 66[ 	]+bit 4,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 66[ 	]+bit 4,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 66[ 	]+bit 4,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 6f[ 	]+bit 5,a
[ 	]+[0-9a-f]+:[ 	]+cb 68[ 	]+bit 5,b
[ 	]+[0-9a-f]+:[ 	]+cb 69[ 	]+bit 5,c
[ 	]+[0-9a-f]+:[ 	]+cb 6a[ 	]+bit 5,d
[ 	]+[0-9a-f]+:[ 	]+cb 6b[ 	]+bit 5,e
[ 	]+[0-9a-f]+:[ 	]+cb 6c[ 	]+bit 5,h
[ 	]+[0-9a-f]+:[ 	]+cb 6d[ 	]+bit 5,l
[ 	]+[0-9a-f]+:[ 	]+cb 6e[ 	]+bit 5,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 6e[ 	]+bit 5,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 6e[ 	]+bit 5,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 77[ 	]+bit 6,a
[ 	]+[0-9a-f]+:[ 	]+cb 70[ 	]+bit 6,b
[ 	]+[0-9a-f]+:[ 	]+cb 71[ 	]+bit 6,c
[ 	]+[0-9a-f]+:[ 	]+cb 72[ 	]+bit 6,d
[ 	]+[0-9a-f]+:[ 	]+cb 73[ 	]+bit 6,e
[ 	]+[0-9a-f]+:[ 	]+cb 74[ 	]+bit 6,h
[ 	]+[0-9a-f]+:[ 	]+cb 75[ 	]+bit 6,l
[ 	]+[0-9a-f]+:[ 	]+cb 76[ 	]+bit 6,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 76[ 	]+bit 6,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 76[ 	]+bit 6,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 7f[ 	]+bit 7,a
[ 	]+[0-9a-f]+:[ 	]+cb 78[ 	]+bit 7,b
[ 	]+[0-9a-f]+:[ 	]+cb 79[ 	]+bit 7,c
[ 	]+[0-9a-f]+:[ 	]+cb 7a[ 	]+bit 7,d
[ 	]+[0-9a-f]+:[ 	]+cb 7b[ 	]+bit 7,e
[ 	]+[0-9a-f]+:[ 	]+cb 7c[ 	]+bit 7,h
[ 	]+[0-9a-f]+:[ 	]+cb 7d[ 	]+bit 7,l
[ 	]+[0-9a-f]+:[ 	]+cb 7e[ 	]+bit 7,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 7e[ 	]+bit 7,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 7e[ 	]+bit 7,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 87[ 	]+res 0,a
[ 	]+[0-9a-f]+:[ 	]+cb 80[ 	]+res 0,b
[ 	]+[0-9a-f]+:[ 	]+cb 81[ 	]+res 0,c
[ 	]+[0-9a-f]+:[ 	]+cb 82[ 	]+res 0,d
[ 	]+[0-9a-f]+:[ 	]+cb 83[ 	]+res 0,e
[ 	]+[0-9a-f]+:[ 	]+cb 84[ 	]+res 0,h
[ 	]+[0-9a-f]+:[ 	]+cb 85[ 	]+res 0,l
[ 	]+[0-9a-f]+:[ 	]+cb 86[ 	]+res 0,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 86[ 	]+res 0,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 86[ 	]+res 0,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 8f[ 	]+res 1,a
[ 	]+[0-9a-f]+:[ 	]+cb 88[ 	]+res 1,b
[ 	]+[0-9a-f]+:[ 	]+cb 89[ 	]+res 1,c
[ 	]+[0-9a-f]+:[ 	]+cb 8a[ 	]+res 1,d
[ 	]+[0-9a-f]+:[ 	]+cb 8b[ 	]+res 1,e
[ 	]+[0-9a-f]+:[ 	]+cb 8c[ 	]+res 1,h
[ 	]+[0-9a-f]+:[ 	]+cb 8d[ 	]+res 1,l
[ 	]+[0-9a-f]+:[ 	]+cb 8e[ 	]+res 1,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 8e[ 	]+res 1,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 8e[ 	]+res 1,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 97[ 	]+res 2,a
[ 	]+[0-9a-f]+:[ 	]+cb 90[ 	]+res 2,b
[ 	]+[0-9a-f]+:[ 	]+cb 91[ 	]+res 2,c
[ 	]+[0-9a-f]+:[ 	]+cb 92[ 	]+res 2,d
[ 	]+[0-9a-f]+:[ 	]+cb 93[ 	]+res 2,e
[ 	]+[0-9a-f]+:[ 	]+cb 94[ 	]+res 2,h
[ 	]+[0-9a-f]+:[ 	]+cb 95[ 	]+res 2,l
[ 	]+[0-9a-f]+:[ 	]+cb 96[ 	]+res 2,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 96[ 	]+res 2,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 96[ 	]+res 2,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 9f[ 	]+res 3,a
[ 	]+[0-9a-f]+:[ 	]+cb 98[ 	]+res 3,b
[ 	]+[0-9a-f]+:[ 	]+cb 99[ 	]+res 3,c
[ 	]+[0-9a-f]+:[ 	]+cb 9a[ 	]+res 3,d
[ 	]+[0-9a-f]+:[ 	]+cb 9b[ 	]+res 3,e
[ 	]+[0-9a-f]+:[ 	]+cb 9c[ 	]+res 3,h
[ 	]+[0-9a-f]+:[ 	]+cb 9d[ 	]+res 3,l
[ 	]+[0-9a-f]+:[ 	]+cb 9e[ 	]+res 3,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 9e[ 	]+res 3,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 9e[ 	]+res 3,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb a7[ 	]+res 4,a
[ 	]+[0-9a-f]+:[ 	]+cb a0[ 	]+res 4,b
[ 	]+[0-9a-f]+:[ 	]+cb a1[ 	]+res 4,c
[ 	]+[0-9a-f]+:[ 	]+cb a2[ 	]+res 4,d
[ 	]+[0-9a-f]+:[ 	]+cb a3[ 	]+res 4,e
[ 	]+[0-9a-f]+:[ 	]+cb a4[ 	]+res 4,h
[ 	]+[0-9a-f]+:[ 	]+cb a5[ 	]+res 4,l
[ 	]+[0-9a-f]+:[ 	]+cb a6[ 	]+res 4,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 a6[ 	]+res 4,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 a6[ 	]+res 4,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb af[ 	]+res 5,a
[ 	]+[0-9a-f]+:[ 	]+cb a8[ 	]+res 5,b
[ 	]+[0-9a-f]+:[ 	]+cb a9[ 	]+res 5,c
[ 	]+[0-9a-f]+:[ 	]+cb aa[ 	]+res 5,d
[ 	]+[0-9a-f]+:[ 	]+cb ab[ 	]+res 5,e
[ 	]+[0-9a-f]+:[ 	]+cb ac[ 	]+res 5,h
[ 	]+[0-9a-f]+:[ 	]+cb ad[ 	]+res 5,l
[ 	]+[0-9a-f]+:[ 	]+cb ae[ 	]+res 5,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 ae[ 	]+res 5,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 ae[ 	]+res 5,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb b7[ 	]+res 6,a
[ 	]+[0-9a-f]+:[ 	]+cb b0[ 	]+res 6,b
[ 	]+[0-9a-f]+:[ 	]+cb b1[ 	]+res 6,c
[ 	]+[0-9a-f]+:[ 	]+cb b2[ 	]+res 6,d
[ 	]+[0-9a-f]+:[ 	]+cb b3[ 	]+res 6,e
[ 	]+[0-9a-f]+:[ 	]+cb b4[ 	]+res 6,h
[ 	]+[0-9a-f]+:[ 	]+cb b5[ 	]+res 6,l
[ 	]+[0-9a-f]+:[ 	]+cb b6[ 	]+res 6,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 b6[ 	]+res 6,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 b6[ 	]+res 6,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb bf[ 	]+res 7,a
[ 	]+[0-9a-f]+:[ 	]+cb b8[ 	]+res 7,b
[ 	]+[0-9a-f]+:[ 	]+cb b9[ 	]+res 7,c
[ 	]+[0-9a-f]+:[ 	]+cb ba[ 	]+res 7,d
[ 	]+[0-9a-f]+:[ 	]+cb bb[ 	]+res 7,e
[ 	]+[0-9a-f]+:[ 	]+cb bc[ 	]+res 7,h
[ 	]+[0-9a-f]+:[ 	]+cb bd[ 	]+res 7,l
[ 	]+[0-9a-f]+:[ 	]+cb be[ 	]+res 7,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 be[ 	]+res 7,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 be[ 	]+res 7,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb c7[ 	]+set 0,a
[ 	]+[0-9a-f]+:[ 	]+cb c0[ 	]+set 0,b
[ 	]+[0-9a-f]+:[ 	]+cb c1[ 	]+set 0,c
[ 	]+[0-9a-f]+:[ 	]+cb c2[ 	]+set 0,d
[ 	]+[0-9a-f]+:[ 	]+cb c3[ 	]+set 0,e
[ 	]+[0-9a-f]+:[ 	]+cb c4[ 	]+set 0,h
[ 	]+[0-9a-f]+:[ 	]+cb c5[ 	]+set 0,l
[ 	]+[0-9a-f]+:[ 	]+cb c6[ 	]+set 0,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 c6[ 	]+set 0,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 c6[ 	]+set 0,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb cf[ 	]+set 1,a
[ 	]+[0-9a-f]+:[ 	]+cb c8[ 	]+set 1,b
[ 	]+[0-9a-f]+:[ 	]+cb c9[ 	]+set 1,c
[ 	]+[0-9a-f]+:[ 	]+cb ca[ 	]+set 1,d
[ 	]+[0-9a-f]+:[ 	]+cb cb[ 	]+set 1,e
[ 	]+[0-9a-f]+:[ 	]+cb cc[ 	]+set 1,h
[ 	]+[0-9a-f]+:[ 	]+cb cd[ 	]+set 1,l
[ 	]+[0-9a-f]+:[ 	]+cb ce[ 	]+set 1,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 ce[ 	]+set 1,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 ce[ 	]+set 1,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb d7[ 	]+set 2,a
[ 	]+[0-9a-f]+:[ 	]+cb d0[ 	]+set 2,b
[ 	]+[0-9a-f]+:[ 	]+cb d1[ 	]+set 2,c
[ 	]+[0-9a-f]+:[ 	]+cb d2[ 	]+set 2,d
[ 	]+[0-9a-f]+:[ 	]+cb d3[ 	]+set 2,e
[ 	]+[0-9a-f]+:[ 	]+cb d4[ 	]+set 2,h
[ 	]+[0-9a-f]+:[ 	]+cb d5[ 	]+set 2,l
[ 	]+[0-9a-f]+:[ 	]+cb d6[ 	]+set 2,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 d6[ 	]+set 2,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 d6[ 	]+set 2,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb df[ 	]+set 3,a
[ 	]+[0-9a-f]+:[ 	]+cb d8[ 	]+set 3,b
[ 	]+[0-9a-f]+:[ 	]+cb d9[ 	]+set 3,c
[ 	]+[0-9a-f]+:[ 	]+cb da[ 	]+set 3,d
[ 	]+[0-9a-f]+:[ 	]+cb db[ 	]+set 3,e
[ 	]+[0-9a-f]+:[ 	]+cb dc[ 	]+set 3,h
[ 	]+[0-9a-f]+:[ 	]+cb dd[ 	]+set 3,l
[ 	]+[0-9a-f]+:[ 	]+cb de[ 	]+set 3,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 de[ 	]+set 3,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 de[ 	]+set 3,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb e7[ 	]+set 4,a
[ 	]+[0-9a-f]+:[ 	]+cb e0[ 	]+set 4,b
[ 	]+[0-9a-f]+:[ 	]+cb e1[ 	]+set 4,c
[ 	]+[0-9a-f]+:[ 	]+cb e2[ 	]+set 4,d
[ 	]+[0-9a-f]+:[ 	]+cb e3[ 	]+set 4,e
[ 	]+[0-9a-f]+:[ 	]+cb e4[ 	]+set 4,h
[ 	]+[0-9a-f]+:[ 	]+cb e5[ 	]+set 4,l
[ 	]+[0-9a-f]+:[ 	]+cb e6[ 	]+set 4,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 e6[ 	]+set 4,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 e6[ 	]+set 4,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb ef[ 	]+set 5,a
[ 	]+[0-9a-f]+:[ 	]+cb e8[ 	]+set 5,b
[ 	]+[0-9a-f]+:[ 	]+cb e9[ 	]+set 5,c
[ 	]+[0-9a-f]+:[ 	]+cb ea[ 	]+set 5,d
[ 	]+[0-9a-f]+:[ 	]+cb eb[ 	]+set 5,e
[ 	]+[0-9a-f]+:[ 	]+cb ec[ 	]+set 5,h
[ 	]+[0-9a-f]+:[ 	]+cb ed[ 	]+set 5,l
[ 	]+[0-9a-f]+:[ 	]+cb ee[ 	]+set 5,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 ee[ 	]+set 5,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 ee[ 	]+set 5,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb f7[ 	]+set 6,a
[ 	]+[0-9a-f]+:[ 	]+cb f0[ 	]+set 6,b
[ 	]+[0-9a-f]+:[ 	]+cb f1[ 	]+set 6,c
[ 	]+[0-9a-f]+:[ 	]+cb f2[ 	]+set 6,d
[ 	]+[0-9a-f]+:[ 	]+cb f3[ 	]+set 6,e
[ 	]+[0-9a-f]+:[ 	]+cb f4[ 	]+set 6,h
[ 	]+[0-9a-f]+:[ 	]+cb f5[ 	]+set 6,l
[ 	]+[0-9a-f]+:[ 	]+cb f6[ 	]+set 6,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 f6[ 	]+set 6,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 f6[ 	]+set 6,\(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb ff[ 	]+set 7,a
[ 	]+[0-9a-f]+:[ 	]+cb f8[ 	]+set 7,b
[ 	]+[0-9a-f]+:[ 	]+cb f9[ 	]+set 7,c
[ 	]+[0-9a-f]+:[ 	]+cb fa[ 	]+set 7,d
[ 	]+[0-9a-f]+:[ 	]+cb fb[ 	]+set 7,e
[ 	]+[0-9a-f]+:[ 	]+cb fc[ 	]+set 7,h
[ 	]+[0-9a-f]+:[ 	]+cb fd[ 	]+set 7,l
[ 	]+[0-9a-f]+:[ 	]+cb fe[ 	]+set 7,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 fe[ 	]+set 7,\(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 fe[ 	]+set 7,\(iy\+5\)
