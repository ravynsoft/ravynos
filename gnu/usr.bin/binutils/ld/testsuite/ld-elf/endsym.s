 .ifdef HPUX
end  .comm 4
end2 .comm 2
 .else
  .comm end,4,4
  .comm end2,2,2
 .endif
