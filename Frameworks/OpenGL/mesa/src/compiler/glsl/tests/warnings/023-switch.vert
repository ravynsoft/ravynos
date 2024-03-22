#version 130

void main()
{
  int defined = 2;
  int undefined;
  float fooFloat;
  int fooInt;

  switch(undefined) {
  case 0:
    fooFloat = 0.0;
  case 1:
    fooFloat = 1.0;
  default:
    fooFloat = undefined;
  }

  switch(defined) {
  case 0:
    fooFloat = 0.0;
  case 1:
    fooFloat = 1.0;
  default:
    fooFloat = undefined;
  }
}

