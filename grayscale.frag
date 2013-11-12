#version 130

uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform sampler2D texture_2;

const float alpha = 0.875;

const float blur = 1.0/3072.0;

void main(void)
{
  vec4 sum = vec4(0.0);

  sum += texture2D(texture_0, vec2(gl_TexCoord[0].x - 4.0*blur, gl_TexCoord[0].y - 4.0*blur)) * 0.05;
  sum += texture2D(texture_0, vec2(gl_TexCoord[0].x - 3.0*blur, gl_TexCoord[0].y - 3.0*blur)) * 0.09;
  sum += texture2D(texture_0, vec2(gl_TexCoord[0].x - 2.0*blur, gl_TexCoord[0].y - 2.0*blur)) * 0.12;
  sum += texture2D(texture_0, vec2(gl_TexCoord[0].x - 1.0*blur, gl_TexCoord[0].y - 1.0*blur)) * 0.15;
  sum += texture2D(texture_0, vec2(gl_TexCoord[0].x           , gl_TexCoord[0].y           )) * 0.16;
  sum += texture2D(texture_0, vec2(gl_TexCoord[0].x + 1.0*blur, gl_TexCoord[0].y + 1.0*blur)) * 0.15;
  sum += texture2D(texture_0, vec2(gl_TexCoord[0].x + 2.0*blur, gl_TexCoord[0].y + 2.0*blur)) * 0.12;
  sum += texture2D(texture_0, vec2(gl_TexCoord[0].x + 3.0*blur, gl_TexCoord[0].y + 3.0*blur)) * 0.09;
  sum += texture2D(texture_0, vec2(gl_TexCoord[0].x + 4.0*blur, gl_TexCoord[0].y + 4.0*blur)) * 0.05;

  float red   = sum.r * 0.299;
  float green = sum.g * 0.587;
  float blue  = sum.b * 0.114;
  float grey  = red * alpha + green * alpha + blue * alpha;

  vec4 t1 = texture2D(texture_1, gl_TexCoord[0].st);
  vec4 t2 = texture2D(texture_2, gl_TexCoord[0].st);

  gl_FragColor = vec4(grey);
  gl_FragColor.a = sum.a * t1.a * t2.a;
  gl_FragColor.rgb += t1.rgb + t2.rgb;
}
