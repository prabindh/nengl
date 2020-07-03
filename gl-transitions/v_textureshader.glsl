attribute vec4 aVertexPosition;
attribute vec2 aTextureCoord;
vec3 normal;
uniform mat4 mvMatrix;
uniform vec3 uLightVector;
varying vec2 vTextureCoord;
varying vec3 vColor;
varying float diffuse;
float distance;
vec3 lightPos = vec3(0.0,18.0, 25.0);
vec4 converted;
vec4 converted_lightPos;
void main(void) {
converted = mvMatrix * aVertexPosition;
converted_lightPos = mvMatrix * vec4(lightPos,1.0);
vTextureCoord = aTextureCoord;
normal=normalize(converted.xyz);
distance=length(vec3(converted_lightPos)-vec3(converted));
diffuse = max(dot(normal, normalize(vec3(converted_lightPos-converted))), 0.1);
vColor= diffuse*vec3(0.5,0.5,0.5);
gl_Position = mvMatrix *aVertexPosition;
}