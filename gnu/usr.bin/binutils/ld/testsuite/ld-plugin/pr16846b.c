extern __thread int foo;

int
bar (void) 
{
  return foo;
}
