#version 150

in  vec4  interpolatedColor;
in  vec4 Position;
in  vec2 texCoord;
in  vec3 R;
in  vec3 T;
out vec4 fColor;

uniform samplerCube texMap;
uniform sampler2D texture;
uniform float fog;
uniform bool reflection;
uniform bool textOn;
uniform bool shadowOn;
uniform bool refraction;

void main(){
	float dist = abs(Position.z);
	float fogFactor;
	float fogMaxDist = 80;
	float fogMinDist = 0.0;
	vec4 fogColor = vec4( 1.0, 1.0, 1.0, 1.0 );

	if(textOn)
		fColor = texture2D( texture, texCoord );
	else if(reflection)
		fColor = textureCube(texMap, R);
	else if(refraction)
		//fColor =  mix(textureCube(texMap, T), vec4(1.0, 1.0,1.0,1.0), 0.3f);
		fColor =  textureCube(texMap, T);
	else if(shadowOn)
		fColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	else
		fColor = interpolatedColor;
	
	//add the effect of fog.
	if(fog>0.5)
		fogFactor = (fogMaxDist - dist)/(fogMaxDist - fogMinDist);
	else
		fogFactor = pow(2.71, -dist*0.08);

	fogFactor = clamp(fogFactor, 0.0, 1.0);

	if(fog>2.0)
		fogFactor = 1.0;

	fColor = mix(fogColor, fColor, fogFactor);
} 

