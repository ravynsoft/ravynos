%!CairoScript

/1sqrt3 0.577359269 def

/T { % cr size -- cr
  exch % size cr
  0 0 m 1 index 0 l 1 index dup 2 div exch //1sqrt3 mul l h

  exch 2 div
  dup 4 ge {
    exch % size/2 cr

    1 index T
    save 1 index 0 translate 1 index T restore
    save 1 index dup 2 div exch //1sqrt3 mul translate 1 index T restore

    exch
  } if
  pop
} bind def

dict
  /width  512 set
  /height 300 set
  surface context

1 1 1 set-source-rgb paint

.5 set-line-width

0 300 translate
1 -1 scale

512 T 0 0 0 set-source-rgb stroke

/target get (out.png) write-to-png pop
pop
