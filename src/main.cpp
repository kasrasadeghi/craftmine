#include "RenderWindow.h"
#include "Camera.h"
#include "TextRenderer.h"

#include "TerrainGen.h"
#include "Player.h"
#include "Shaders.h"
#include "World.h"
#include "Terrain.h"

#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>

#include <future>

int main() {
  srand(time(NULL));

  // RenderWindow window {"Hello World", 800, 600};
  RenderWindow window {"Hello World"};
  window.setMousePos(window.width()/2.f, window.height()/2.f);
  window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwWindowHint(GLFW_SAMPLES, 4);

  Player player;
  player.setPos(glm::vec3(2000, 100, 2000));

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

    if (player._current_mode == Player::Mode::Menger) {
      window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
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
    if (player._current_mode != Player::Mode::Menger) {
      float dx = mouse_x - window.width()/2.f;
      float dy = mouse_y - window.height()/2.f;
      player.camera.yaw(-dx/20.f);
      player.camera.pitch(-dy/20.f);
      window.setMousePos(window.width()/2.f, window.height()/2.f);
    } else {
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
    }
  });

  std::vector<glm::vec4> vertices;
  vertices.emplace_back(0.5, -0.5,  0.5, 1);
  vertices.emplace_back(0.5, -0.5, -0.5, 1);
  vertices.emplace_back(0.5,  0.5, -0.5, 1);

  std::vector<glm::uvec3> faces;
  faces.emplace_back(0, 1, 2);

  glViewport(0, 0, window.width(), window.height());
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDepthFunc(GL_LESS);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_MULTISAMPLE);

  TextRenderer tr {(float)window.width(), (float)window.height()};
  tr.renderText("generating terrain", window.width()/2 - 100, window.height()/2, 1, glm::vec4(1));
  window.swapBuffers();

  // create world with flat plane
  World world(player);
  TerrainGen::spawn(world, player);

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
  std::vector<glm::vec4> off_colors  (10, glm::vec4(0, 0, 0, 1));

  Terrain::setColors(base_colors, off_colors);

  auto update_bases = [&](){
    glUniform4fv(uniform.bases, 10, (const GLfloat*)base_colors.data());
  };
  update_bases();

  auto update_offs = [&](){
    glUniform4fv(uniform.offs, 10, (const GLfloat*)off_colors.data());
  };
  update_offs();

  glm::vec4 light_position {0, 1000, 0, 1};


  while (window.isOpen()) {
    // glfwGetFramebufferSize(window, &window_width, &window_height);
		glViewport(0, 0, window.width(), window.height());
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_MULTISAMPLE);

    if (player._current_mode != Player::Mode::Menger) {
      if (window.getKey(GLFW_KEY_W)) { player.move(2, world);; }
      if (window.getKey(GLFW_KEY_S)) { player.move(3, world); }
      if (window.getKey(GLFW_KEY_A)) { player.move(0, world); }
      if (window.getKey(GLFW_KEY_D)) { player.move(1, world); }
      if (window.getKey(GLFW_KEY_UP) || window.getKey(GLFW_KEY_SPACE)) { player.jump(); }
      if (window.getKey(GLFW_KEY_DOWN)) { player.moveDown(); }
    }
    
    glBindVertexArray(worldVAO);
  
    float aspect = static_cast<float>(window.width()) / window.height();
		glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 1000.0f);

    glm::mat4 view_matrix = player.camera.get_view_matrix();


    // COLLISION TESTING

    player.handleTick(world);
    world.handleTick(player);

    // if world is dirty
    if (world.dirty()) {

      for (const glm::ivec2& chunk_index : world._active_set) {
        if (not world.hasChunk(chunk_index)) {
          world._chunks[chunk_index] = {};
        }
        if (not world.isChunkGenerated(chunk_index)) {
          TerrainGen::chunk(world, chunk_index);
        }
      }

      // FIXME: simultaneous terrain gen is unstable
      // auto get_chunk = [&](glm::ivec2 chunk_index) {
        // if (not world.hasChunk(chunk_index)) {
        //   world._chunks[chunk_index] = {};
        // }
        // if (not world.isChunkGenerated(chunk_index)) {
        //   TerrainGen::chunk(world, chunk_index);
        // }
      // };

      // std::vector<std::future<void>> handles;

      // for (const glm::ivec2& chunk_index : world._active_set) {
      //   handles.emplace_back(std::async(std::launch::async, get_chunk, chunk_index));
      // }

      // for (auto &handle : handles) {
      //   handle.get();
      // }

      world.build(instances);
      glBindBuffer(GL_ARRAY_BUFFER, VBO.instances_buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Instance) * instances.size(), instances.data(), GL_STATIC_DRAW);
      world._dirty = false;
    }

    glUseProgram(program_id);

		// Pass uniforms in.
		glUniformMatrix4fv(uniform.projection, 1, GL_FALSE, &projection_matrix[0][0]);
		glUniformMatrix4fv(uniform.view,       1, GL_FALSE, &view_matrix[0][0]);
		glUniform4fv(      uniform.light_pos,  1, &light_position[0]);
    glUniform1i(       uniform.wireframe,  wireframe_mode);

    glDrawElementsInstanced(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, NULL, instances.size());

    tr.renderText(player._grounded ? "grounded" : "not grounded", 100, 200, 1, glm::vec4(1));
    tr.renderText((player.collided(world) ? "" : "not ") + std::string("collided"), 100, 230, 1, glm::vec4(1));
    tr.renderText("player pos:   " + glm::to_string(player.feet()), 200, 50, 1, glm::vec4(1));
    tr.renderText("player block: " + glm::to_string(player.blockPosition()), 200, 80, 1, glm::vec4(1));
    tr.renderText("chunk pos:    " + glm::to_string(World::toChunk(player.blockPosition())), 200, 110, 1, glm::vec4(1));
    tr.renderText("chunk loaded? " + std::string(world.hasChunk(World::toChunk(player.blockPosition())) ? "true" : "false"), 200, 140, 1, glm::vec4(1));
    tr.renderText("chunk genned? " + std::string(world.isChunkGenerated(World::toChunk(player.blockPosition())) ? "true" : "false"), 200, 170, 1, glm::vec4(1));
    for (int i = 0; i < 3; ++i) {
      auto p = player.blockPosition();
      tr.renderText((world(p.x, p.y - i, p.z) ? "1" : "0"), 1000, 100 + i*30, 1, glm::vec4(1));
    }

    window.swapBuffers();
    glfwPollEvents();
  }
}