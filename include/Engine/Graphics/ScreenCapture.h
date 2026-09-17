/// @file ScreenCapture.h
/// @brief 画面キャプチャに関する機能を提供するクラス

#pragma once

#include <d3d12.h>

#include "Engine/Core/ComPtr.h"

class ScreenCapture
{
public:
    ScreenCapture()  = default;
    ~ScreenCapture() = default;

    /// @brief 初期化を行う
    /// @param device デバイス
    void Init(ID3D12Device* device);

    /// @brief バックバッファの内容をコピーするコマンドを記録する
    bool RecordCopy(ID3D12GraphicsCommandList* commandList, ID3D12Resource* backBuffer);

    /// @brief キャプチャした内容をファイルに保存する
    /// @param filename 保存先のファイル名
    /// @param paperWhiteNits ディスプレイの白色の輝度
    bool SaveToFile(const wchar_t* filename, float paperWhiteNits);

private:
    ID3D12Device* m_pDevice                          = nullptr;
    engine::ComPtr<ID3D12Resource> m_pReadbackBuffer = nullptr;

    // コピー先のフットプリント情報
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT m_dstFootprint = {};
};
