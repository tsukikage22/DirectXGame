#include "Engine/Scene/GameObject.h"

// コンストラクタ
GameObject::GameObject(engine::ModelHandle handle) { SetModelHandle(handle); }

GameObject::~GameObject() = default;

// ワールド行列CBの更新
void GameObject::UpdateTransformGPU(int frameIndex) {
    m_transformGPU[frameIndex].Update(m_transform.CalcWorldMatrix());
}
