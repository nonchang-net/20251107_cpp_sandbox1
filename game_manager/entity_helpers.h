#pragma once

#include <SDL3/SDL.h>

#include <functional>
#include <memory>
#include <string>

namespace MyGame {

// 前方宣言
class Entity;
class Locator;
class Rotater;
class VelocityMove;
class AngularVelocity;
class RectRenderer;
class RotatedRectRenderer;
class TextRenderer;
class UIAnchorComponent;
enum class UIAnchor;

/**
 * @brief RectEntityをコンポーネントで代替するヘルパー関数
 *
 * Locator + VelocityMove + RectRendererを持つEntityを作成します。
 * RectEntityの完全な代替となります。
 *
 * @param layer レイヤー番号
 * @param x X座標
 * @param y Y座標
 * @param w 幅
 * @param h 高さ
 * @param color 色
 * @return 作成されたEntity
 */
std::unique_ptr<Entity> createRectEntity(
    int layer, float x, float y, float w, float h, SDL_Color color);

/**
 * @brief RotateRectEntityをコンポーネントで代替するヘルパー関数
 *
 * Locator + Rotater + VelocityMove + AngularVelocity + RotatedRectRendererを持つEntityを作成します。
 * RotateRectEntityの完全な代替となります。
 *
 * @param layer レイヤー番号
 * @param x X座標
 * @param y Y座標
 * @param w 幅
 * @param h 高さ
 * @param color 色
 * @param angle 初期回転角度（度数法）
 * @param pivot_x X方向のピボット位置（0.0～1.0）
 * @param pivot_y Y方向のピボット位置（0.0～1.0）
 * @return 作成されたEntity
 */
std::unique_ptr<Entity> createRotateRectEntity(
    int layer, float x, float y, float w, float h, SDL_Color color,
    float angle = 0.0f, float pivot_x = 0.5f, float pivot_y = 0.5f);

/**
 * @brief TextEntityをコンポーネントで代替するヘルパー関数（静的テキスト）
 *
 * Locator + TextRendererを持つEntityを作成します。
 * UIAnchorを指定することで、UI要素として扱うことができます。
 *
 * @param layer レイヤー番号
 * @param x X座標（UIAnchorがある場合はオフセット、ない場合はワールド座標）
 * @param y Y座標（UIAnchorがある場合はオフセット、ない場合はワールド座標）
 * @param text 表示するテキスト
 * @param color 色
 * @param anchor UIアンカー（nullptrの場合はゲーム要素として扱う）
 * @return 作成されたEntity
 */
std::unique_ptr<Entity> createTextEntity(
    int layer, float x, float y, const std::string& text,
    SDL_Color color = {255, 255, 255, 255},
    const UIAnchor* anchor = nullptr);

/**
 * @brief TextEntityをコンポーネントで代替するヘルパー関数（動的テキスト）
 *
 * Locator + TextRendererを持つEntityを作成します。
 * テキストを返す関数を渡すことで、動的に更新されるテキストを表示できます。
 *
 * @param layer レイヤー番号
 * @param x X座標（UIAnchorがある場合はオフセット、ない場合はワールド座標）
 * @param y Y座標（UIAnchorがある場合はオフセット、ない場合はワールド座標）
 * @param text_provider テキストを返す関数
 * @param color 色
 * @param anchor UIアンカー（nullptrの場合はゲーム要素として扱う）
 * @return 作成されたEntity
 */
std::unique_ptr<Entity> createTextEntity(
    int layer, float x, float y, std::function<std::string()> text_provider,
    SDL_Color color = {255, 255, 255, 255},
    const UIAnchor* anchor = nullptr);

}  // namespace MyGame

// 実装部分をインクルード
#include "entity_helpers_impl.h"
