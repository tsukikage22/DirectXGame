/// @file PassBindings.h
/// @brief パイプラインごとにコマンドリストに渡す情報をまとめた構造体群

#pragma once

#include <d3d12.h>

#include <cstdint>

struct ScenePassBindings {
    ID3D12GraphicsCommandList* pCmdList;   // コマンドリスト
    uint32_t frameIndex;                   // フレーム番号
    ID3D12DescriptorHeap* pCbvSrvUavHeap;  // CBV/SRV/UAV用ディスクリプタヒープ
    D3D12_GPU_VIRTUAL_ADDRESS sceneCB;     // b0 シーンCBのGPUアドレス
    D3D12_GPU_VIRTUAL_ADDRESS displayCB;   // b3  ディスプレイCBのGPUアドレス
    D3D12_GPU_DESCRIPTOR_HANDLE iesSRV;    // t0, space1 iesSRV
    D3D12_GPU_DESCRIPTOR_HANDLE lightSRV;  // t0, space2 ライトバッファのSRV

    /// @brief 初期化漏れを検出するためのチェック
    bool IsValid() const {
        return pCmdList != nullptr && pCbvSrvUavHeap != nullptr &&
               sceneCB != 0 && displayCB != 0 && iesSRV.ptr != 0 &&
               lightSRV.ptr != 0;
    }
};

struct CompositePassBindings {
    ID3D12GraphicsCommandList* pCmdList;   // コマンドリスト
    ID3D12DescriptorHeap* pCbvSrvUavHeap;  // CBV/SRV/UAV用ディスクリプタヒープ
    D3D12_GPU_VIRTUAL_ADDRESS displayCB;   // b3  ディスプレイCBのGPUアドレス
    D3D12_GPU_DESCRIPTOR_HANDLE uiSRV;     // t0, space0 UIテクスチャのSRV

    bool IsValid() const {
        return pCmdList != nullptr && pCbvSrvUavHeap != nullptr &&
               displayCB != 0 && uiSRV.ptr != 0;
    }
};

struct SkyboxPassBindings {
    ID3D12GraphicsCommandList* pCmdList;    // コマンドリスト
    ID3D12DescriptorHeap* pCbvSrvUavHeap;   // CBV/SRV/UAV用ディスクリプタヒープ
    D3D12_GPU_VIRTUAL_ADDRESS sceneCB;      // b0 スカイボックスCBのGPUアドレス
    D3D12_GPU_VIRTUAL_ADDRESS displayCB;    // b3  ディスプレイCBのGPUアドレス
    D3D12_GPU_DESCRIPTOR_HANDLE skyboxSRV;  // t0, space0 スカイボックスのSRV

    bool IsValid() const {
        return pCmdList != nullptr && pCbvSrvUavHeap != nullptr &&
               sceneCB != 0 && displayCB != 0 && skyboxSRV.ptr != 0;
    }
};
