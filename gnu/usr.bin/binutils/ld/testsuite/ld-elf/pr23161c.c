extern char *_end __attribute__ ((visibility("hidden")));
extern char *_edata __attribute__ ((visibility("hidden")));
extern char *__bss_start __attribute__ ((visibility("hidden")));

int
foo (void)
{
  return _end[0] + _edata[0] + __bss_start[0];
}
