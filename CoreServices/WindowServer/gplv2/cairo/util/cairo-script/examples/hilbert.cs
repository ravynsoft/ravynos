%!CairoScript

/hA { % cr dist lvl *hA* cr dist
  dup not { pop } {
      1 sub
      3 copy hB 0 exch L pop
      3 copy hA 0 L pop
      3 copy hA neg 0 exch L pop
      hC
  } ifelse
} bind def

/hB { % cr dist lvl *hB* cr dist
  dup not { pop } {
      1 sub
      3 copy hA 0 L pop
      3 copy hB 0 exch L pop
      3 copy hB neg 0 L pop
      hD
  } ifelse
} bind def

/hC { % cr dist lvl *hC* cr dist
  dup not { pop } {
      1 sub
      3 copy hD neg 0 L pop
      3 copy hC neg 0 exch L pop
      3 copy hC 0 L pop
      hA
  } ifelse
} bind def

/hD { % cr dist lvl *hD* cr dist
  dup not { pop } {
      1 sub
      3 copy hC neg 0 exch L pop
      3 copy hD neg 0 L pop
      3 copy hD 0 exch L pop
      hB
  } ifelse
} bind def

dict
  /width  1024 set
  /height 1024 set
  surface context

2 2 m 4 10 hA pop 1 g set-source stroke

/target get (out.png) write-to-png pop
pop
