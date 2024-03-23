%!CairoScript

/pi4 0.785398163 def
/sqrt2 1.414213562 def

/R { % cr size *R* cr
  dup 1 lt { pop } {
      exch /current-point get % size cr x y
      4 2 roll % x y size cr

      % draw a rectangle in relative co-ordinates
      1 index 2 div neg dup exch M
      1 index 0 L
      0 2 index L
      1 index neg 0 L
      h

      save
      1 index dup 2 div neg exch translate
      4 2 roll 3 copy % size cr x y cr x y
      m
      //pi4 rotate
      pop 4 2 roll % x y size cr
      1 index //sqrt2 div
      R
      restore

      save
      1 index dup 2 div exch translate
      4 2 roll 3 copy % size cr x y cr x y
      m
      //pi4 neg rotate
      pop 4 2 roll % x y size cr
      1 index //sqrt2 div
      R
      restore

      4 1 roll pop pop pop
  } ifelse
} bind def

dict
  /width  1024 set
  /height 512 set
  surface context

1 1 1 set-source-rgb paint

0 512 translate 1 -1 scale
512 64 m 128. R 0 0 0 set-source-rgb fill

/target get (out.png) write-to-png pop
pop
