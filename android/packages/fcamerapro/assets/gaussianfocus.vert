// default.vert

attribute vec4 vPosition;
attribute vec2 vTexCoordIn;

varying vec2 vTexCoordOut;

void main() {
	gl_Position = vPosition;
	vTexCoordOut = vTexCoordIn;
}
