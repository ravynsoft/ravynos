#version 450

layout(location = 0) out vec4 out_color;
layout(push_constant) uniform UBO {
	layout(offset = 80) vec4 color;
} data;

void main() {
	out_color = data.color;
}
