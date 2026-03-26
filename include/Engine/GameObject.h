#pragma once

#include <d3d12.h>

#include "Engine/Model.h"
#include "Engine/Transform.h"

class GameObject {
public:
    GameObject(Model* pModel);
    ~GameObject();

    /// @brief モデルをセット
    /// @param pModel
    bool SetModel(Model* pModel);

    //=========================================
    // アクセサ
    //=========================================
    Transform& GetTransform() { return m_transform; }
    Model& GetModel() { return *m_model; }
    uint32_t GetIndex() const { return m_objectIndex; }

    void SetIndex(uint32_t index) { m_objectIndex = index; }

private:
    Transform m_transform;
    Model* m_model = nullptr;

    uint32_t m_objectIndex = 0;  // FrameResource内のTransformGPUと対応付けるID
};