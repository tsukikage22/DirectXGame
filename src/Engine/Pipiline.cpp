#include "Engine/Pipeline.h"

bool Pipeline::Create(
    ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc) {
    // 引数チェック
    if (pDevice == nullptr) {
        return false;
    }

    // シェーダーの読み込み

    // パイプラインステートの設定

    // パイプラインステートの生成
}