#include "RenderWindow.h"
#include "Camera.h"
#include "Perlin.h"

#include "Player.h"
#include "Shaders.h"
#include "World.h"
#include "Physics.h"

#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>

int main() {
  // RenderWindow window {"Hello World", 800, 600};
  RenderWindow window {"Hello World"};

  Player player;

  bool wireframe_mode = false;
  window.setKeyCallback([&](int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
      window.close();
    }

    if (key == GLFW_KEY_H && mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) {
      wireframe_mode = !wireframe_mode;
      glPolygonMode(GL_FRONT_AND_BACK, (wireframe_mode ? GL_LINE : GL_FILL));
    }

    player.handleKey(key, scancode, action, mods);
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
        player.camera.yaw(dx);
        player.camera.pitch(dy);

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
  World world{256, 256};
  srand(time(NULL));

  // compute total step count
  float s = glm::max<float>(world._width, world._height);
  float ds = 1/s;
  
  for (int i = 0; i < world._width; ++i) {
    for (int k = 0; k < world._height; ++k) {
      world(i, 50 + perlin(i / 50.f, k / 50.f) * 10, k) = 1;
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

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(   3, 1, GL_UNSIGNED_INT, sizeof(Instance), (void*)(sizeof(glm::vec3) + sizeof(GLuint)));
    glVertexAttribDivisor(    3, 1);

  GLuint program_id = 0;
	CreateProgram(program_id);
  glUseProgram(program_id);


  struct {
		GLint projection = 0;
		GLint view = 0;
		GLint light_pos = 0;
    GLint wireframe = 0;
    GLint bases = 0;
    GLint offs = 0;
	} uniform;

  uniform.projection = glGetUniformLocation(program_id, "projection");
	uniform.view       = glGetUniformLocation(program_id, "view");
	uniform.light_pos  = glGetUniformLocation(program_id, "light_position");
  uniform.wireframe  = glGetUniformLocation(program_id, "wireframe");
  uniform.bases      = glGetUniformLocation(program_id, "base_colors");
  uniform.offs       = glGetUniformLocation(program_id, "off_colors");

  std::vector<glm::vec4> base_colors (10, glm::vec4(0, 0, 0, 1));
  base_colors[0] = glm::vec4(0.2, 0.8, 0, 1);
  base_colors[1] = glm::vec4(0.2, 0.8, 0, 1);
  base_colors[2] = glm::vec4(1, 0, 0, 1);

  auto update_bases = [&](){
    glUniform4fv(uniform.bases, 10, (const GLfloat*)base_colors.data());
  };
  update_bases();
  
  std::vector<glm::vec4> off_colors (10, glm::vec4(0, 0, 0, 1));
  off_colors[0] = glm::vec4(0, 0.6, 0, 1);
  off_colors[1] = glm::vec4(0, 0.6, 0, 1);
  off_colors[2] = glm::vec4(1, 0, 0, 1);

  auto update_offs = [&](){
    glUniform4fv(uniform.offs, 10, (const GLfloat*)off_colors.data());
  };
  update_offs();


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
    
    glm::vec3 l = player.camera.look();
    auto forward = glm::normalize(glm::vec3(l.x, 0, l.z)) * Camera::zoom_speed;
    if (window.getKey(GLFW_KEY_W)) { player.camera.translate(forward); }
    if (window.getKey(GLFW_KEY_S)) { player.camera.translate(-forward); }
    if (window.getKey(GLFW_KEY_A)) { player.camera.strafe(-1); }
    if (window.getKey(GLFW_KEY_D)) { player.camera.strafe(1); }
    if (window.getKey(GLFW_KEY_UP)) { player.camera.translate(player.camera.up() * glm::vec3(0.2)); }
    if (window.getKey(GLFW_KEY_DOWN)) { player.camera.translate(-player.camera.up() * glm::vec3(0.2)); }

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

    glm::mat4 view_matrix = player.camera.get_view_matrix();

    glm::vec3 eye = player.camera.eye();

    // COLLISION TESTING

    auto isAir = [&](int i, int j, int k) {
      if (i < 0 || j < 0 || k < 0 
        || i >= world._width 
        || j >= CHUNK_HEIGHT
        || k >= world._height) {
        return true;
      }

      return world(i, j, k) == 0;
    };

    glm::ivec3 world_index = glm::floor(eye/*  + glm::vec3(0, -1.75, 0) */);
    std::cout << glm::to_string(world_index) << std::endl;
    std::cout << not isAir(world_index.x, world_index.y, world_index.z) << std::endl;
    

    bool collided = false;
    for (int i = -1; i <= 1; ++i)
    for (int k = -1; k <= 1; ++k) 
    for (int j = -3; j <= 1; ++j) {
      glm::ivec3 box = world_index + glm::ivec3(i, j, k);
            
      if (not isAir(box.x, box.y, box.z)) {
        if (Physics::verticalCollision(box, eye.y - 1.75, eye.y)
            && Physics::horizontalCollision(box, eye, 0.5)
          ) { 
          collided = true;
          break;
        }
      }
    }

    if (collided) {
      base_colors[0] = base_colors[2];
      off_colors[0] = off_colors[2];
    } else {
      base_colors[0] = base_colors[1];
      off_colors[0] = off_colors[1];
    }
    update_bases();
    update_offs();

    glUseProgram(program_id);

		// Pass uniforms in.
		glUniformMatrix4fv(uniform.projection, 1, GL_FALSE, &projection_matrix[0][0]);
		glUniformMatrix4fv(uniform.view,       1, GL_FALSE, &view_matrix[0][0]);
		glUniform4fv(      uniform.light_pos,  1, &light_position[0]);
    glUniform1i(       uniform.wireframe,  wireframe_mode);

    glDrawElementsInstanced(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, NULL, instances.size());

    window.swapBuffers();
    glfwPollEvents();
  }
}