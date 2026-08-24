/// @file AssetLoadScope.h
/// @brief コンストラクタでbatchを受け取り，デストラクタでEndする

#pragma once

#include <d3d12.h>
#include <directxtk12/ResourceUploadBatch.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

#include "Engine/Core/GenHandle.h"

class CommandQueue;
class ModelLoader;
class Scene;
class IESProfile;
class EnvironmentMap;

class AssetLoadScope {
public:
    AssetLoadScope(std::unique_ptr<DirectX::ResourceUploadBatch> pbatch,
        CommandQueue& queue, ModelLoader& loader, Scene& scene,
        IESProfile& iesProfile, EnvironmentMap& environmentMap);

    ~AssetLoadScope();

    /// @brief モデルをロードし，シーンに登録する
    engine::ModelHandle LoadModel(const std::filesystem::path& path);

    /// @brief IESプロファイルをロードし，配光テクスチャを作成する
    std::optional<uint32_t> LoadIESProfile(const std::filesystem::path& path);

    /// @brief 環境マップの作成のためにHDRIを読み込み，テクスチャを作成する
    /// @param path HDRIファイルのパス
    /// @return 読み込みに成功したかどうか
    [[nodiscard]] bool LoadEnvironmentMap(const std::filesystem::path& path);

private:
    std::unique_ptr<DirectX::ResourceUploadBatch> m_pbatch;
    CommandQueue& m_queue;
    ModelLoader& m_loader;
    Scene& m_scene;
    IESProfile& m_iesProfile;
    EnvironmentMap& m_environmentMap;

    // コピー・ムーブは不可（参照メンバを持つため）
    AssetLoadScope(const AssetLoadScope&)            = delete;
    AssetLoadScope& operator=(const AssetLoadScope&) = delete;
    AssetLoadScope(AssetLoadScope&&)                 = delete;
    AssetLoadScope& operator=(AssetLoadScope&&)      = delete;
};