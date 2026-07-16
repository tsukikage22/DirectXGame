#include "Engine/Resource/ModelLoadScope.h"

#include "Engine/Core/CommandQueue.h"
#include "Engine/Resource/ModelLoader.h"
#include "Engine/Scene/Scene.h"

/// @brief Begin済みのbatchを受け取り，モデルロードができるようにする
ModelLoadScope::ModelLoadScope(
    std::unique_ptr<DirectX::ResourceUploadBatch> pbatch, CommandQueue& queue,
    ModelLoader& loader, Scene& scene)
    : m_pbatch(std::move(pbatch)),
      m_queue(queue),
      m_loader(loader),
      m_scene(scene) {};

/// @brief デストラクタでResouceUploadBatchとUploadヒープの破棄をする
ModelLoadScope::~ModelLoadScope() {
    if (!m_pbatch) {
        return;
    }
    auto future = m_pbatch->End(m_queue.GetD3DQueue());
    future.wait();  // 転送完了を待機
    m_scene.DiscardModelUploads();
}

/// @brief モデルをロードし，シーンに登録する
engine::ModelHandle ModelLoadScope::LoadModel(
    const std::filesystem::path& path) {
    auto model                      = m_loader.LoadModel(path, *m_pbatch);
    engine::ModelHandle modelHandle = m_scene.RegisterModel(std::move(model));
    assert(modelHandle.IsValid() && "Failed to load model.");
    return modelHandle;
}