#version 400

// Texturing variables
in vec4 f_uv;	// Input from vertex shader (linearly interpolated)
in vec4 f_col;	// More input from vertex shader
in vec4 f_pos;  // Global coordinate position

uniform sampler2D colorTex;	// Input from main OpenGL program

uniform float opacity;
uniform float zClip;

// Output to framebuffer
out vec4 frag_color;

void main() {
	vec4 texColor = texture2D(colorTex, f_uv.st);
	texColor.a *= opacity;

	if (texColor.a < 0.1 || f_pos.z > zClip) {
		discard;
	}

	frag_color = texColor;
}
