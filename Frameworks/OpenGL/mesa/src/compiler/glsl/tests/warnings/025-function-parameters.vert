#version 130

void foo(float normalVar, out float outVar, inout float inoutVar)
{
  outVar = 1.0f;
}

void main()
{
   float undefinedFloat;
   float noRaise;
   float undefinedFloat2;

   foo(undefinedFloat, noRaise, undefinedFloat2);
}

