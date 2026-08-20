/// @file ModelLoader.h
/// @brief モデルのロード

#pragma once

#include <filesystem>
#include <memory>

#include "Engine/Model/Model.h"
#include "directxtk12/ResourceUploadBatch.h"

// 前方宣言
class TextureManager;
class GraphicsDevice;

class ModelLoader {
public:
    /// @brief 初期化，必要なポインタの受け取り
    bool Init(GraphicsDevice& graphicsDevice, TextureManager& textureManager);

    /// @brief 終了処理，ポインタの破棄
    void Term();

    /// @brief モデルのロード
    std::unique_ptr<Model> LoadModel(
        const std::filesystem::path& path, DirectX::ResourceUploadBatch& batch);

private:
    GraphicsDevice* m_pGraphicsDevice = nullptr;
    TextureManager* m_pTextureManager = nullptr;
};