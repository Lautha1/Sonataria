#version 400

// Input variables that can change per-vertex (attribes from openGL)
in vec3 v_pos;	// - position of the vertex
in vec3 v_col;	// - Color of the vertex
in vec2 v_uv;	// - uv coordinates for the vertex

// Uniform variables (can only change per primitive)
uniform mat4 model;			// Model matrix
uniform mat4 projection;	// Projection matrix

// Output to the fragment shader
out vec4 f_col;
out vec4 f_uv;

void main() {
	// Copy in the color and texture attributes
	f_uv = vec4(v_uv, 0.0, 1.0);
	f_col = vec4(v_col, 1.0);

	// Apply transformation to position and output to fragment
	gl_Position = projection * model * vec4(v_pos, 1.0);
}
