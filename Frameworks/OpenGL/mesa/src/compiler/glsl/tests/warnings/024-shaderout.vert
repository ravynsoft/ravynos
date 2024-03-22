#version 130

out int fooOut;

void main()
{
  int defined = 2;
  int undefined;
  int fooInt;

  defined = fooOut;
  fooOut = undefined;
  /* Technically at this point fooOut is still undefined. But it was
   * initialized that is what the unitialized warning detects in any
   * case. "Real undefined" is beyond the scope of what mesa is/should
   * detect*/
  defined = fooOut;
}

