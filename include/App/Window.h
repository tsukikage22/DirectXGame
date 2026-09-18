#pragma once

#include <Windows.h>

#include <cstdint>

struct IInputReceiver;
struct IWindowEventListener;

class Window
{
public:
    Window() = default;
    ~Window()
    {
        Destroy();
    };

    bool Create(int width, int height, const wchar_t* title);
    void Destroy();

    // メッセージポンプ
    bool ProcessMessages();

    // メッセージの処理
    LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    /// @brief ボーダーレスフルスクリーンの切り替え
    void ToggleFullscreen();

    bool IsActive() const
    {
        return m_isActive;
    }

    HWND GetHwnd() const
    {
        return m_hWnd;
    }

    void SetInputReceiver(IInputReceiver* receiver)
    {
        m_inputReceiver = receiver;
    }

    void SetWindowEventListener(IWindowEventListener* listener)
    {
        m_windowEventListener = listener;
    }

    bool IsMinimized() const
    {
        return m_isMinimized;
    }

private:
    /// @brief リサイズをイベントリスナーに通知する
    void NotifyResize();

    HINSTANCE m_hInst = nullptr;
    HWND m_hWnd       = nullptr;

    // イベント受け取り用インターフェース
    IInputReceiver* m_inputReceiver             = nullptr;
    IWindowEventListener* m_windowEventListener = nullptr;

    // ウィンドウの配置情報（フルスクリーン切り替え時に使用）
    WINDOWPLACEMENT m_windowPlacement = {};
    LONG_PTR m_windowStyle            = 0;

    // フルスクリーンかどうかのフラグ
    bool m_isFullscreen = false;

    uint32_t m_width          = 0;
    uint32_t m_height         = 0;
    uint32_t m_notifiedWidth  = 0;
    uint32_t m_notifiedHeight = 0;

    bool m_isActive     = false;
    bool m_isMinimized  = false;
    bool m_isSizeMoving = false;
};
