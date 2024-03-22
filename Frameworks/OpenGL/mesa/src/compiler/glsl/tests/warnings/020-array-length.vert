#version 130

void main()
{
  float array[4];
  int fooLength;

  fooLength = array.length();
  array[0] = 2.0;
  fooLength = array.length();
}

