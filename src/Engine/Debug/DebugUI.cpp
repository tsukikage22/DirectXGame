#include "Engine/Debug/DebugUI.h"

#include "Engine/Core/EngineConfig.h"
#include "Engine/Graphics/ColorTarget.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Scene/Camera.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"
#include "imgui.h"

// ImGuiの初期化
bool DebugUI::Init(ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue,
    DXGI_FORMAT format, DescriptorPool* pPoolCBV_SRV_UAV, HWND hWnd) {
    // ImGuiのコンテキストを作成
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // ImGuiのバックエンドを初期化
    ImGui_ImplDX12_InitInfo info = {};
    info.Device                  = pDevice;
    info.CommandQueue            = pCommandQueue;
    info.NumFramesInFlight       = config::kFrameCount;
    info.RTVFormat               = format;

    // SRVディスクリプタプールの割り当てと解放
    info.SrvDescriptorHeap    = pPoolCBV_SRV_UAV->GetHeap();
    m_ImGuiSrvAllocator.pPool = pPoolCBV_SRV_UAV;
    info.UserData = &m_ImGuiSrvAllocator;  // UserDataにpoolとallocationsを渡す

    info.SrvDescriptorAllocFn =  // ディスクリプタの割り当て関数
        [](ImGui_ImplDX12_InitInfo* im_info,
            D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu,
            D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
            ImGuiSrvAllocator* pAllocator =
                static_cast<ImGuiSrvAllocator*>(im_info->UserData);

            DescriptorAllocation alloc = pAllocator->pPool->Allocate();
            *out_cpu                   = alloc.GetCPUHandle();
            *out_gpu                   = alloc.GetGPUHandle();

            // ハンドルをキーにallocationを保持
            pAllocator->allocations.emplace(out_cpu->ptr, std::move(alloc));
        };

    info.SrvDescriptorFreeFn =  // ディスクリプタの解放関数
        [](ImGui_ImplDX12_InitInfo* im_info, D3D12_CPU_DESCRIPTOR_HANDLE cpu,
            D3D12_GPU_DESCRIPTOR_HANDLE gpu) {
            ImGuiSrvAllocator* pAllocator =
                static_cast<ImGuiSrvAllocator*>(im_info->UserData);
            // CPUハンドルをキーにしてallocationを解放
            pAllocator->allocations.erase(cpu.ptr);
        };

    if (!ImGui_ImplDX12_Init(&info)) return false;
    if (!ImGui_ImplWin32_Init(hWnd)) return false;
    return true;
}

// ImGuiの終了処理
void DebugUI::Term() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_ImGuiSrvAllocator.allocations.clear();
}

// デバッグUIのフレーム開始時の処理
void DebugUI::BeginFrame(InputSystem& input, Camera& camera) {
    // ImGui描画開始
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Input Systemの更新
    ImGuiIO& io = ImGui::GetIO();
    input.SetUICaptureState(io.WantCaptureMouse, io.WantCaptureKeyboard);

    // デバッグGUIの作成
    // FPS表示UI
    DrawFPSPanel();

    // 露出調整UI
    DrawExposurePanel(camera);

    // 描画データの確定
    ImGui::Render();
}

// デバッグUIのレンダリング
void DebugUI::Render(
    ColorTarget& uiTarget, ID3D12GraphicsCommandList* pCmdList) {
    // UI用レンダーターゲットのリソースバリアの設定
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = uiTarget.GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    pCmdList->ResourceBarrier(1, &barrier);

    // UI用レンダーターゲットの設定
    auto rtvHandle = uiTarget.GetRTVCPUHandle();
    BeginPass(pCmdList, kImGuiLayout, &rtvHandle, nullptr);

    // レンダーターゲットのクリア
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    pCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // ImGuiの描画
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCmdList);

    // UI用レンダーターゲットをSRVに戻す
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    pCmdList->ResourceBarrier(1, &barrier);
}

// FPS表示UIの描画
void DebugUI::DrawFPSPanel() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Debug")) {
        ImGui::Text(
            "%.1f FPS (%.3f ms/frame)", io.Framerate, 1000.0f / io.Framerate);
    }
    ImGui::End();
}

// 露出調整UIの描画
void DebugUI::DrawExposurePanel(Camera& camera) {
    // 露出調整パネル
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Exposure")) {
        float ev100 = camera.ComputeEV100();  // 現在のEV100を取得

        // EV100のスライダー
        bool changed =
            ImGui::SliderFloat("EV100", &ev100, -6.0f, 17.0f, "%.2f");

        // EV100を固定値に設定するボタン
        if (ImGui::SmallButton("Sunny 16")) {  // 晴天の昼間
            ev100   = 15.0f;
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Indoor")) {  // 室内
            ev100   = 8.0f;
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Night")) {  // 夜景
            ev100   = -2.0f;
            changed = true;
        }

        // シャッタースピードを固定するか絞り値を固定するかの選択
        if (ImGui::RadioButton("Fix Shutter Speed", m_fixShutterSpeed)) {
            m_fixShutterSpeed = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Fix Aperture", !m_fixShutterSpeed)) {
            m_fixShutterSpeed = false;
        }

        // EV100が変更された場合はシャッタースピードを更新
        if (changed) {
            camera.ApplyEV100(ev100, m_fixShutterSpeed);
        }

        // 露出パラメータの表示
        ImGui::Text("Aperture: f/%.1f", camera.GetAperture());
        float ss = camera.GetShutterSpeed();
        if (ss >= 1.0f) {
            ImGui::Text("Shutter Speed: %.1f s", ss);
        } else {
            ImGui::Text("Shutter Speed: 1/%.0f s", 1.0f / ss);
        }
        ImGui::Text("Exposure: %.3e", camera.ComputeExposure());
    }
    ImGui::End();
}