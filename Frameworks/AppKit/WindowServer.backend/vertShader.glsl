#version 100
attribute vec3 vposition;
attribute vec4 color;
attribute vec2 texture;
varying vec4 fragColor;
varying vec2 texCoord;
void main()
{
    gl_Position = vec4(vposition, 1.0);
    fragColor = color;
    texCoord = texture;
}


