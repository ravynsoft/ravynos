/xdef { exch def } def
/-rot { 3 1 roll } def
/rot { 3 -1 roll } def
/2dup { 2 copy } def

/SIZE 600. def
<< /width SIZE /height SIZE >> surface context
1 1 1 set-source-rgb paint
0 0 0 set-source-rgb
EVEN_ODD set-fill-rule
SIZE SIZE scale

0.5 0 translate

save
1 1 scale
/n 90 def
n neg 1 n {
	/x xdef
	0 0 m
	x 1 l
	x 0.5 add 1 l
	0 0 l
} for
restore
fill

/target get (out.png) write-to-png pop
pop
