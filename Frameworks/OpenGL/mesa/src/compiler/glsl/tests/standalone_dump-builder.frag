varying vec2 Var_18;
uniform sampler2D Var_19;
uniform sampler2D Var_1A;
uniform float Var_1B;
void main()
{
   vec3 Var_21 = texture2D(Var_19, Var_18).xyz;
   vec3 Var_22 = texture2D(Var_1A, Var_18).xyz;
   vec3 Var_23 = ((Var_22 - Var_21) / Var_1B);
   (Var_22 += (Var_23 * Var_1B));
   (gl_FragColor = vec4(Var_22, 1.0));
}
