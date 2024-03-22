#version 130

void main()
{
   int i;
   int undefined;
   int undefined2;
   int defined = 2;
   float fooFloat;

   for (i = 0; i < undefined; i++) {
      fooFloat = 10.0;
   }

   for (; undefined < undefined2; i++) {
      fooFloat = 10.0;
   }

   for (i = 0; i < defined; i++) {
      fooFloat = 10.0;
   }
}

