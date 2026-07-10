#pragma once

#include <d3d12.h>

#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/GenHandle.h"
#include "Engine/Model/Model.h"
#include "Engine/Scene/Transform.h"
#include "Engine/Shader/TransformGPU.h"

class GameObject {
public:
    GameObject(engine::ModelHandle handle);
    ~GameObject();

    /// @brief ワールド行列CBの更新
    void UpdateTransformGPU(int frameIndex);

    //=========================================
    // アクセサ
    //=========================================
    Transform& GetTransform() { return m_transform; }
    TransformGPU& GetTransformGPU(int frameIndex) {
        return m_transformGPU[frameIndex];
    }

    void SetModelHandle(engine::ModelHandle handle) { m_modelHandle = handle; }
    const engine::ModelHandle GetModelHandle() const { return m_modelHandle; }

private:
    Transform m_transform;
    TransformGPU m_transformGPU
        [config::kFrameCount];  // ワールド行列のシェーダーリソース

    engine::ModelHandle m_modelHandle;
};