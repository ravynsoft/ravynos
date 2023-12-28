extern int retval;

int
__attribute__ ((visibility ("internal")))
func2 (void)
{
  return retval;
}
