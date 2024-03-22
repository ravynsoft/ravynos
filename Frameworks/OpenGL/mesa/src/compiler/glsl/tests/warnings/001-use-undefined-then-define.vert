#version 130

void main()
{
  float foo;
  float undefinedThenDefined;

  foo = undefinedThenDefined;
  undefinedThenDefined = 2.0;
  foo = undefinedThenDefined;
}

