#include "Player.h"
#include "World.h"

void Player::handleTick(const World& world){
  // generate impulse to jerk player if collided
  // if movement resolves collision, no impulse
  // otherwise impulse

  // if not on ground, tick gravity
  if (_current_mode == Mode::Survival) {
    if (not _grounded) {
      // tick gravity
      _velocity_y -= 0.5 * 1/60.f;

      // FIXME: cast ray to ground and then ground if reached by current velocity and then place player on ground

      // apply vel_y
      camera.translate(glm::vec3(0, _velocity_y, 0));
    }
    _grounded = grounded(world);
    if (_grounded) {
      auto wi = glm::floor(feet());
      if (world(wi.x, wi.y, wi.z)) {
        camera.setPos({feet().x, wi.y + 2.25f, feet().z});
      }
    }
  }
  // FIXME: maybe: if we're collided and grounded bump up

}


  bool Player::grounded(const World& world) {

    glm::ivec3 world_index = glm::floor(head());

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