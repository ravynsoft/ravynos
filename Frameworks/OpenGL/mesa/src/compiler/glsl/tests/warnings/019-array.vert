#version 130

void main()
{
  int undefinedIndex;
  int undefinedIndex2;
  int definedIndex = 2;
  int definedIndex2 = 2;
  float array[4];
  float fooPos;
  int fooLength;

  fooPos = array[undefinedIndex];
  fooPos = array[definedIndex];

  fooPos = array[definedIndex+definedIndex2];
  fooPos = array[undefinedIndex+undefinedIndex2];
  array[0] = 10.0;
  fooPos = array[definedIndex];

  array[undefinedIndex2] = array[undefinedIndex];
}

