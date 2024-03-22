#version 130

float __foo(float x)
{
   return 6.0 * x;
}

float __bar(float x)
{
   return 3.0 * x;
}

float __blat(float x)
{
   return 2.0 * x;
}

void main()
{
   gl_Position = vec4(__foo(gl_Vertex.x), __bar(gl_Vertex.y), __blat(gl_Vertex.z), 1.0);
}

