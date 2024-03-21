 .weak xweakobj
 .weak xweakhidobj
 .hidden xweakhidobj

 .data
 .global x
 .type	x,@object
x:
 .dword xweakhidobj
 .dword xweakobj
 .dword xregobj
.Lfe1:
 .size	x,.Lfe1-x
