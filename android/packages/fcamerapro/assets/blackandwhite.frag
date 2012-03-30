// default.frag 

#extension GL_OES_EGL_image_external:enable

precision mediump float;

varying mediump vec2 vTexCoordOut;
uniform samplerExternalOES texture;

void main() {
	vec4 trueColor = texture2D(texture, vTexCoordOut);
	gl_FragColor = vec4(0.299*trueColor.r + 0.587*trueColor.g + 0.144*trueColor.b);
}
