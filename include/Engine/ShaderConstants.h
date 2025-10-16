#pragma once

#include <directxmath.h>

// 変換行列
struct TransformMatrix {
    DirectX::XMFLOAT4X4 world;       // ワールド行列
    DirectX::XMFLOAT4X4 view;        // ビュー行列
    DirectX::XMFLOAT4X4 projection;  // 射影行列
};