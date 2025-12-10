#pragma once

#include <DirectXMath.h>
#include <d3d12.h>

class Transform {
public:
    Transform();
    ~Transform();

    /// @brief ワールド行列の計算
    const DirectX::XMMATRIX CalcWorldMatrix() const;

    //=========================================
    // アクセサ
    //=========================================
    void SetPosition(const DirectX::XMFLOAT3& position) {
        m_position = position;
    }
    void SetRotation(const DirectX::XMFLOAT3& rotation) {
        m_rotation = rotation;
    }
    void SetScale(const DirectX::XMFLOAT3& scale) { m_scale = scale; }

private:
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;
    DirectX::XMFLOAT3 m_scale;
};