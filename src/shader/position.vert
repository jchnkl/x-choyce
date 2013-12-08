#version 120

uniform vec2 t1_scale, t2_scale;
uniform vec2 t1_offset, t2_offset;

// uniform vec2 t1_scale = vec2(0.5, 0.5);
// uniform vec2 t2_scale = vec2(0.5, 0.5);

void main()
{
  gl_TexCoord[0].st = gl_MultiTexCoord0.st;

  gl_TexCoord[1].st = gl_MultiTexCoord0.st / t1_scale;
  gl_TexCoord[2].st = gl_MultiTexCoord0.st / t2_scale;

  gl_TexCoord[1].y -= t1_offset.y;
  gl_TexCoord[2].xy -= t2_offset.xy;

  gl_Position = ftransform();
}
