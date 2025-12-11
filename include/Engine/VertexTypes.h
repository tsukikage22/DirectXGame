#pragma once

#include <DirectXMath.h>
#include <d3d12.h>

#include <vector>

/// @brief 基本の頂点フォーマット
struct StandardVertex {
    DirectX::XMFLOAT3 position;  // 座標
    DirectX::XMFLOAT3 normal;    // 法線
    DirectX::XMFLOAT3 tangent;   // 接空間
    DirectX::XMFLOAT2 texcoord;  // テクスチャ座標
    DirectX::XMFLOAT4 color;     // 頂点カラー

    /// @brief 頂点レイアウトの取得
    static std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
        return { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                     D3D12_APPEND_ALIGNED_ELEMENT,
                     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT,
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT,
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT,
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT,
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } };
    }
};