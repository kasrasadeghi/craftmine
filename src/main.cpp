#include "RenderWindow.h"
#include "Camera.h"
#include "TextRenderer.h"

#include "TerrainGen.h"
#include "Player.h"
#include "Shaders.h"
#include "ShaderUtil.h"
#include "World.h"
#include "Terrain.h"

#include <iostream>
#include <vector>
#include <sstream>

#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <future>

constexpr bool SHADOWS = false;

int main() {
  srand(time(NULL));

  // RenderWindow window {"Hello World", 1920, 1080};
  RenderWindow window {"Hello World"};
  window.setMousePos(window.width()/2.f, window.height()/2.f);
  window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwWindowHint(GLFW_SAMPLES, 4);

  Player player;
  player.setPos(glm::vec3(2000, 100, 2000));
  World world(player);

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

    player.handleMouse(button, action, mods, world);
  });

  window.setCursorCallback([&](double mouse_x, double mouse_y) {
    float dx = mouse_x - window.width()/2.f;
    float dy = mouse_y - window.height()/2.f;
    player.camera.yaw(-dx/20.f);
    player.camera.pitch(-dy/20.f);
    window.setMousePos(window.width()/2.f, window.height()/2.f);
  });

  std::vector<glm::vec4> vertices;
  vertices.emplace_back(0.5, -0.5,  0.5, 1);
  vertices.emplace_back(0.5, -0.5, -0.5, 1);
  vertices.emplace_back(0.5,  0.5, -0.5, 1);

  std::vector<glm::uvec3> faces;
  faces.emplace_back(0, 1, 2);

  glViewport(0, 0, window.width(), window.height());
  glClearColor(0.0, 0.0, 0.0, 0);

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
  TerrainGen::spawn(world, player);

  std::vector<Instance> instances;
  world.build(instances, player);

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

  ShaderSource program_sources;
  program_sources.vertex = world_vertex_shader;
  program_sources.geometry = world_geometry_shader;
  program_sources.fragment = world_fragment_shader;

	GLuint program_id = CreateProgram(program_sources, {"vertex_position", "instance_offset", "direction", "texture_index"});
  glUseProgram(program_id);

  struct {
		GLint projection = 0;
		GLint view = 0;
		GLint light_pos = 0;
    GLint light_space = 0;
    GLint wireframe = 0;
    GLint bases = 0;
    GLint offs = 0;
	} uniform;

  uniform.projection   = glGetUniformLocation(program_id, "projection");
	uniform.view         = glGetUniformLocation(program_id, "view");
	uniform.light_space  = glGetUniformLocation(program_id, "light_space");
	uniform.light_pos    = glGetUniformLocation(program_id, "light_position");
  uniform.wireframe    = glGetUniformLocation(program_id, "wireframe");
  uniform.bases        = glGetUniformLocation(program_id, "base_colors");
  uniform.offs         = glGetUniformLocation(program_id, "off_colors");

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

  std::vector<std::string> build_messages;
  auto str = [](auto o) -> std::string {
    std::stringstream i;
    i << o; 
    return i.str();
  };

  // Setup framebuffer for depth
  const unsigned int SHADOW_WIDTH = 10000;
  const unsigned int SHADOW_HEIGHT = 10000;

  GLuint depthFBO;
  glGenFramebuffers(1, &depthFBO);

  // setup depth texture
  GLuint depthMapTex;
  glGenTextures(1, &depthMapTex); 
  glBindTexture(GL_TEXTURE_2D, depthMapTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
      SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  // settings of texture
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  // bind shadow framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTex, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  

  double fps_counter_time = glfwGetTime();
  int framecounter = 0;
  while (window.isOpen()) {
    framecounter ++;
    // glfwGetFramebufferSize(window, &window_width, &window_height);
    if (window.getKey(GLFW_KEY_W)) { player.move(2, world);; }
    if (window.getKey(GLFW_KEY_S)) { player.move(3, world); }
    if (window.getKey(GLFW_KEY_A)) { player.move(0, world); }
    if (window.getKey(GLFW_KEY_D)) { player.move(1, world); }
    if (window.getKey(GLFW_KEY_UP) || window.getKey(GLFW_KEY_SPACE)) { player.jump(); }
    if (window.getKey(GLFW_KEY_DOWN) || window.getKey(GLFW_KEY_LEFT_SHIFT)) { player.moveDown(); }
    
    glBindVertexArray(worldVAO);

    player.handleTick(world);
    world.handleTick(player); // updates world._active set

    if (world._might_need_generation) {
      build_messages.clear();
      double start = glfwGetTime();
      build_messages.emplace_back("building active set of size " + str(world._active_set.size()));

      constexpr int gen_chunk_limit = 1;
      int gen_chunk_count = 0;
      bool incomplete = false;
      for (const glm::ivec2& chunk_index : world._active_set) {
        // FIXME: simplify/refactor this logic
        if (not world.hasChunk(chunk_index)) {
          world._chunks[chunk_index] = {};
        }
        if (not world.isChunkGenerated(chunk_index) && gen_chunk_count < gen_chunk_limit) {
          TerrainGen::chunk(world, chunk_index);
          gen_chunk_count ++;
        } else {
          // FIXME: I think incomplete is wrong
          incomplete = true;
        }
      }
      world._might_need_generation = incomplete;
      
      build_messages.emplace_back("generate chunk: " + str(glfwGetTime() - start));
    }

    world.build(instances, player);
    glBindBuffer(GL_ARRAY_BUFFER, VBO.instances_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Instance) * instances.size(), instances.data(), GL_STATIC_DRAW);


    float aspect = static_cast<float>(window.width()) / window.height();
    glm::mat4 projection_matrix(0);
    glm::mat4 view_matrix(0);
    glm::mat4 light_space_matrix(0);

    /// Draw scene
    glUseProgram(program_id);
    glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
    glClearColor(0.5, 0.5, 0.5, 1); // Sky color
    glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

    if (SHADOWS) {
      /// Render to Shadow Texture
      // Set rendering options
      glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
      glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
      // glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      
      // Compute uniforms
      glm::vec4 light_offset = glm::rotate(glm::vec4(0, 128 + 60, 0, 0), 0.5f * glm::sin((float)glfwGetTime()/5), glm::vec3(1, 0, 1));
      light_position = glm::vec4(glm::vec3{player.head().x, 0, player.head().z} + glm::vec3(light_offset), 1);

      light_space_matrix = glm::mat4(0);
      projection_matrix = glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, 50.f, 200.0f);
  		// projection_matrix = glm::perspective(glm::radians(120.0f), aspect, 5.f, 1000.0f);
      view_matrix = glm::lookAt(glm::vec3(light_position), player.head(), glm::vec3(1, 0, 0));
      // view_matrix = player.camera.get_view_matrix();
      
      // Pass uniforms in.
      glUniformMatrix4fv(uniform.projection, 1, GL_FALSE, &projection_matrix[0][0]);
      glUniformMatrix4fv(uniform.view,       1, GL_FALSE, &view_matrix[0][0]);
      glUniformMatrix4fv(uniform.light_space,1, GL_FALSE, &light_space_matrix[0][0]);
      glUniform4fv(      uniform.light_pos,  1, &light_position[0]);
      glUniform1i(       uniform.wireframe,  wireframe_mode);

      glDrawElementsInstanced(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, NULL, instances.size());
    }

    /// Render to Screen
    // Set rendering options
    glViewport(0, 0, window.width(), window.height());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compute uniforms
    light_space_matrix = projection_matrix * view_matrix;
		projection_matrix = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 1000.0f);
    view_matrix = player.camera.get_view_matrix();

    // Pass uniforms in.
		glUniformMatrix4fv(uniform.projection, 1, GL_FALSE, &projection_matrix[0][0]);
		glUniformMatrix4fv(uniform.view,       1, GL_FALSE, &view_matrix[0][0]);
		glUniformMatrix4fv(uniform.light_space,1, GL_FALSE, &light_space_matrix[0][0]);
		glUniform4fv(      uniform.light_pos,  1, &light_position[0]);
    glUniform1i(       uniform.wireframe,  wireframe_mode);

    glBindTexture(GL_TEXTURE_2D, depthMapTex);
    glDrawElementsInstanced(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, NULL, instances.size());
    

    /// Render Text
    tr.renderText(player._grounded ? "grounded" : "not grounded", 100, 200, 1, glm::vec4(1));
    tr.renderText((player.collided(world) ? "" : "not ") + std::string("collided"), 100, 230, 1, glm::vec4(1));
    tr.renderText("player pos:   " + glm::to_string(player.feet()), 200, 50, 1, glm::vec4(1));
    tr.renderText("player block: " + glm::to_string(player.blockPosition()), 200, 80, 1, glm::vec4(1));
    tr.renderText("chunk pos:    " + glm::to_string(World::toChunk(player.blockPosition())), 200, 110, 1, glm::vec4(1));
    tr.renderText("chunk loaded? " + std::string(world.hasChunk(World::toChunk(player.blockPosition())) ? "true" : "false"), 200, 140, 1, glm::vec4(1));
    tr.renderText("chunk genned? " + std::string(world.isChunkGenerated(World::toChunk(player.blockPosition())) ? "true" : "false"), 200, 170, 1, glm::vec4(1));
    tr.renderText("player block? " + Terrain::str(player._held_block), 100, window.height() - 100, 1, glm::vec4(1));
    tr.renderText("player mode: " + player.modeString(), window.width() - 500, 100, 1, glm::vec4(1));
    tr.renderText("+", window.width()/2, window.height()/2, 1, glm::vec4(1));
    tr.renderText(str(1 / (glfwGetTime() - fps_counter_time)) + " FPS", window.width() - 200, 50, 1, glm::vec4(1));
    fps_counter_time = glfwGetTime();
    for (int i = 0; i < 3; ++i) {
      auto p = player.blockPosition();
      auto y = p.y - i;

      if (y < CHUNK_HEIGHT && y >= 0) {
        tr.renderText((world(p.x, y, p.z) ? "1" : "0"), 1000, 100 + i*30, 1, glm::vec4(1));
      }
    }

    tr.renderText(str(world._chunks.size() * CHUNK_HEIGHT * CHUNK_SIZE * CHUNK_SIZE / 1024.f / 1024.f) + " MB", window.width() - 400, window.height()/2, 1, glm::vec4(1));
    // FIXME: also need to include instances in memory usage heuristics
    
    float msgi = 100;
    for (auto&& message : build_messages) {
      tr.renderText(message, 1600, msgi += 30, 1, glm::vec4(1));
    }

    window.swapBuffers();
    glfwPollEvents();
  }
}