#pragma once

#include <d3d12.h>

#include "Engine/Model/Model.h"
#include "Engine/Scene/Transform.h"
#include "Engine/Shader/TransformGPU.h"

class GameObject {
public:
    GameObject(Model* pModel);
    ~GameObject();

    /// @brief モデルをセット
    /// @param pModel
    bool SetModel(Model* pModel);

    /// @brief ワールド行列CBの更新
    void UpdateTransformGPU(int frameIndex);

    //=========================================
    // アクセサ
    //=========================================
    Transform& GetTransform() { return m_transform; }
    TransformGPU& GetTransformGPU(int frameIndex) {
        return m_transformGPU[frameIndex];
    }
    const Model& GetModel() const { return *m_model; }

    void SetIndex(uint32_t index) { m_index = index; }
    uint32_t GetIndex() const { return m_index; }

private:
    Transform m_transform;
    TransformGPU m_transformGPU[2];  // ワールド行列のシェーダーリソース
    Model* m_model = nullptr;

    uint32_t m_index = 0;  // シーン内のオブジェクトインデックス
};