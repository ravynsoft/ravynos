#version 130

void fooFunction(inout float outVar);

out float fooOut;

void main()
{
   float willBeDefined;

   fooFunction(willBeDefined);
   fooOut = willBeDefined;
}

