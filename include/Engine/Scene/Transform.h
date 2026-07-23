#pragma once

#include <DirectXMath.h>

// 座標系・回転について
// DirectXの慣習に従い，左手系・回転軸の正方向から原点を見て時計回り
// DirectXMathは行ベクトル規約のためワールド行列はS * R * Tの順で計算する
// 回転の順序はRoll(z), Pitch(x), Yaw(y)の順で行う
// また，行列はHLSLに渡す前に転置する必要がある（HLSLはcolumn-major，C++はrow-majorのため）
// XMQuaternionMultiply(Q1, Q2)はQ2*Q1を返す（Q1を先に適用）．
// 引数は行列の乗算と同様に「適用順」で左から並べる

namespace engine {
inline constexpr DirectX::XMFLOAT3 kForward = { 0.0f, 0.0f, 1.0f };
inline constexpr DirectX::XMFLOAT3 kUp      = { 0.0f, 1.0f, 0.0f };
inline constexpr DirectX::XMFLOAT3 kRight   = { 1.0f, 0.0f, 0.0f };
}  // namespace engine

class Transform {
public:
    Transform();
    ~Transform();

    /// @brief ワールド行列の計算
    DirectX::XMMATRIX CalcWorldMatrix() const;

    /// @brief
    /// オイラー角（度）で姿勢を設定する．これは初期姿勢の設定などに使い，それ以外は基本的にRotateを使う
    /// @param rotation
    void SetRotation(float pitch, float yaw, float roll);

    /// @brief ワールド座標系での回転操作
    void XM_CALLCONV RotateWorld(DirectX::FXMVECTOR axis, float angleDeg);

    /// @brief ローカル座標系での回転操作
    void XM_CALLCONV RotateLocal(DirectX::FXMVECTOR axis, float angleDeg);

    //=========================================
    // アクセサ
    //=========================================
    void SetPosition(const DirectX::XMFLOAT3& position) {
        m_position = position;
    }
    void SetScale(const DirectX::XMFLOAT3& scale) { m_scale = scale; }
    DirectX::XMFLOAT3 GetPosition() const { return m_position; }
    DirectX::XMFLOAT3 GetScale() const { return m_scale; }
    DirectX::XMFLOAT4 GetOrientation() const { return m_orientation; }

private:
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT4 m_orientation;
    DirectX::XMFLOAT3 m_scale;
};