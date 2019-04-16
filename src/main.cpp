#include "RenderWindow.h"
#include "Camera.h"

#include "Shaders.h"
#include "World.h"

#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

int main() {
  // RenderWindow window {"Hello World", 800, 600};
  RenderWindow window {"Hello World"};

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

  // create world with flat plane
  World world;
  srand(time(NULL));

  auto rand2 = []() -> glm::vec2 {
    float theta = rand()/(float)RAND_MAX * glm::two_pi<float>();
    return glm::vec2 {glm::cos(theta), glm::sin(theta)};
  };

  // gradients
  glm::vec2 g00 = rand2();
  glm::vec2 g01 = rand2();
  glm::vec2 g10 = rand2();
  glm::vec2 g11 = rand2();

  auto p4 = [](auto a, auto b, auto c, auto d){
    std::cout 
      << glm::to_string(a) << " "
      << glm::to_string(b) << " "
      << glm::to_string(c) << " "
      << glm::to_string(d) << std::endl;
  };

  // compute total step count
  // float s = glm::max<float>(world.width, world.height);
  float s = 16.f;
  float ds = 1/s;
  
  for (int i = 0; i < s; ++i) {
    for (int k = 0; k < s; ++k) {
      // distances
      glm::vec2 d00 = ds * glm::vec2(i, k);
      glm::vec2 d01 = ds * glm::vec2(i - (s-1), k);
      glm::vec2 d10 = ds * glm::vec2(i, k - (s-1));
      glm::vec2 d11 = ds * glm::vec2(i - (s-1), k - (s-1));

      // products
      float p00 = glm::dot(g00, d00);
      float p01 = glm::dot(g01, d01);
      float p10 = glm::dot(g10, d10);
      float p11 = glm::dot(g11, d11);

      // heights and interpolation
      float h0_ = glm::mix(p00, p01, d00.x);
      float h1_ = glm::mix(p10, p11, d00.x);
      float h__ = glm::mix(h0_, h1_, d00.y);

      float v = h__;
      
      world(i, 50 + v * 10, k) = 1;
    }
  }

  std::vector<Instance> instances;
  world.build(instances);

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

    glBufferData(GL_ARRAY_BUFFER, sizeof(Instance) * instances.size(), instances.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(    1, 3, GL_FLOAT, GL_FALSE, sizeof(Instance), (void*)0);
    glVertexAttribDivisor(    1, 1);

    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(   2, 1, GL_UNSIGNED_INT, sizeof(Instance), (void*)(sizeof(glm::vec3)));
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
    glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

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
		glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 1000.0f);

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