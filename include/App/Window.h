#pragma once

#include <Windows.h>

struct IInputReceiver;
struct IWindowEventListener;

class Window {
public:
    Window() = default;
    ~Window() { Destroy(); };

    bool Create(int width, int height, const wchar_t* title);
    void Destroy();

    // メッセージポンプ
    bool ProcessMessages();

    // メッセージの処理
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    bool IsActive() const { return m_isActive; }

    HWND GetHwnd() const { return m_hWnd; }

    void SetInputReceiver(IInputReceiver* receiver) {
        m_inputReceiver = receiver;
    }

    void setWindowEventListener(IWindowEventListener* listener) {
        m_windowEventListener = listener;
    }

private:
    HINSTANCE m_hInst = nullptr;
    HWND m_hWnd       = nullptr;

    // イベント受け取り用インターフェース
    IInputReceiver* m_inputReceiver             = nullptr;
    IWindowEventListener* m_windowEventListener = nullptr;

    bool m_isActive = false;
};