SECTIONS
{
  .text : {
    SORT_BY_ALIGNMENT (*) (.text .text.*)
  }

  .data : {
    SORT_BY_ALIGNMENT (*) (.data .data.*)
  }
}