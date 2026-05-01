#pragma once

struct IWindowEventListener {
public:
    virtual ~IWindowEventListener() = default;

    /// @brief ウィンドウ移動時の処理
    virtual void OnWindowMoved() = 0;

    virtual void OnDisplayChanged() = 0;

    // TODO: ウィンドウサイズの変更，フォーカス，最小化などのイベントも追加する
};