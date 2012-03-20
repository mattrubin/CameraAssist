// default.frag 

#extension GL_OES_EGL_image_external:enable

precision mediump float;

varying mediump vec2 vTexCoordOut;
uniform samplerExternalOES texture;

void main() {
	vec4 color = texture2D(texture, vTexCoordOut);
	if(color.x > 1.0 && color.x > 1.0 && color.x > 1.0)
		color = (sin((vTexCoordOut.x+vTexCoordOut.y)/0.02)>0.0)?vec4(0.0, 1.0, 0.5, color.a):vec4(0.0, 0.5, 1.0, color.a);
	gl_FragColor = color;
}
