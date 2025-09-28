PHDRS
{
  text    PT_LOAD FLAGS(5);
  data    PT_LOAD FLAGS(6);
  tls     PT_TLS;
}

SECTIONS
{
  .init           :
  {
  } :text
  .text           :
  {
  }
  .data :
  {
  } :data
  .got : { *(.got .toc) }
  .tdata :
  {
    *(.tdata*)
  } :data :tls
  .tbss :
  {
    *(.tbss*)
  } :data :tls
}
