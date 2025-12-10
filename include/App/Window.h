#pragma once

#include <Windows.h>

class Window {
public:
    Window() = default;
    ~Window() { Destroy(); };

    bool Create(int width, int height, const wchar_t* title);
    void Destroy();

    bool ProcessMessages();

    bool IsActive() const { return m_isActive; }

    HWND GetHwnd() const { return m_hWnd; }

private:
    HINSTANCE m_hInst = nullptr;
    HWND m_hWnd       = nullptr;

    bool m_isActive = false;
};