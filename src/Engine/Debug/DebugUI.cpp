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
    // FPS表示
    if (ImGui::Begin("Debug")) {
        ImGui::Text(
            "%.1f FPS (%.3f ms/frame)", io.Framerate, 1000.0f / io.Framerate);
    }
    ImGui::End();

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