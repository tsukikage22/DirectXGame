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

    /// @brief 終了処理を行う
    void Term();

    /// @brief リソースの内容をリードバックバッファにコピーするコマンドを記録する
    /// @param pCmdList コマンドリスト
    /// @param pSource コピー元のリソース
    /// @note pSourceをCOPY_SOURCEに遷移させてから呼ぶこと
    bool RecordReadback(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pSource);

    /// @brief キャプチャした内容をファイルに保存する
    /// @param filePath 保存先のファイルパス
    /// @param paperWhiteNits scRGBへの変換に使う白色の輝度
    /// @note GPUの実行完了を待機して呼ぶこと
    bool SaveAsPNG(const wchar_t* filePath, float paperWhiteNits);

private:
    ID3D12Device* m_pDevice                          = nullptr;
    engine::ComPtr<ID3D12Resource> m_pReadbackBuffer = nullptr;

    // リードバックバッファのフットプリント情報
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT m_readbackFootprint = {};
};
