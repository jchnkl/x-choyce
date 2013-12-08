#version 120

uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform sampler2D texture_2;

void main(void)
{
  vec4 t0 = texture2D(texture_0, gl_TexCoord[0].st);
  vec4 t1 = texture2D(texture_1, gl_TexCoord[1].st);
  vec4 t2 = texture2D(texture_2, gl_TexCoord[2].st);

  gl_FragColor = mix(t0, t1, t1.a);
  gl_FragColor = mix(gl_FragColor, t2, t2.a);
  gl_FragColor.a = 1.0;
}
