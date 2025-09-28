#include <iostream>
#include <cstdlib>

void
foo()
{
}

template<typename C>
void
bar(C*)
{
}

template
void
bar<int>(int*);

int
main()
{
  try
    {
      throw(1);
    }
  catch(int)
    {
      std::cout << "caught" << std::endl;
      exit(0);
    }
  std::cout << "failed" << std::endl;
  exit(1);
}
