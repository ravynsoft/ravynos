#version 130

void main()
{
  bool defined = false;
  bool undefined;
  int fooInt;
  int definedInt = 2;
  int undefinedInt;

  fooInt = defined ? definedInt : undefinedInt;
  fooInt = defined ? undefinedInt : definedInt;

  fooInt = undefined ? definedInt : undefinedInt;
  fooInt = undefined ? undefinedInt : definedInt;
}

