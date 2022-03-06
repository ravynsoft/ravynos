#version 100
precision mediump float;
varying vec4 fragColor;
varying vec2 texCoord;
uniform sampler2D aTexture;
void main()
{
    gl_FragColor = fragColor * texture2D(aTexture, texCoord);
}
