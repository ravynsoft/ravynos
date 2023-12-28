SECTIONS
{
  .text 0x4000 : {
    * (.text .text.*);
  }

  .data 0x8000 : {
    * (.data .data.*);
  }
}
