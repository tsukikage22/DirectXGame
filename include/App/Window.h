#pragma once

#include <Windows.h>

struct IInputReceiver;

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

private:
    HINSTANCE m_hInst = nullptr;
    HWND m_hWnd       = nullptr;

    IInputReceiver* m_inputReceiver = nullptr;

    bool m_isActive = false;
};