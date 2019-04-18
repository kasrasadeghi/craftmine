#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

const char* vertex_shader =
R"zzz(#version 410 core

// input from render
layout (location = 0) in vec4 vertex_position;

// input from instances
layout (location = 1) in vec3 instance_offset;
layout (location = 2) in uint direction;
layout (location = 3) in uint texture_index;

out uint vs_direction;
out uint vs_texture_index;

void main()
{
  vec3 pos = vertex_position.xyz;
  switch(direction) {
    case 0: // +X
      pos.z *= -1; break;
    case 1: // +Y
      pos = pos.yxz; break;
    case 2: // +Z
      pos = pos.zyx; break;
    case 3: // -X
      pos = -pos; 
      break;
    case 4: // -Y
      pos = -pos.yxz; 
      pos.x *= -1;
      break;
    case 5: // -Z
      pos = -pos.zyx; 
      pos.x *= -1;
      break;
  }
  vs_direction = direction;
  vs_texture_index = texture_index;

	gl_Position = vec4(instance_offset, 0) + vec4(pos, 1);
}
)zzz";

const char* geometry_shader =
R"zzz(#version 410 core

// layout description between vertex and fragment shader
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

// input uniform
uniform mat4 projection;
uniform mat4 view;

// input from vertex shader
in uint vs_direction[];
in uint vs_texture_index[];

// pass along from vertex shader
flat out uint sq_direction;
flat out uint sq_texture_index;

// output to fragment shader
flat out vec4 normal;
out vec4 world_position;
out vec4 bary_coord;
flat out float perimeter;
out vec2 tex_coord;

void main()
{
  sq_direction = vs_direction[0];
  sq_texture_index = vs_texture_index[0];
  
	vec3 AB = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 AC = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	normal = normalize(vec4(cross(AB, AC), 0));

	vec3 A = gl_in[0].gl_Position.xyz;
	vec3 B = gl_in[1].gl_Position.xyz;
	vec3 C = gl_in[2].gl_Position.xyz;
  vec3 D = A + (C - B);

	perimeter = length(A - B) + length(B - C) + length(C - A);
	
  world_position = vec4(A, 1);
  gl_Position = projection * view * world_position;
  bary_coord = vec4(1, 0, 0, 0);
  tex_coord = vec2(0, 0);
  EmitVertex();

  world_position = vec4(D, 1);
  gl_Position = projection * view * world_position;
  bary_coord = vec4(0, 0, 0, 1);
  tex_coord = vec2(0, 1);
  EmitVertex();

  world_position = vec4(B, 1);
  gl_Position = projection * view * world_position;
  bary_coord = vec4(0, 1, 0, 0);
  tex_coord = vec2(1, 0);
  EmitVertex();

  world_position = vec4(C, 1);
  gl_Position = projection * view * world_position;
  bary_coord = vec4(0, 0, 1, 0);
  tex_coord = vec2(1, 1);
  EmitVertex();
  
	EndPrimitive();
}
)zzz";

// must use every input from geometry shader
const char* fragment_shader =
R"zzz(#version 410 core

uniform bool wireframe; 
uniform vec4 light_position;

uniform vec4 base_colors[10];
uniform vec4 off_colors[10];

// input from vertex shader passed through geometry shader
flat in uint sq_direction;
flat in uint sq_texture_index;

flat in vec4 normal;
in vec4 world_position;
in vec4 bary_coord;
flat in float perimeter;
in vec2 tex_coord;
flat in vec3 flag_color;

out vec4 fragment_color;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float blerp (float i00, float i10, float i01, float i11, float u, float v) {
    float lerp_x0 = mix(i00,i10, u);
    float lerp_x1 = mix(i01,i11, u);

    return mix(lerp_x0, lerp_x1, v);
}

float get_noise_level(float u, float v, float grit_level) {
  // 3 grits 8x8, 16x16, 32x32
  float grit00 = rand(floor(vec2(u,v) * grit_level) + vec2(0,0));
  float grit10 = rand(floor(vec2(u,v) * grit_level) + vec2(1,0));
  float grit01 = rand(floor(vec2(u,v) * grit_level) + vec2(0,1));
  float grit11 = rand(floor(vec2(u,v) * grit_level) + vec2(1,1));

  float new_u = (u - (floor(u * grit_level) / grit_level)) * grit_level;
  float new_v = (v - (floor(v * grit_level) / grit_level)) * grit_level;

  float grit = blerp(grit00, grit10, grit01, grit11, new_u, new_v);
  return grit;
}

float cross_lines(float u, float v, float rand) {
  return sin(u - v + rand);
}

float ProceduralNoise(float u, float v) {
  // 3 grits 8x8, 16x16, 32x32
  float high_grit = get_noise_level(u, v, 8);
  float mid_grit = get_noise_level(u, v, 16);
  float low_grit = get_noise_level(u, v, 32);

  
  return (high_grit + 2/3.f * mid_grit + 1/3.f * low_grit) / 2;
}

float TWO_PI = 6.28318530717958647692528676655900576;
#define WATER 3

vec2 get_gradient(int i, int j) {
  float theta = rand(vec2(i, j));
  return vec2(cos(theta), sin(theta));
}

