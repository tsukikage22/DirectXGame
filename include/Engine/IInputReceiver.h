#pragma once

#include <cstdint>

// マウスボタン列挙型
enum class Button {
    Left,
    Right,
    Middle,

    Count
};

/// @brief 入力受け取りインターフェース
struct IInputReceiver {
public:
    virtual ~IInputReceiver() = default;

    //==================================
    // イベント注入API
    //==================================
    // キーボード
    virtual void OnKeyDown(uint32_t vk) = 0;
    virtual void OnKeyUp(uint32_t vk)   = 0;

    // マウス
    virtual void OnMouseMove(int x, int y) = 0;
    virtual void OnMouseDown(Button b)     = 0;
    virtual void OnMouseUp(Button b)       = 0;
};