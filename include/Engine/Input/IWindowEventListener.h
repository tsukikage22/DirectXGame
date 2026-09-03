#pragma once

#include <cstdint>

struct IWindowEventListener
{
public:
    virtual ~IWindowEventListener() = default;

    /// @brief ウィンドウ移動時の処理
    virtual void OnWindowMoved() = 0;

    virtual void OnWindowResized(uint32_t width, uint32_t height) = 0;

    // TODO: フォーカス，最小化などのイベントも追加する
};
