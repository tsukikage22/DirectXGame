#pragma once

#include "Engine/GameObject.h"

class Scene {
public:
    Scene();
    ~Scene();

    GameObject& CreateGameObject(Model* pModel);

    void RemoveGameObject(GameObject* pObj);

    void Update();

    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const {
        return m_gameObjects;
    }

private:
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
};