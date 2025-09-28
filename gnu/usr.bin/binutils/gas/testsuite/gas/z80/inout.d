#objdump: -d
#name: input and output

.*: .*

Disassembly of section .text:

0+ <.text>:
[ 	]+0:[ 	]+db 76[ 	]+in a,\(0x76\)
[ 	]+2:[ 	]+ed 78[ 	]+in a,\(c\)
[ 	]+4:[ 	]+ed 40[ 	]+in b,\(c\)
[ 	]+6:[ 	]+ed 48[ 	]+in c,\(c\)
[ 	]+8:[ 	]+ed 50[ 	]+in d,\(c\)
[ 	]+a:[ 	]+ed 58[ 	]+in e,\(c\)
[ 	]+c:[ 	]+ed 60[ 	]+in h,\(c\)
[ 	]+e:[ 	]+ed 68[ 	]+in l,\(c\)
[ 	]+10:[ 	]+d3 76[ 	]+out \(0x76\),a
[ 	]+12:[ 	]+ed 79[ 	]+out \(c\),a
[ 	]+14:[ 	]+ed 41[ 	]+out \(c\),b
[ 	]+16:[ 	]+ed 49[ 	]+out \(c\),c
[ 	]+18:[ 	]+ed 51[ 	]+out \(c\),d
[ 	]+1a:[ 	]+ed 59[ 	]+out \(c\),e
[ 	]+1c:[ 	]+ed 61[ 	]+out \(c\),h
[ 	]+1e:[ 	]+ed 69[ 	]+out \(c\),l
