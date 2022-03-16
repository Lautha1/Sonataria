#version 400

// Texturing variables
in vec4 f_uv;	// Input from vertex shader (linearly interpolated)
in vec4 f_col;	// More input from vertex shader

uniform sampler2D greyTex;	// Input from main OpenGL program
uniform vec3 colorTint;		// Color to tint the text

// Output to framebuffer
out vec4 frag_color;

void main() {
	// Convert GL_RED to greyscale
	vec4 texColor = texture2D(greyTex, f_uv.st).rrrr;

	// Discard invisible fragments
	if (texColor.a < 0.1) {
		discard;
	}

	// Tint and return color
	frag_color = texColor * vec4(colorTint, 1.0);
}
