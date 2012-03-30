// default.frag 

#extension GL_OES_EGL_image_external:enable

precision mediump float;

varying mediump vec2 vTexCoordOut;
uniform samplerExternalOES texture;

void main() {
	float deltaX = 1.0/800.0;
	float deltaY = 1.0/600.0;
	
	vec4 trueColor = texture2D(texture, vTexCoordOut);
	
	vec4 composite = vec4(0.0);
	float diffSum = 0.0;
	int radius = 2;
	
	for(int dx=-radius; dx<radius; dx++){
		for(int dy=-radius; dy<radius; dy++){
			vec2 coordinates = vec2(vTexCoordOut.x+float(dx)*deltaX, vTexCoordOut.y+float(dy)*deltaY);
			vec4 color = texture2D(texture, coordinates);
			float weight = float(2*radius*2*radius);
			composite += color/weight;
			
			vec4 diffColor = abs(trueColor-color);
			diffSum += diffColor.r+diffColor.g+diffColor.b;
		}
	}
	
	composite = vec4(1.0, 0.0, 0.0, 0.0);
	vec4 diffColor = trueColor-composite;
	if(diffSum<1.0)
		gl_FragColor = trueColor-2.0*(1.0-diffSum)*diffColor;
	else
		gl_FragColor = trueColor+2.0*(diffSum-1.0)*diffColor;
		
	////if(diffSum<1.0) gl_FragColor=vec4(0.5);
//	gl_FragColor = vec4(diffSum/2.0);
}
