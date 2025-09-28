#include <stdio.h>
#include <unistd.h>

ssize_t
__wrap_read (int fd, void *buffer, size_t count)
{
  puts ("PASS");
  return fd + count + sizeof (buffer);
}


int
main ()
{
  int i = read (1, "abc", 4);
  return i == 0;
}
