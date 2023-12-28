extern int retval;

int
__attribute__ ((visibility ("hidden")))
func3 (void)
{
  return retval;
}
