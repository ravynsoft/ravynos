SECTIONS
{
  .text : {
    SORT_BY_INIT_PRIORITY (*) (.text .text.*)
  }

  .data : {
    SORT_BY_INIT_PRIORITY (*) (.data .data.*)
  }
}