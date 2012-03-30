// default.frag 

#extension GL_OES_EGL_image_external:enable

precision mediump float;

varying mediump vec2 vTexCoordOut;
uniform samplerExternalOES texture;

void main() {
	float deltaX = 1.0/800.0;
	float deltaY = 1.0/600.0;
	
	vec4 composite = vec4(0.0);
	int radius = 3;
	for(int dx=-radius; dx<radius; dx++){
		for(int dy=-radius; dy<radius; dy++){
			vec2 coordinates = vec2(vTexCoordOut.x+float(dx)*deltaX, vTexCoordOut.y+float(dy)*deltaY);
			vec4 color = texture2D(texture, coordinates);
			float weight = float(2*radius*2*radius);
			composite += color/weight;
		}
	}
	
	gl_FragColor = composite;
}
