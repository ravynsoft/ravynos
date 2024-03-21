SECTIONS
{
  .text 0x0 : {
    * (.text .text.*);
  }

  .data 0x3000 : {
    * (.data .data.*);
  }
}
