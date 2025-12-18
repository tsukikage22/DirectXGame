#pragma once

#include <d3d12.h>
#include <windows.h>
#include <wrl.h>

#include <sstream>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace dxdebug {

/// @brief デバッグレイヤーを有効化
inline void EnableDebugLayer() {
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
        debug->EnableDebugLayer();

        ComPtr<ID3D12Debug1> debug1;
        if (SUCCEEDED(debug.As(&debug1))) {
            debug1->SetEnableGPUBasedValidation(TRUE);
        }
    }
#endif
}

/// @brief InfoQueueの設定
inline void SetupInfoQueue(ID3D12Device* pDevice) {
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(pDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
    }
#endif
}

/// @brief InfoQueueのメッセージを出力
inline void DumpInfoQueueMessages(ID3D12Device* pDevice) {
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        return;
    }

    const UINT64 numMessages =
        infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
    for (UINT64 i = 0; i < numMessages; i++) {
        // メッセージサイズを取得
        SIZE_T sz = 0;
        infoQueue->GetMessage(i, nullptr, &sz);

        std::vector<uint8_t> messageData(sz);
        auto* msg = reinterpret_cast<D3D12_MESSAGE*>(messageData.data());

        // メッセージを取得
        infoQueue->GetMessage(i, msg, &sz);

        const char* severity = "UNKNOWN";
        switch (msg->Severity) {
            case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                severity = "CORRUPTION";
                break;
            case D3D12_MESSAGE_SEVERITY_ERROR:
                severity = "ERROR";
                break;
            case D3D12_MESSAGE_SEVERITY_WARNING:
                severity = "WARNING";
                break;
            case D3D12_MESSAGE_SEVERITY_INFO:
                severity = "INFO";
                break;
            case D3D12_MESSAGE_SEVERITY_MESSAGE:
                severity = "MESSAGE";
                break;
            default:
                break;
        }

        std::stringstream ss;
        ss << "[D3D12 " << severity << "] " << msg->pDescription << "\n";
        OutputDebugStringA(ss.str().c_str());
    }
    infoQueue->ClearStoredMessages();
#endif
}

/// @brief HRESULTを文字列に変換
inline std::wstring HrToMessage(HRESULT hr) {
    wchar_t* pMsgBuf = nullptr;
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS;
    DWORD lang = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    DWORD len  = FormatMessageW(
        flags, nullptr, (DWORD)hr, lang, (LPWSTR)&pMsgBuf, 0, nullptr);

    std::wstring msg =
        (len && pMsgBuf) ? std::wstring(pMsgBuf, len) : L"no message";

    if (pMsgBuf) {
        LocalFree(pMsgBuf);
    }
    return msg;
}

/// @brief HRESULTの出力
inline void OutputHr(
    HRESULT hr, const wchar_t* expr, const char* file, int line) {
    std::wstringstream ss;
    ss << L"[HRESULT] 0x" << std::hex << (unsigned)hr << L" " << HrToMessage(hr)
       << L" | " << file << L":" << std::dec << line << L" | " << expr << L"\n";
    OutputDebugStringW(ss.str().c_str());
}

}  // namespace dxdebug

/// @brief HRESULTチェックマクロ（失敗でreturn false）
#define CHECK_HR(device, call)                                      \
    do {                                                            \
        HRESULT hr__ = (call);                                      \
        if (FAILED(hr__)) {                                         \
            dxdebug::OutputHr(hr__, L## #call, __FILE__, __LINE__); \
            dxdebug::DumpInfoQueueMessages(device);                 \
            return false;                                           \
        }                                                           \
    } while (0)