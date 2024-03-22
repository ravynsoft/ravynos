#version 130

void main()
{
  int defined = 2;
  int undefined;
  float fooFloat;

  if ((undefined | 2) == 0) {
    fooFloat = 10.0;
  }

  if ((defined | 2) == 0) {
    fooFloat = 10.0;
  }

  if ((undefined | defined) == 0) {
    fooFloat = 10.0;
  }

  if ((defined | undefined) == 0) {
    fooFloat = 10.0;
  }
}
