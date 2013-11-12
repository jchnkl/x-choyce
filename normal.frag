#version 130

uniform sampler2D texture_0;

void main(void)
{
  vec4 t0 = texture2D(texture_0, gl_TexCoord[0].st);

  gl_FragColor.a = t0.a;
  gl_FragColor.rgb = t0.rgb;
}
