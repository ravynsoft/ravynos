extern int retval;

int
__attribute__ ((visibility ("protected")))
func1 (void)
{
  return retval;
}
