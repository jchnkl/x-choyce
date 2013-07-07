#version 130

uniform sampler2D u_texture;

const float blur = 1.0/3072.0;

void main(void)
{
  vec4 sum = vec4(0.0);

  sum += texture2D(u_texture, vec2(gl_TexCoord[0].x - 4.0*blur, gl_TexCoord[0].y - 4.0*blur)) * 0.05;
  sum += texture2D(u_texture, vec2(gl_TexCoord[0].x - 3.0*blur, gl_TexCoord[0].y - 3.0*blur)) * 0.09;
  sum += texture2D(u_texture, vec2(gl_TexCoord[0].x - 2.0*blur, gl_TexCoord[0].y - 2.0*blur)) * 0.12;
  sum += texture2D(u_texture, vec2(gl_TexCoord[0].x - 1.0*blur, gl_TexCoord[0].y - 1.0*blur)) * 0.15;
  sum += texture2D(u_texture, vec2(gl_TexCoord[0].x           , gl_TexCoord[0].y           )) * 0.16;
  sum += texture2D(u_texture, vec2(gl_TexCoord[0].x + 1.0*blur, gl_TexCoord[0].y + 1.0*blur)) * 0.15;
  sum += texture2D(u_texture, vec2(gl_TexCoord[0].x + 2.0*blur, gl_TexCoord[0].y + 2.0*blur)) * 0.12;
  sum += texture2D(u_texture, vec2(gl_TexCoord[0].x + 3.0*blur, gl_TexCoord[0].y + 3.0*blur)) * 0.09;
  sum += texture2D(u_texture, vec2(gl_TexCoord[0].x + 4.0*blur, gl_TexCoord[0].y + 4.0*blur)) * 0.05;

  float grey = sum.r * 0.299 + sum.g * 0.587 + sum.b * 0.114;
  grey *= 0.85;

  gl_FragColor = vec4(grey, grey, grey, 1.0f);
}
