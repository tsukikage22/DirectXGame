/// @file Camera.h
/// @brief カメラの管理
#pragma once

#include <DirectXMath.h>

class Camera {
public:
    Camera();

    //================================
    // カメラの設定
    //================================
    /// @brief 位置の設定
    /// @param position
    void SetPosition(const DirectX::XMFLOAT3& position) {
        m_position = position;
    };

    /// @brief 向きの設定（角度）
    /// @param rotation
    void SetRotation(const DirectX::XMFLOAT3& rotation) {
        m_rotation = rotation;
    };

    /// @brief 向きの設定（注視点）
    /// @param target
    void SetTarget(const DirectX::XMFLOAT3& target) { m_target = target; };

    /// @brief 垂直視野角の設定
    /// @param fovY
    void SetFovY(float fovY) { m_fovY = fovY; };

    /// @brief アスペクト比の設定
    /// @param aspect
    void SetAspect(float aspect) { m_aspect = aspect; };

    /// @brief 最も近い描画距離の設定
    /// @param nearZ
    void SetNearZ(float nearZ) { m_nearZ = nearZ; };

    /// @brief 最も遠い描画距離の設定
    /// @param farZ
    void SetFarZ(float farZ) { m_farZ = farZ; };

    //================================
    // 行列の計算
    //================================
    /// @brief ビュー行列の計算
    /// @return ビュー行列
    DirectX::XMMATRIX GetViewMatrix() const;

    /// @brief 射影行列の計算
    /// @return 射影行列
    DirectX::XMMATRIX GetProjectionMatrix() const;

private:
    // カメラの状態 TODO: 回転はクォータニオンの方が良い
    DirectX::XMFLOAT3 m_position;  // カメラの位置
    DirectX::XMFLOAT3 m_rotation;  // カメラの向き
    DirectX::XMFLOAT3 m_target;    // カメラの注視点
    DirectX::XMFLOAT3 m_up;        // カメラの上方向ベクトル
    float m_fovY;                  // 垂直視野角（ラジアン）
    float m_aspect;                // アスペクト比
    float m_nearZ = 1.0f;          // ニアクリップ距離
    float m_farZ  = 1000.0f;       // ファークリップ距離

    // 行列
    DirectX::XMFLOAT4X4 m_viewMatrix;  // ビュー行列
    DirectX::XMFLOAT4X4 m_projMatrix;  // 射影行列
};
