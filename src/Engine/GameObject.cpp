#include "Engine/GameObject.h"

// コンストラクタ
GameObject::GameObject(Model* pModel) { SetModel(pModel); }

GameObject::~GameObject() = default;

// モデルをセットして初期化
bool GameObject::SetModel(Model* pModel) {
    if (pModel == nullptr) {
        return false;
    }
    m_model = pModel;
    return true;
}
