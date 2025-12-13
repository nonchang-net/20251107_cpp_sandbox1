#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace MyGame {

// 前方宣言
class Entity;

/**
 * @brief コンポーネントの基底クラス
 *
 * Entityに付与される機能の単位です。
 * 座標、回転、描画などの機能を個別のコンポーネントとして実装します。
 */
class Component {
 public:
  virtual ~Component() = default;

  /**
   * @brief コンポーネントの更新処理
   * @param entity このコンポーネントが所属するEntity
   * @param delta_time 前フレームからの経過時間（ミリ秒）
   */
  virtual void update(Entity* entity, Uint64 delta_time) {}

  /**
   * @brief コンポーネントの描画処理
   * @param entity このコンポーネントが所属するEntity
   * @param renderer SDLレンダラー
   */
  virtual void render(Entity* entity, SDL_Renderer* renderer) {}
};

}  // namespace MyGame

// 個別のコンポーネントヘッダーをインクルード
// 注：これらのヘッダーは前方宣言でComponentを使用しているため、
// Componentの完全な定義の後にインクルードする必要があります
#include "components/transform.h"
#include "components/velocity.h"
#include "components/direction.h"
#include "components/ui_anchor.h"
#include "components/sprite_animator.h"
#include "components/bounce_on_edge.h"
#include "components/blink.h"
#include "components/dynamic_pivot.h"
#include "components/renderers/rect_renderer.h"
#include "components/renderers/rotated_rect_renderer.h"
#include "components/renderers/text_renderer.h"
#include "components/renderers/sprite_renderer.h"
