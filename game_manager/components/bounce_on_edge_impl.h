#pragma once

#include "bounce_on_edge.h"

namespace MyGame {

// BounceOnEdgeの実装
inline void BounceOnEdge::update(Entity* entity, Uint64 delta_time) {
  auto* locator = entity->getComponent<Locator>();
  auto* velocity = entity->getComponent<VelocityMove>();
  auto* renderer = entity->getComponent<RectRenderer>();

  if (!locator || !velocity || !renderer) {
    return;
  }

  auto [x, y] = locator->getPosition();
  auto [w, h] = renderer->getSize();
  auto [vx, vy] = velocity->getVelocity();

  bool changed = false;
  if (x < 0 || x + w > screen_width_) {
    vx = -vx;
    changed = true;
  }
  if (y < 0 || y + h > screen_height_) {
    vy = -vy;
    changed = true;
  }

  if (changed) {
    velocity->setVelocity(vx, vy);
  }
}

}  // namespace MyGame
