#pragma once

#include <Windows.h>

#include <array>
#include <cstdint>

#include "Engine/IInputReceiver.h"

class InputSystem : public IInputReceiver {
public:
    InputSystem();
    ~InputSystem();

    // フレーム開始時の処理
    void BeginFrame();

    //==================================
    // 入力取得API
    //==================================
    // キーボード
    bool IsKeyDown(int vk) const;       // キーが現在押されているか
    bool WasKeyPressed(int vk) const;   // キーが押された瞬間
    bool WasKeyReleased(int vk) const;  // キーが離された瞬間

    // マウス
    int MouseDX() const;
    int MouseDY() const;
    bool IsMouseDown(Button b) const;
    bool WasMousePressed(Button b) const;
    bool WasMouseReleased(Button b) const;

    //==================================
    // イベント注入API
    //==================================
    void OnKeyDown(uint32_t vk) override;
    void OnKeyUp(uint32_t vk) override;
    void OnMouseMove(int x, int y) override;
    void OnMouseDown(Button b) override;
    void OnMouseUp(Button b) override;

private:
    // キーボード状態
    std::array<bool, 256> m_KeyCur{};   // 現在のキー状態
    std::array<bool, 256> m_KeyPrev{};  // 前フレームのキー状態

    // マウス状態
    struct MouseState {
        int x     = 0;
        int y     = 0;
        int prevX = 0;
        int prevY = 0;
        int dx    = 0;
        int dy    = 0;
        int wheel = 0;
        std::array<bool, static_cast<size_t>(Button::Count)>
            buttonCur{};  // 現在のボタン状態
        std::array<bool, static_cast<size_t>(Button::Count)>
            buttonPrev{};  // 前フレームのボタン状態
    } m_Mouse;
};