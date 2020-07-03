precision mediump float;
varying vec2 vTextureCoord;
uniform sampler2D uSampler0;
varying vec3 vColor;
varying float diffuse;
vec4 tempc;

uniform float progress;

vec4 getFromColor(vec2 p)
{
    vec4 tempc;
    tempc=texture2D(uSampler0, vec2(p.x, 1.0-p.y));
    return tempc;
}
vec4 getToColor(vec2 p)
{
    vec4 tempc;
    tempc=vec4(0.0, 0.0, 0.0, 1.0);
    return tempc;
}
vec4 transition(vec2 uv) {
  
  vec2 p = uv.xy / vec2(1.0).xy;
  
  float circPos = atan(p.y - 0.5, p.x - 0.5) + progress * 2.0;
  float modPos = mod(circPos, 3.1415 / 4.);
  float signed = sign(progress - modPos);
  
  return mix(getToColor(p), getFromColor(p), step(signed, 0.5));
  
}
void main(void) {
gl_FragColor=transition(vTextureCoord);
}