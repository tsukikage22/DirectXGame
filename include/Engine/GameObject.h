#pragma once

#include "Engine/Model.h"
#include "Engine/Transform.h"

class GameObject {
public:
    GameObject();
    ~GameObject();

    void Update();

    //=========================================
    // アクセサ
    //=========================================
    Transform& GetTransform() { return m_transform; }
    Model& GetModel() { return *m_model; }

private:
    Transform m_transform;
    Model* m_model = nullptr;
};