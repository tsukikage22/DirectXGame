/// @file   main.cpp
/// @brief  エントリーポイント

#include "App/Application.h"
#include "App/Window.h"

// エントリーポイント
int wmain(int argc, wchar_t** argv, wchar_t** envp) {
    // アプリケーションの実行
    Application app;
    return app.Run();
}