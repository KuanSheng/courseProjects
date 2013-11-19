#version 150

uniform mat4 projection_matrix;
uniform mat4 model_matrix;

in  vec4 vPosition;
in  vec4 Normal;
in  vec4 vColor;
in  vec4 vTexCoord;
out vec2 texCoord;
out vec4 interpolatedColor;
out vec4 Position;
out vec3 R;
out vec3 T;

void main() 
{
  vec4 viewPos = model_matrix*vPosition;
  vec3 eyeDirec = viewPos.xyz;
  vec4 NN = model_matrix*Normal;
  vec3 N = normalize(NN.xyz);
  R = reflect(eyeDirec, N);
  T = refract(eyeDirec, N, 0.3f);

  texCoord = vTexCoord.st;
  
  gl_Position = projection_matrix*model_matrix*vPosition;
  interpolatedColor = vColor;

  Position = gl_Position;
} 
