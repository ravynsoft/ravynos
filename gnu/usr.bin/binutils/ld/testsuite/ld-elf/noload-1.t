SECTIONS
{
  . = SIZEOF_HEADERS;
  TEST (NOLOAD) :
  {
    *(TEST)
  }
  /DISCARD/ : { *(.*) }
}
