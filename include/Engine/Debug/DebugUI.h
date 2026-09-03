/// @file DebugUI.h
/// @brief デバッグUIの管理
#pragma once

#include <Windows.h>
#include <d3d12.h>

#include <unordered_map>

#include "Engine/Core/DescriptorAllocation.h"
#include "Engine/Scene/Light.h"

// 前方宣言
class InputSystem;
class Camera;
class ColorTarget;
class Scene;
class GraphicsDevice;

class DebugUI
{
public:
    DebugUI()  = default;
    ~DebugUI() = default;

    /// @brief ImGuiの初期化
    /// @param graphicsDevice GraphicsDevice
    /// @param format バックバッファのフォーマット
    /// @param hWnd ウィンドウハンドル
    bool Init(GraphicsDevice& graphicsDevice, DXGI_FORMAT format, HWND hWnd);

    /// @brief ImGuiの終了処理
    void Term();

    /// @brief デバッグUIのフレーム開始時の処理
    /// @param input InputSystemの参照
    /// @param camera Cameraの参照
    /// @param scene Sceneの参照
    /// @param shadowMapSRV シャドウマップのSRVのGPUハンドル
    void BeginFrame(InputSystem& input, Camera& camera, Scene& scene, D3D12_GPU_DESCRIPTOR_HANDLE shadowMapSRV);

    /// @brief デバッグUIのレンダリング
    /// @param uiTarget UI用レンダーターゲット
    /// @param pCmdList コマンドリスト
    void Render(ColorTarget& uiTarget, ID3D12GraphicsCommandList* pCmdList);

    /// @brief デバッグビューの種類の取得
    /// @return
    int GetDebugView() const
    {
        return m_debugView;
    }

private:
    //=========================================
    // Inner Class
    //=========================================

    /// @brief ImGui用のディスクリプタアロケータ
    struct ImGuiSrvAllocator
    {
        DescriptorPool* pPool;
        std::unordered_map<SIZE_T, DescriptorAllocation> allocations;
    };

    //=========================================
    // private methods
    //=========================================

    /// @brief FPS表示UIの描画
    void DrawFPSPanel();

    /// @brief 露出調整UIの描画
    /// @param camera Cameraの参照
    void DrawExposurePanel(Camera& camera);

    /// @brief ライト調整UIの描画
    /// @param scene Sceneの参照
    void DrawLightPanel(Scene& scene);

    /// @brief デバッグビューUIの描画
    void DrawDebugViewPanel();

    /// @brief シャドウマップ確認UIの描画
    /// @param shadowMapSRV シャドウマップのSRVのGPUハンドル
    void DrawShadowMapPanel(D3D12_GPU_DESCRIPTOR_HANDLE shadowMapSRV);

    //=========================================
    // private variables
    //=========================================
    // ImGui用のディスクリプタアロケータ
    ImGuiSrvAllocator m_ImGuiSrvAllocator;

    // 露出調整パネルのパラメータ
    bool m_fixShutterSpeed = true; // シャッタースピード固定か絞り値固定か

    // デバッグビューの種類
    int m_debugView = 0;

    // コピー禁止
    DebugUI(const DebugUI&)            = delete;
    DebugUI& operator=(const DebugUI&) = delete;
};
