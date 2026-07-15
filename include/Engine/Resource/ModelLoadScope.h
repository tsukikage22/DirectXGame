/// @file ModelLoadScope.h
/// @brief モデルロード用オブジェクト

#pragma once

#include <d3d12.h>
#include <directxtk12/ResourceUploadBatch.h>

#include <filesystem>
#include <memory>

#include "Engine/Core/GenHandle.h"

class CommandQueue;
class ModelLoader;
class Scene;

class ModelLoadScope {
public:
    ModelLoadScope(std::unique_ptr<DirectX::ResourceUploadBatch> pbatch,
        CommandQueue& queue, ModelLoader& loader, Scene& scene);

    ~ModelLoadScope();

    /// @brief モデルをロードし，シーンに登録する
    engine::ModelHandle LoadModel(const std::filesystem::path& path);

private:
    std::unique_ptr<DirectX::ResourceUploadBatch> m_pbatch;
    CommandQueue& m_queue;
    ModelLoader& m_loader;
    Scene& m_scene;

    // コピー・ムーブは不可（参照メンバを持つため）
    ModelLoadScope(const ModelLoadScope&)            = delete;
    ModelLoadScope& operator=(const ModelLoadScope&) = delete;
    ModelLoadScope(ModelLoadScope&&)                 = delete;
    ModelLoadScope& operator=(ModelLoadScope&&)      = delete;
};