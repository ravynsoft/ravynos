SECTIONS
{
  .empty : {
  here = . == ADDR(.empty);
  ASSERT (. == ADDR(.empty), "dot is not ADDR");
  ASSERT (here, "here is zero");
  }
  ASSERT (!SIZEOF(.empty), "Empty is not empty")
  /DISCARD/ : { *(.reginfo) }
}
