extern char *_end;
extern char *_edata;
extern char *__bss_start;

int
foo (void)
{
  return _end[0] + _edata[0] + __bss_start[0];
}
