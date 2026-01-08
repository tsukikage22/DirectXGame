#include "Engine/InputSystem.h"

InputSystem::InputSystem()  = default;
InputSystem::~InputSystem() = default;

void InputSystem::BeginFrame() {
    // キーボード状態の更新
    m_KeyPrev = m_KeyCur;

    // マウス移動量の更新
    m_Mouse.dx = m_Mouse.x - m_Mouse.prevX;
    m_Mouse.dy = m_Mouse.y - m_Mouse.prevY;

    // ホイール移動量更新
    m_Mouse.wheel = 0;

    // マウス状態更新
    m_Mouse.prevX      = m_Mouse.x;
    m_Mouse.prevY      = m_Mouse.y;
    m_Mouse.buttonPrev = m_Mouse.buttonCur;
}

//==============================
// 入力取得API
//==============================
// キーボード
// キーの現在状態
bool InputSystem::IsKeyDown(int vk) const { return m_KeyCur[vk]; }
// キーが押された瞬間
bool InputSystem::WasKeyPressed(int vk) const {
    return m_KeyCur[vk] && !m_KeyPrev[vk];
}
// キーが離された瞬間
bool InputSystem::WasKeyReleased(int vk) const {
    return !m_KeyCur[vk] && m_KeyPrev[vk];
}

// マウス
int InputSystem::MouseDX() const { return m_Mouse.dx; }
int InputSystem::MouseDY() const { return m_Mouse.dy; }
bool InputSystem::IsMouseDown(Button b) const {
    return m_Mouse.buttonCur[static_cast<size_t>(b)];
}
bool InputSystem::WasMousePressed(Button b) const {
    return m_Mouse.buttonCur[static_cast<size_t>(b)] &&
           !m_Mouse.buttonPrev[static_cast<size_t>(b)];
}
bool InputSystem::WasMouseReleased(Button b) const {
    return !m_Mouse.buttonCur[static_cast<size_t>(b)] &&
           m_Mouse.buttonPrev[static_cast<size_t>(b)];
}

//==============================
// イベント注入API
//==============================
// キーボード
void InputSystem::OnKeyDown(uint32_t vk) { m_KeyCur[vk] = true; }
void InputSystem::OnKeyUp(uint32_t vk) { m_KeyCur[vk] = false; }

// マウス
void InputSystem::OnMouseMove(int x, int y) {
    m_Mouse.x = x;
    m_Mouse.y = y;
}
void InputSystem::OnMouseDown(Button b) {
    m_Mouse.buttonCur[static_cast<size_t>(b)] = true;
}
void InputSystem::OnMouseUp(Button b) {
    m_Mouse.buttonCur[static_cast<size_t>(b)] = false;
}