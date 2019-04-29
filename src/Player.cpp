#include "Player.h"
#include "World.h"
#include "Physics.h"
#include <iostream>

void Player::handleTick(const World& world) {
  // if not on ground, tick gravity
  if (_current_mode == Mode::Survival) {
    if (not _grounded) {
      // tick gravity
      _velocity_y -= 0.5 * 1/60.f;


      // apply vel_y
      camera.translate(glm::vec3(0, _velocity_y, 0));
    }
    _grounded = grounded(world);

    // FIXME: cast ray to ground and then ground if reached by current velocity and then place player on ground
    if (_grounded) {
      auto wi = glm::round(feet());
      if (world(wi.x, wi.y, wi.z)) {
        camera.setPos({feet().x, wi.y + 0.5f + 1.75f, feet().z});
      }
    }
  }
  // FIXME: maybe: if we're collided and grounded bump up
}


bool Player::grounded(const World& world) {

  glm::ivec3 world_index = glm::round(head());

  for (int i = -1; i <= 1; ++i)
  for (int k = -1; k <= 1; ++k) 
  for (int j = -3 ; j <= -1; ++j) {
    glm::ivec3 box = world_index + glm::ivec3(i, j, k);
    if (not world.isAir(box.x, box.y, box.z)) {
      if (Physics::verticalCollision(box, feet().y, head().y)
          && Physics::horizontalCollision(box, head(), 0.5)
        ) {
        return true;
      }
    }
  }

  return false;
}

bool Player::collided(const World& world) {

  glm::ivec3 world_index = blockPosition();

  for (int i = -1; i <= 1; ++i)
  for (int k = -1; k <= 1; ++k) 
  for (int j = -5; j <= 1; ++j) {
    glm::ivec3 box = world_index + glm::ivec3(i, j, k);
    if (not world.isAir(box.x, box.y, box.z)) {
      if (Physics::verticalCollision(box, feet().y+0.1, head().y)
          && Physics::horizontalCollision(box, head(), 0.5))
      {
        return true;
      }
    }
  }

  return false;
}

void Player::handleMouse(int button, int action, int mods, World& world) {
  if (_current_mode == Player::Mode::Creative && action == GLFW_PRESS) {
    bool found = false;
    bool placement_found = false;
    glm::ivec3 block{-1, -1, -1};
    glm::ivec3 prev{-1, -1, -1};
    for (float k = 0; k < 10; k += 0.05) {
      glm::ivec3 currblock = World::toBlock(head() + glm::vec3(k) * camera.look());
      if (currblock != block) {
        prev = block;
        block = currblock;
        placement_found = true;
      }
      if (world(block.x, block.y, block.z) != 0) {
        found = true;
        break;
      }
    }

    if (not found) {
      return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      world(block.x, block.y, block.z) = 0;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
      auto& world_block = world(block.x, block.y, block.z);
      if (world_block != 0) {
        _held_block = world_block;
      }
    }

    if (placement_found && button == GLFW_MOUSE_BUTTON_RIGHT && _held_block != 0) {
      world(prev.x, prev.y, prev.z) = _held_block;
    }

    world._chunks[World::toChunk(block)]._instances.clear();
    world._dirty = true;
  }
}