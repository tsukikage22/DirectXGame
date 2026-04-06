/// @file ModelLoader.h
/// @brief モデルのロード

#pragma once

#include <d3d12.h>

#include <filesystem>
#include <memory>

#include "Engine/DescriptorPool.h"
#include "Engine/Model.h"
#include "Engine/Scene.h"
#include "Engine/TextureManager.h"
#include "directxtk12/ResourceUploadBatch.h"

class ModelLoader {
public:
    /// @brief 初期化，必要なポインタの受け取り
    bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
        TextureManager* pTextureManager);

    /// @brief 終了処理，ポインタの破棄
    void Term();

    /// @brief モデルのロード
    std::unique_ptr<Model> LoadModel(
        const std::filesystem::path& path, DirectX::ResourceUploadBatch& batch);

private:
    ID3D12Device* m_pDevice           = nullptr;
    DescriptorPool* m_pPoolCBV        = nullptr;
    TextureManager* m_pTextureManager = nullptr;
};