#version 130

struct s {
  float c;
  float x;
};

void main()
{
  float fooFloat;
  s fooStruct;

  fooFloat = fooStruct.c;
  fooStruct.c = 10.0;
  fooFloat = fooStruct.c;
  fooStruct.c = 20.0;

  /* Technically .x is also uninitialized, but detecting this is beyond
   * scope. FWIW, gcc doesn't detect this neither.
   */
  fooFloat = fooStruct.x;
}

