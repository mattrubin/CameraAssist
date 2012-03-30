// default.frag 

#extension GL_OES_EGL_image_external:enable

precision mediump float;

varying mediump vec2 vTexCoordOut;
uniform samplerExternalOES texture;

void main() {
	float deltaX = 1.0/800.0;
	float deltaY = 1.0/600.0;
	
	
	float sum = 0.0;
	float squareSum = 0.0;


	int radius = 2;

	int dy = 0;
	int dx = -radius;
	
	// get the color at (x-radius, y)
	vec2 coordinates = vec2(vTexCoordOut.x+float(dx)*deltaX, vTexCoordOut.y+float(dy)*deltaY);
	vec4 firstColor = texture2D(texture, vTexCoordOut);
	vec4 prevColor = firstColor;
	
	// iterate over the pixels from (x-radius+1, y) to (x+radius, y)
	for(dx = -(radius-1); dx <= radius; dx++){
		
		// get the color for this pixel
		coordinates = vec2(vTexCoordOut.x+float(dx)*deltaX, vTexCoordOut.y+float(dy)*deltaY);
		vec4 nextColor = texture2D(texture, coordinates);
		
		// Compare to the previous pixel
		vec4 colorDiff = nextColor-prevColor;
		float luminanceDiff = 0.299*colorDiff.r + 0.587*colorDiff.g + 0.144*colorDiff.b;
		
		// Add to the sums
		sum += luminanceDiff;
		squareSum += pow(luminanceDiff, 2.0);
	}
	
	float sumSquared = pow(sum, 2.0);
	
	gl_FragColor = vec4(squareSum/(0.5));
}
