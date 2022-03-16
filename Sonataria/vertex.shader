#version 400

// Input variables that can change per-vertex (attribes from openGL)
in vec3 v_pos;	// - position of the vertex
in vec3 v_col;	// - Color of the vertex
in vec2 v_uv;	// - uv coordinates for the vertex

// Uniform variables (can only change per primitive)
uniform mat4 model;			// Model matrix
uniform mat4 projection;	// Projection matrix

uniform bool enableStretch;	    // Turn on stretching
uniform vec2 xStretch;			// Apply base scale and translate to X
uniform vec2 yStretch;			// Apply base scale and translate to Y

// Output to the fragment shader
out vec4 f_col;
out vec4 f_uv;
out vec4 f_pos;

void main() {
	// Copy in the color and texture attributes
	f_uv = vec4(v_uv, 0.0, 1.0);
	f_col = vec4(v_col, 1.0);

	// Apply optional stretching
	vec4 localPos = vec4(v_pos, 1.0);
	if (enableStretch) {
		localPos.x *= xStretch.x; // First param is scale
		localPos.x += xStretch.y; // Second param is translation

		localPos.y *= yStretch.x; // First param is scale
		localPos.y += yStretch.y; // Second param is translation
	}

	// Apply transformation to position and output to fragment
	f_pos = projection * model * localPos;
	gl_Position = f_pos;
}
