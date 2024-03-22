#version 130

void foo(float normalVar, out float outVar, inout float inoutVar);

void main()
{
   int undefinedIndex;
   int definedIndex = 2;
   float willBeDefined[4];

   foo(willBeDefined[undefinedIndex], willBeDefined[undefinedIndex], willBeDefined[undefinedIndex]);
   foo(willBeDefined[definedIndex], willBeDefined[definedIndex], willBeDefined[definedIndex]);
   willBeDefined[0] = 10.0;
   foo(willBeDefined[undefinedIndex], willBeDefined[undefinedIndex], willBeDefined[undefinedIndex]);
   foo(willBeDefined[definedIndex], willBeDefined[definedIndex], willBeDefined[definedIndex]);
}

