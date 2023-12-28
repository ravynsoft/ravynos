#objdump: -dw
#name: i386 VMX

.*: +file format .*

Disassembly of section .text:

0+000 <foo>:
   0:	0f 01 c1 [ 	]*vmcall
   3:	0f 01 c2 [ 	]*vmlaunch
   6:	0f 01 c3 [ 	]*vmresume
   9:	0f 01 c4 [ 	]*vmxoff
   c:	66 0f c7 30 [ 	]*vmclear \(%eax\)
  10:	0f c7 30 [ 	]*vmptrld \(%eax\)
  13:	0f c7 38 [ 	]*vmptrst \(%eax\)
  16:	f3 0f c7 30 [ 	]*vmxon  \(%eax\)
  1a:	0f 78 c3 [ 	]*vmread %eax,%ebx
  1d:	0f 78 c3 [ 	]*vmread %eax,%ebx
  20:	0f 78 03 [ 	]*vmread %eax,\(%ebx\)
  23:	0f 78 03 [ 	]*vmread %eax,\(%ebx\)
  26:	0f 79 d8 [ 	]*vmwrite %eax,%ebx
  29:	0f 79 d8 [ 	]*vmwrite %eax,%ebx
  2c:	0f 79 18 [ 	]*vmwrite \(%eax\),%ebx
  2f:	0f 79 18 [ 	]*vmwrite \(%eax\),%ebx
[ 	]*[a-f0-9]+:	0f 01 c1[ 	]*vmcall
[ 	]*[a-f0-9]+:	0f 01 c2[ 	]*vmlaunch
[ 	]*[a-f0-9]+:	0f 01 c3[ 	]*vmresume
[ 	]*[a-f0-9]+:	0f 01 c4[ 	]*vmxoff
[ 	]*[a-f0-9]+:	67 66 0f c7 30[ 	]*vmclear \(%bx,%si\)
[ 	]*[a-f0-9]+:	67 0f c7 30[ 	]*vmptrld \(%bx,%si\)
[ 	]*[a-f0-9]+:	67 0f c7 38[ 	]*vmptrst \(%bx,%si\)
[ 	]*[a-f0-9]+:	67 f3 0f c7 30[ 	]*vmxon  \(%bx,%si\)
[ 	]*[a-f0-9]+:	0f 78 c3[ 	]*vmread %eax,%ebx
[ 	]*[a-f0-9]+:	0f 78 c3[ 	]*vmread %eax,%ebx
[ 	]*[a-f0-9]+:	67 0f 78 03[ 	]*vmread %eax,\(%bp,%di\)
[ 	]*[a-f0-9]+:	67 0f 78 03[ 	]*vmread %eax,\(%bp,%di\)
[ 	]*[a-f0-9]+:	0f 79 d8[ 	]*vmwrite %eax,%ebx
[ 	]*[a-f0-9]+:	0f 79 d8[ 	]*vmwrite %eax,%ebx
[ 	]*[a-f0-9]+:	67 0f 79 18[ 	]*vmwrite \(%bx,%si\),%ebx
[ 	]*[a-f0-9]+:	67 0f 79 18[ 	]*vmwrite \(%bx,%si\),%ebx
#pass
