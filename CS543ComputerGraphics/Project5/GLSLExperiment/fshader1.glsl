#version 150

in  vec2 texCoord;
out vec4 fColor;

uniform sampler2D texture;
uniform int systemState;

const float PI = 3.14159265;

void main() 
{ 

    vec3 irgb = texture2D( texture, texCoord ).rgb;
    const vec3 LUMCOEFFS = vec3(0.2125, 0.7154, 0.0721); 
    vec2 ires = textureSize(  texture, 0);
    float ResS = float( ires.s );
    float ResT = float( ires.t );

    vec2 stp0 = vec2(1./ResS, 0. ); 
    vec2 st0p = vec2(0. , 1./ResT);
    vec2 stpp = vec2(1./ResS, 1./ResT);
    vec2 stpm = vec2(1./ResS, -1./ResT);

    float uD = 45.0;
    float uR = 0.5;
    vec2 st = texCoord;
    vec2 xy, dxy, xy1;
    float r, beta, beta1, beta2;
    switch(systemState){
	case 0:
	        fColor = texture2D( texture, texCoord );
		break;
	case 1:
    	        float luminance = dot(irgb, LUMCOEFFS);
    	        fColor = vec4( luminance, luminance,luminance, 1.);
		break;
	case 2:
   	        fColor =vec4( 1.0 - irgb.r, 1.0 - irgb.g, 1.0 - irgb.b, 1.);
	        break;
   	case 3:

    case 5:
		float i00 = dot(texture(texture,texCoord).rgb,       LUMCOEFFS );
        	float im1m1 = dot(texture(texture,texCoord-stpp).rgb,LUMCOEFFS );
        	float ip1p1 = dot(texture(texture,texCoord+stpp).rgb,LUMCOEFFS );
        	float im1p1 = dot(texture(texture,texCoord-stpm).rgb,LUMCOEFFS );
        	float ip1m1 = dot(texture(texture,texCoord+stpm).rgb,LUMCOEFFS );
        	float im10 = dot(texture(texture,texCoord-stp0).rgb,LUMCOEFFS );
        	float ip10 = dot(texture(texture,texCoord+stp0).rgb,LUMCOEFFS );
        	float i0m1 = dot(texture(texture,texCoord-st0p).rgb,LUMCOEFFS );
        	float i0p1 = dot(texture(texture,texCoord+st0p).rgb,LUMCOEFFS );

		float h= -1.*im1p1 - 2.*i0p1 - 1.*ip1p1 + 1.*im1m1 + 2.*i0m1 + 1.*ip1m1;
		float v= -1.*im1m1 - 2.*im10 - 1.*im1p1 + 1.*ip1m1 + 2.*ip10 + 1.*ip1p1;
		
		if(systemState == 3){
			float mag =sqrt(h*h + v*v);
			vec3 target = vec3(mag, mag, mag);

			fColor = vec4( mix(irgb, target, .5), 1. );
		}else{
			float mag = length( vec2(h, v) );
			if( mag > 0.75 ) 
				fColor = vec4( 0., 0., 0., 1.);
			else{ 
				float quantize = 6.0;
				irgb *= quantize; 
				irgb += vec3( .5, .5, .5); 
				ivec3 intrgb = ivec3(irgb) ;
				irgb = vec3( intrgb )/quantize; 
				fColor = vec4( irgb, 1.);
			}
		}
		break;
	case 4:
		vec3 c00 = texture( texture, texCoord ).rgb;
		vec3 cp1p1 = texture( texture, texCoord+stpp ).rgb;

		vec3 diffs = c00-cp1p1;
		float max = diffs.r;
		if(abs(diffs.g) > abs(max))
  			max = diffs.g;
		if(abs(diffs.b) > abs(max))
    			max = diffs.b;

		float gray=clamp(max+0.5, 0., 1.);
                vec4 grayVersion=vec4(gray, gray, gray, 1.);
		vec4 colorVersion=vec4(gray*c00, 1.);

 		fColor = mix(grayVersion,colorVersion,0.1);
		break;
	case 6:
		float Radius = ResS * uR;
		xy = ResS * st;
		dxy = xy - ResS/2.;
		r = length( dxy );
		beta = atan( dxy.y, dxy.x) + radians(uD)*(Radius - r)/Radius;
		xy1 = xy;
		if(r <= Radius)
			xy1 = ResS/2. + r*vec2(cos(beta), sin(beta));
		st = xy1/ResS;

		irgb = texture( texture, st).rgb;
		fColor = vec4(irgb, 1.);
		break;
	case 7:
		xy = ResS * st;
		beta1 = 2*PI*xy.y/120.0;
		beta2 = 2*PI*xy.x/250.0;
		xy1 = xy + vec2(10*sin(beta1), 15*sin(beta2));
		st = xy1/ResS;
		
		irgb = texture( texture, st).rgb;
		fColor = vec4(irgb, 1.);
		break;
	case 8:
       		xy = ResS * st; 
                float p = 1.8;
                float rm = ResS/2.0;
                vec2 xyc = vec2( 0,0 );

                dxy = xy - ResS/2.;
                r = length( dxy );
                float z = sqrt(pow(rm,2)-pow(r,2));
                float betaX = (1-1/p)*(asin(dxy.x/sqrt(dxy.x*dxy.x+z*z)));
                float betaY = (1-1/p)*(asin(dxy.y/sqrt(dxy.y*dxy.y+z*z)));
                xy1 = xy;
		if( r<= rm)
                	xy1 = xy - vec2(z*tan(betaX),z*tan(betaY));

                st = xy1/ResS; 
        irgb = texture( texture, st ).rgb;
        fColor = vec4( irgb, 1. );
		break;
     }
}