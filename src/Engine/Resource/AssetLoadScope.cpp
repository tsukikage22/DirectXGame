#include "Engine/Resource/AssetLoadScope.h"

#include "Engine/Core/CommandQueue.h"
#include "Engine/Resource/IESProfile.h"
#include "Engine/Resource/ModelLoader.h"
#include "Engine/Scene/Scene.h"

/// @brief Begin済みのbatchを受け取り，モデルロードができるようにする
AssetLoadScope::AssetLoadScope(
    std::unique_ptr<DirectX::ResourceUploadBatch> pbatch, CommandQueue& queue,
    ModelLoader& loader, Scene& scene, IESProfile& iesProfile)
    : m_pbatch(std::move(pbatch)),
      m_queue(queue),
      m_loader(loader),
      m_scene(scene),
      m_iesProfile(iesProfile) {};

/// @brief デストラクタでResouceUploadBatchとUploadヒープの破棄をする
AssetLoadScope::~AssetLoadScope() {
    if (!m_pbatch) {
        return;
    }
    auto future = m_pbatch->End(m_queue.GetD3DQueue());
    future.wait();  // 転送完了を待機
    m_scene.DiscardModelUploads();
}

/// @brief モデルをロードし，シーンに登録する
engine::ModelHandle AssetLoadScope::LoadModel(
    const std::filesystem::path& path) {
    auto model                      = m_loader.LoadModel(path, *m_pbatch);
    engine::ModelHandle modelHandle = m_scene.RegisterModel(std::move(model));
    assert(modelHandle.IsValid() && "Failed to load model.");
    return modelHandle;
}

/// @brief IESプロファイルをロードし，配光テクスチャを作成する
std::optional<uint32_t> AssetLoadScope::LoadIESProfile(
    const std::filesystem::path& path) {
    return m_iesProfile.CreateIESTexture(path, *m_pbatch);
}