/SIDE 600 def

<< /width SIDE /height SIDE >> surface context

1 1 1 rgb set-source
paint

SIDE dup scale
0.5 0.5 translate
0.5 -0.5 scale

/theta math.pi 10 div def

10 {
  N
  0 0 1.0 0 //theta arc
  0 0 0.2 //theta 0 arc-
  h
  //theta rotate
  //theta rotate
} repeat

0 0 0.2 0 0 1 radial
  0.0 1 1 0 1 add-color-stop
  1.0 0 0 1 1 add-color-stop
set-source
fill

/target get (out.png) write-to-png pop
pop
