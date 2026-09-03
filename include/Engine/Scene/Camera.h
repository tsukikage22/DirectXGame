/// @file Camera.h
/// @brief カメラの管理
#pragma once

#include <DirectXCollision.h>
#include <DirectXMath.h>

#include "Engine/Scene/Transform.h"

class Camera
{
public:
    Camera();

    //================================
    // カメラの設定
    //================================
    /// @brief 垂直視野角の設定
    /// @param fovYDeg 垂直視野角（度）
    void SetFovYDeg(float fovYDeg)
    {
        m_fovYRad = DirectX::XMConvertToRadians(fovYDeg);
    };

    /// @brief アスペクト比の設定
    /// @param aspect
    void SetAspect(float aspect)
    {
        m_aspect = aspect;
    };

    /// @brief 最も近い描画距離の設定
    /// @param nearZ
    void SetNearZ(float nearZ)
    {
        m_nearZ = nearZ;
    };

    /// @brief 最も遠い描画距離の設定
    /// @param farZ
    void SetFarZ(float farZ)
    {
        m_farZ = farZ;
    };

    /// @brief 露出パラメータの設定
    /// @param aperture
    /// @param shutterSpeed
    /// @param iso
    void SetExposure(float aperture, float shutterSpeed, float iso);

    /// @brief EV100の計算
    float ComputeEV100() const;

    /// @brief 露出の計算
    float ComputeExposure() const;

    /// @brief EV100を指定して露出パラメータを更新する
    /// @param ev100 EV100
    /// @param fixShutterSpeed trueの場合はシャッタースピードを固定
    void ApplyEV100(float ev100, bool fixShutterSpeed);

    /// @brief カメラの視錐台に基づく境界球の計算
    /// @param nearZ 影の描画範囲の最小距離
    /// @param farZ 影の描画範囲の最大距離
    DirectX::BoundingSphere ComputeBoundingSphere(float nearZ, float farZ) const;

    //================================
    // 行列の計算
    //================================
    DirectX::XMFLOAT4X4 GetViewMatrix();

    DirectX::XMFLOAT4X4 GetProjectionMatrix();

    //================================
    // アクセサ
    //================================
    Transform& GetTransform()
    {
        return m_transform;
    }
    const Transform& GetTransform() const
    {
        return m_transform;
    }

    float GetNearZ() const
    {
        return m_nearZ;
    }
    float GetAperture() const
    {
        return m_aperture;
    }
    float GetShutterSpeed() const
    {
        return m_shutterSpeed;
    }

private:
    /// @brief シャッタースピードを固定し，EV100を指定して絞り値を計算する
    float ComputeAperture(float ev100) const;

    /// @brief 絞り値を固定し，EV100を指定してシャッタースピードを計算する
    float ComputeShutterSpeed(float ev100) const;

    Transform m_transform;   // 位置や姿勢
    float m_fovYRad;         // 垂直視野角（ラジアン）
    float m_aspect;          // アスペクト比
    float m_nearZ = 1.0f;    // 描画範囲（最小）
    float m_farZ  = 1000.0f; // 描画範囲（最大）

    // 露出パラメータ
    float m_aperture     = 2.8f;         // 絞り値
    float m_shutterSpeed = 1.0f / 30.0f; // シャッタースピード
    float m_iso          = 100.0f;       // ISO感度

    // 行列
    DirectX::XMFLOAT4X4 m_viewMatrix; // ビュー行列
    DirectX::XMFLOAT4X4 m_projMatrix; // 射影行列
};
