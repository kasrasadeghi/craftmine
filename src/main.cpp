#include "RenderWindow.h"
#include "Camera.h"
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>

const char* vertex_shader =
R"zzz(#version 410 core

// input from render
layout (location = 0) in vec4 vertex_position;

// input from instances
layout (location = 1) in vec3 instance_offset;
layout (location = 2) in uint direction;

void main()
{
  vec3 pos = vertex_position.xyz;
  switch(direction) {
    // case 0: // +X
    case 1: // +Y
      pos = pos.yxz; break;
    case 2: // +Z
      pos = pos.zyx; break;
    case 3: // -X
      pos = -pos; break;
    case 4: // -Y
      pos = -pos.yxz; break;
    case 5: // -Z
      pos = -pos.zyx; break;
  }

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

// output to fragment shader
flat out vec4 normal;
out vec4 world_position;
out vec4 bary_coord;
flat out float perimeter;
out vec2 tex_coord;

void main()
{
	int n = 0;
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

flat in vec4 normal;
in vec4 world_position;
in vec4 bary_coord;
flat in float perimeter;
in vec2 tex_coord;
flat in vec3 flag_color;

out vec4 fragment_color;

void main()
{
	// vec4 color = abs(normal);

	// vec4 light_direction = normalize(world_position - light_position);
	// float dot_nl = dot(normalize(light_direction), normalize(normal));
	// dot_nl = clamp(dot_nl, 0.0, 1.0);
	// fragment_color = clamp(dot_nl * color, 0.0, 1.0);
  fragment_color = vec4(tex_coord, 0, 1);

	bool is_frame = min(bary_coord.x, min(bary_coord.y, bary_coord.z)) * perimeter < 0.05;
	if (wireframe && is_frame) {
		fragment_color = vec4(0, float(is_frame), 0, 1);
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

int main() {
  RenderWindow window {"Hello World", 800, 600};

  Camera camera;

  bool wireframe_mode = false;
  window.setKeyCallback([&](int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
      window.close();
    }

    if (key == GLFW_KEY_F && mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) {
      wireframe_mode = !wireframe_mode;
      glPolygonMode(GL_FRONT_AND_BACK, (wireframe_mode ? GL_LINE : GL_FILL));
    }

    camera.handleKey(key, scancode, action, mods);
  });

  struct MouseState_ {
    bool pressed = false;
    int current_button = 0;
    glm::ivec2 prev_pos = {-1, -1};
  } mouse;

  window.setMouseCallback([&](int button, int action, int mods) {
    mouse.pressed = (action == GLFW_PRESS);
    mouse.current_button = button;
    if (action == GLFW_RELEASE) {
      mouse.prev_pos = {-1, -1};
    }
  });

  window.setCursorCallback([&](double mouse_x, double mouse_y) {
    if (not mouse.pressed) {
      return;
    }

    if (mouse.current_button == GLFW_MOUSE_BUTTON_LEFT) {
      glm::ivec2& prev = mouse.prev_pos;
      if (prev == glm::ivec2{-1, -1}) {
        prev = {mouse_x, mouse_y};
      } else {
        int dx = mouse_x - prev.x;
        int dy = mouse_y - prev.y;
        camera.yaw(dx);
        camera.pitch(dy);

        prev = {mouse_x, mouse_y};
      }
    }
  });

  std::vector<glm::vec4> vertices;
  vertices.emplace_back(0.5, -0.5,  0.5, 1);
  vertices.emplace_back(0.5, -0.5, -0.5, 1);
  vertices.emplace_back(0.5,  0.5, -0.5, 1);

  std::vector<glm::uvec3> faces;
  faces.emplace_back(0, 1, 2);

  // // naive triangle add
  // auto addCube = [&](float size, glm::vec3 pos) {
  //   auto s = size;
  //   auto x = pos.x;
  //   auto y = pos.y;
  //   auto z = pos.z;
  //   auto face_base = vertices.size();
  //   vertices.emplace_back( s + x,  s + y,  s + z, 1.0f);
  //   vertices.emplace_back(-s + x,  s + y,  s + z, 1.0f);
  //   vertices.emplace_back(-s + x, -s + y,  s + z, 1.0f);
  //   vertices.emplace_back( s + x, -s + y,  s + z, 1.0f);
  //   vertices.emplace_back( s + x, -s + y, -s + z, 1.0f);
  //   vertices.emplace_back( s + x,  s + y, -s + z, 1.0f);
  //   vertices.emplace_back(-s + x,  s + y, -s + z, 1.0f);
  //   vertices.emplace_back(-s + x, -s + y, -s + z, 1.0f);

  //   auto make_face = [&faces, face_base](uint a, uint b, uint c, uint d) {
  //     a += face_base; b += face_base; c += face_base; d += face_base;
  //   	faces.emplace_back(b, a, d);
  //   	faces.emplace_back(c, b, d);
  //   };
  //   make_face(2, 1, 6, 7); // F'
  //   make_face(0, 3, 4, 5); // F   +x
  //   make_face(4, 3, 2, 7); // U'
  //   make_face(6, 1, 0, 5); // U   +y
  //   make_face(4, 5, 6, 7); // R'
  //   make_face(0, 1, 2, 3); // R   +z
  // };

  // addCube(1, {0, 0, 0});

  struct Instance_ {
    Instance_(glm::vec3 p, GLuint d): x(p.x), y(p.y), z(p.z), direction(d) {}
    float x;
    float y;
    float z;
    GLuint direction; // 0 .. 5 = x, y, z, -x, -y, -z
  } __attribute__((packed));

  std::vector<Instance_> instances;
  instances.emplace_back(glm::vec3{0, 0, 0}, 0);
  instances.emplace_back(glm::vec3{0, 0, 0}, 1);
  instances.emplace_back(glm::vec3{0, 0, 0}, 2);
  instances.emplace_back(glm::vec3{0, 0, 0}, 3);
  instances.emplace_back(glm::vec3{0, 0, 0}, 4);
  instances.emplace_back(glm::vec3{0, 0, 0}, 5);

  GLuint worldVAO;
  struct VBO_ {
    GLuint vertex_buffer;
    GLuint instances_buffer;
    GLuint index_buffer;
  } VBO;

	glGenVertexArrays(1, &worldVAO);
	glBindVertexArray(worldVAO);
	glGenBuffers(3, (GLuint*)(&VBO));

  // Setup element array buffer.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO.index_buffer);
  	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::uvec3) * faces.size(), faces.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(    0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO.instances_buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Instance_) * instances.size(), instances.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(    1, 3, GL_FLOAT, GL_FALSE, sizeof(Instance_), (void*)0);
    glVertexAttribDivisor(    1, 1);

    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(   2, 1, GL_UNSIGNED_INT, sizeof(Instance_), (void*)(sizeof(glm::vec3)));
    glVertexAttribDivisor(    2, 1);

  GLuint program_id = 0;
	CreateProgram(program_id);
  glUseProgram(program_id);
  

  struct {
		GLint projection = 0;
		GLint view = 0;
		GLint light_pos = 0;
    GLint wireframe = 0;
	} uniform;

  uniform.projection = glGetUniformLocation(program_id, "projection");
	uniform.view       = glGetUniformLocation(program_id, "view");
	uniform.light_pos  = glGetUniformLocation(program_id, "light_position");
  uniform.wireframe  = glGetUniformLocation(program_id, "wireframe");

  glm::vec4 light_position {0, 10, 0, 1};

  while (window.isOpen()) {
    // glfwGetFramebufferSize(window, &window_width, &window_height);
		glViewport(0, 0, window.width(), window.height());
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);

    glBindVertexArray(worldVAO);

    // if world is dirty
    // // Setup vertex data in a VBO.
    // CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, g_buffer_objects[kGeometryVao][kVertexBuffer]));
    // // NOTE: We do not send anything right now, we just describe it to OpenGL.
    // CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj_vertices.size() * 4, obj_vertices.data(), GL_STATIC_DRAW));
    // CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
    // CHECK_GL_ERROR(glEnableVertexAttribArray(0));

    // // Setup element array buffer.
    // CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kGeometryVao][kIndexBuffer]));
    // CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * obj_faces.size() * 3, obj_faces.data(), GL_STATIC_DRAW));

    float aspect = static_cast<float>(window.width()) / window.height();
		glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), aspect, 0.0001f, 1000.0f);

    glm::mat4 view_matrix = camera.get_view_matrix();

    glUseProgram(program_id);

		// Pass uniforms in.
		glUniformMatrix4fv(uniform.projection, 1, GL_FALSE, &projection_matrix[0][0]);
		glUniformMatrix4fv(uniform.view, 1, GL_FALSE, &view_matrix[0][0]);
		glUniform4fv(      uniform.light_pos, 1, &light_position[0]);
    glUniform1i(       uniform.wireframe, wireframe_mode);

    glDrawElementsInstanced(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, NULL, instances.size());

    window.swapBuffers();
    glfwPollEvents();
  }
}