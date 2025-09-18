#pragma once

#include <directxtk12/ResourceUploadBatch.h>

#include "Engine/DescriptorPool.h"
#include "Engine/ComPtr.h"

class Texture {
public:
    Texture();
    ~Texture();

    bool Init();
    void Term();

private:
    engine::ComPtr<ID3D12Resource> m_pTex;  // テクスチャリソース
    DescriptorPool* m_pPool;                // ディスクリプタプール
    uint32_t m_index;  // ディスクリプタプールのインデックス

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
};