float perlin(float x, float y) {

  vec2 pos = vec2(x, y) + vec2(5);

  int x0 = int(pos.x);
  int x1 = x0 + 1;
  int y0 = int(pos.y);
  int y1 = y0 + 1;

  vec2 g00 = get_gradient(x0, y0);
  vec2 g01 = get_gradient(x0, y1);
  vec2 g10 = get_gradient(x1, y0);
  vec2 g11 = get_gradient(x1, y1);

  // corners
  vec2 c00 = vec2(x0, y0);
  vec2 c10 = vec2(x1, y0);
  vec2 c01 = vec2(x0, y1);
  vec2 c11 = vec2(x1, y1);

  // distances
  vec2 d00 = pos - c00;
  vec2 d01 = pos - c01;
  vec2 d10 = pos - c10;
  vec2 d11 = pos - c11;

  // products
  float p00 = dot(g00, d00);
  float p01 = dot(g01, d01);
  float p10 = dot(g10, d10);
  float p11 = dot(g11, d11);

  // heights and interpolation
  float h0_ = mix(p00, p10, d00.x);
  float h1_ = mix(p01, p11, d00.x);
  float h = mix(h0_, h1_, d00.y);

  return h;
}

vec4 axis_color() {
  uint dir = sq_direction;
  if (dir >= 3) {
    dir -= 3;
  } 

  vec4 color = vec4(0);
  color[dir] = 1;

  return color;
}

vec4 height_atten(vec4 color) {
  float w = (70 - world_position.y)/35;
  return mix(vec4(0, 0, 0, 0), color, 1 - w);
}

vec4 menger_color(vec4 color) {
  // vec4 color = abs(normal);

	vec4 light_direction = normalize(world_position - light_position);
	float dot_nl = dot(normalize(light_direction), normalize(normal));
	dot_nl = clamp(dot_nl, 0.0, 1.0);
	return mix(clamp(dot_nl * color, 0.0, 1.0), color, .7);
}

void main()
{
  // fragment_color = vec4(tex_coord, 0, 1);

  // float alpha = 10;
  // float beta = 1;

  // float vert_line_weight = 17;
  // float hori_line_weight = 17;

  // float noise = cross_lines(hori_line_weight * tex_coord.x, vert_line_weight * tex_coord.y , alpha * ProceduralNoise(beta * tex_coord.x, beta * tex_coord.y) );
  // fragment_color = mix(fragment_color, vec4(noise, noise, noise, 1), 0.5);

  // fragment_color = mix(vec4(0, 0, 0, 1), vec4(0, 1, 0, 1), perlin(world_position.x, world_position.y));

  vec2 i = world_position.xz;

  float k = 8;
  i = vec2(floor(i * 8))/8;

  float p = perlin(2 * i.x, 2 * i.y);
  fragment_color = mix(base_colors[sq_texture_index], off_colors[sq_texture_index], p);
  fragment_color = height_atten(fragment_color);
	
  fragment_color = menger_color(fragment_color);
  // if (sq_texture_index == WATER) fragment_color.w = 0.5; //FIXME: water should be see-through
  if (sq_texture_index == WATER) fragment_color.w = 1;
  else fragment_color.w = 1;

  if (wireframe) {
    bool is_frame = min(bary_coord.x, min(bary_coord.y, bary_coord.z)) * perimeter < 0.05;
    if (is_frame) {
      fragment_color = vec4(0, float(is_frame), 0, 1);
    }
  }
}
)zzz";


void
CreateProgram(GLuint& program_id) {

	auto createShader = [](const char* source, GLenum shaderType, std::string name = "") -> GLuint {
		GLuint shader_id = 0;
		const char* source_ptr = source;
		shader_id = glCreateShader(shaderType);
		glShaderSource(shader_id, 1, &source_ptr, nullptr);
		glCompileShader(shader_id);
		int status = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
		if (not status) {
			std::cout << "problem compiling shader: " << name << std::endl;
      GLint success;
      GLchar infoLog[1024];
      glGetShaderInfoLog(shader_id, 1024, NULL, infoLog);
      std::cout << infoLog << std::endl;
		}

		return shader_id;
	};

	GLuint vertex_shader_id         = createShader(vertex_shader, GL_VERTEX_SHADER, "vertex");
	GLuint geometry_shader_id       = createShader(geometry_shader, GL_GEOMETRY_SHADER, "geometry");
	GLuint fragment_shader_id       = createShader(fragment_shader, GL_FRAGMENT_SHADER, "fragment");

	// @output program_id
	program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glAttachShader(program_id, geometry_shader_id);

	// Bind attributes.
	glBindAttribLocation(program_id, 0, "vertex_position");
  glBindAttribLocation(program_id, 1, "instance_offset");
  glBindAttribLocation(program_id, 2, "direction");
  glBindAttribLocation(program_id, 3, "texture_index");
	glBindFragDataLocation(program_id, 0, "fragment_color");
	glLinkProgram(program_id);

  GLint success;
  glGetProgramiv(program_id, GL_LINK_STATUS, &success);
  if (not success) {
    GLchar infoLog[1024];
    glGetShaderInfoLog(program_id, 1024, NULL, infoLog);
    std::cout << infoLog << std::endl;
  }
}