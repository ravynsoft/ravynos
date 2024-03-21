static char *_edata_p;
static char *_end_p;
static char *__bss_start_p;
extern char *_end;
extern char *_edata;
extern char *__bss_start;

void
bar (void)
{
  _edata_p = (char*) &_edata;
  _end_p = (char*) &_end;
  __bss_start_p = (char*) &__bss_start;
}

void
_start ()
{
  bar ();
}
