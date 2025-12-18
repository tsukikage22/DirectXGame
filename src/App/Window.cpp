#include "App/Window.h"

namespace /* anonymous */ {
/// @brief ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto instance =
        reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE: {
            auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        } break;

        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;

        default: {
        } break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// ウィンドウクラス名
const auto ClassName = TEXT("DirectXGameWindowClass");
}  // namespace

bool Window::Create(int width, int height, const wchar_t* title) {
    // インスタンスハンドルを取得
    m_hInst = GetModuleHandle(nullptr);
    if (m_hInst == nullptr) {
        return false;
    }

    // ウィンドウクラスの設定
    WNDCLASSEX wc    = {};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hIcon         = LoadIcon(m_hInst, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(m_hInst, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
    wc.lpszMenuName  = nullptr;
    wc.lpszClassName = ClassName;

    // ウィンドウの登録
    if (!RegisterClassEx(&wc)) {
        return false;
    }

    // ウィンドウサイズの設定
    RECT rc    = {};
    rc.right   = static_cast<LONG>(width);
    rc.bottom  = static_cast<LONG>(height);
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect(&rc, style, FALSE);

    // ウィンドウの作成
    m_hWnd = CreateWindowEx(0, ClassName, TEXT("DirectX Game"), style,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, m_hInst, this);

    if (m_hWnd == nullptr) {
        return false;
    }

    // ウィンドウの表示
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return true;
}

void Window::Destroy() {
    if (m_hInst != nullptr) {
        UnregisterClass(ClassName, m_hInst);
    }

    m_hInst = nullptr;
    m_hWnd  = nullptr;
}

bool Window::ProcessMessages() {
    MSG msg = {};

    // メッセージループ
    // そのフレームのメッセージをすべて処理する
    // QUITメッセージが来たらfalseを返す
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}
