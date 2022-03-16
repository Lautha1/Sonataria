#version 400

// The three separate Yuv color planes
uniform sampler2D yTexture;
uniform sampler2D uTexture;
uniform sampler2D vTexture;

// Chromaticity values for conversion to RGB
const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);
const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);
const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);
const vec3 offset = vec3(-0.0625, -0.5, -0.5);

// Main input and output
in vec2 texcoord;
out vec4 fragcolor;

void main() {
	// Read the individual y, u, and v values
	float y = texture(yTexture, texcoord).r;
	float u = texture(uTexture, texcoord).r;
	float v = texture(vTexture, texcoord).r;

	// Construct the vector and offset the values
	vec3 yuv = vec3(y, u, v);
	yuv += offset;

	// Multiply by chromaticity constants to generate RGB value
	fragcolor = vec4(dot(yuv, R_cf), dot(yuv, G_cf), dot(yuv, B_cf), 1.0);
}
