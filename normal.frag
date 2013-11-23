#version 130

uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform sampler2D texture_2;

void main(void)
{
  vec4 t0 = texture2D(texture_0, gl_TexCoord[0].st);
  vec4 t1 = texture2D(texture_1, gl_TexCoord[0].st);
  vec4 t2 = texture2D(texture_2, gl_TexCoord[0].st);

  gl_FragColor.a = t1.a * t2.a;
  gl_FragColor.rgb = t0.rgb + t1.rgb + t2.rgb;
}
