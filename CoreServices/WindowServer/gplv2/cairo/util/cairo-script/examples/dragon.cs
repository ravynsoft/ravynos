%!CairoScript
/pot { % n -- n
  1 sub
  dup  -1 bitshift or
  dup  -2 bitshift or
  dup  -4 bitshift or
  dup  -8 bitshift or
  dup -16 bitshift or
  1 add
} bind def

/direction { % i -- bool
  dup 2 lt { pop true } {
      dup 1 add dup pot dup 3 -1 roll eq { pop pop true } {
	  2 div 1 sub 2 mul exch sub direction not
      } ifelse
  } ifelse
} bind def

/path { % cr dx dy -- cr
  0 1 2048 {
    4 1 roll 3 copy L pop 4 -1 roll direction {
      exch neg
    } {
      neg exch
    } ifelse
  } for
  pop pop
} bind def

dict
  /width  512 set
  /height 512 set
  surface context

1 g set-source paint

//LINE_CAP_ROUND set-line-cap
//LINE_JOIN_ROUND set-line-join
4 set-line-width

256 256 m  12 0 path 0 0 0 rgb set-source stroke
256 256 m -12 0 path 1 0 0 rgb set-source stroke
256 256 m 0  12 path 0 1 0 rgb set-source stroke
256 256 m 0 -12 path 0 0 1 rgb set-source stroke

/target get (out.png) write-to-png pop
pop
