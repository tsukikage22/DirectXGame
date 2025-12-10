#pragma once

#include <d3d12.h>

#include <memory>

#include "Engine/MaterialGPU.h"
#include "Engine/MeshGPU.h"

class Model {
public:
    Model()  = default;
    ~Model() = default;

    const std::vector<std::unique_ptr<MeshGPU>>& GetMeshes() const {
        return m_meshes;
    }
    const std::vector<std::unique_ptr<MaterialGPU>>& GetMaterials() const {
        return m_materials;
    }

private:
    std::vector<std::unique_ptr<MeshGPU>> m_meshes;
    std::vector<std::unique_ptr<MaterialGPU>> m_materials;
};