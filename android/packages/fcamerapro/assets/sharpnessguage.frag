// default.frag 

#extension GL_OES_EGL_image_external:enable

precision mediump float;

varying mediump vec2 vTexCoordOut;
uniform samplerExternalOES texture;

void main() {
	float deltaX = 1.0/800.0;
	float deltaY = 1.0/600.0;
	
	vec4 trueColor = texture2D(texture, vTexCoordOut);
	
	float diffSum = 0.0;

	int dx = 1;
	int dy = 0;
	
	vec2 coordinates = vec2(vTexCoordOut.x+float(dx)*deltaX, vTexCoordOut.y+float(dy)*deltaY);
	vec4 diffColor = texture2D(texture, vTexCoordOut);	
	diffSum += diffColor.r+diffColor.g+diffColor.b;
	
	gl_FragColor = vec4(diffSum/3.0);
}
