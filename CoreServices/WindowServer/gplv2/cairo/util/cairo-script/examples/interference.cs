/rot { 3 -1 roll } def
/2dup { 2 copy } def

/circle { % context radius -- context
	2dup 0 m pop
	0 0 rot 0 math.2pi arc h
} def

/circles { % context #circles -- context
	1 1 rot { circle } for
} def

/SIDE 600. def
/SIZE 600. def
<< /width SIZE /height SIZE >> surface context
1 1 1 set-source-rgb paint
0 0 0 set-source-rgb
//EVEN_ODD set-fill-rule

/r 4 def
/n SIDE 1.5 mul r div integer def

SIZE SIDE div dup scale

save
  302 200 translate
  r dup scale
  n circles
restore

save
  100 300 translate
  r dup scale
  n circles
restore

save
  500 400 translate
  r dup scale
  n circles
restore

fill

/target get (out.png) write-to-png pop
pop
