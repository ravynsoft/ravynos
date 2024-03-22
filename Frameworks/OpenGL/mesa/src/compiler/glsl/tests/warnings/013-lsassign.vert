#version 130

void main()
{
  int defined = 2;
  int undefined;
  int fooInt;

  fooInt >>= undefined;
  fooInt >>= defined;
}

