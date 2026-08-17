#include "Engine/Debug/DebugUI.h"

#include <cmath>

#include "Engine/Core/EngineConfig.h"
#include "Engine/Graphics/ColorTarget.h"
#include "Engine/Graphics/GraphicsPipelineBuilder.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/Light.h"
#include "Engine/Scene/Scene.h"
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
void DebugUI::BeginFrame(InputSystem& input, Camera& camera, Scene& scene) {
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

    // ライト調整UI
    DrawLightPanel(scene);

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

void DebugUI::DrawLightPanel(Scene& scene) {
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Light")) {
        // すべてのライトに対してUIを描画する
        scene.ForEachLight([&](Light& light) {
            LightType type = light.GetType();  // ライトの種類を取得
            const char* typeName;
            if (type == LightType::Directional) {
                typeName = "Directional Light";
            } else if (type == LightType::Point) {
                typeName = "Point Light";
            } else if (type == LightType::Spot) {
                typeName = "Spot Light";
            } else if (type == LightType::Photometric) {
                typeName = "Photometric Light";
            } else {
                typeName = "Unknown Light";
            }

            ImGui::PushID(&light);  // ライトごとにIDをプッシュ
            if (ImGui::CollapsingHeader(
                    typeName, ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Light ID: %p", &light);

                // 有効・無効の切り替え
                bool enabled = light.IsEnabled();
                if (ImGui::Checkbox("Enable Light", &enabled)) {
                    light.ToggleLight();
                }

                // ライトの色の調整
                float color[3] = { light.GetColor().x, light.GetColor().y,
                    light.GetColor().z };
                float h, s, v;
                // HSVに変換．
                ImGui::ColorConvertRGBtoHSV(
                    color[0], color[1], color[2], h, s, v);

                bool changedColor = false;
                changedColor |= ImGui::SliderFloat("Hue", &h, 0.0f, 1.0f);
                changedColor |=
                    ImGui::SliderFloat("Saturation", &s, 0.0f, 1.0f);
                if (changedColor) {
                    float r, g, b;
                    ImGui::ColorConvertHSVtoRGB(h, s, 1.0f, r, g, b);
                    light.SetColor({ r, g, b });
                }

                // 色温度の調整
                float temp       = 5500.0f;
                bool changedTemp = false;
                if (ImGui::SmallButton("Daylight (5500K)")) {
                    temp        = 5500.0f;
                    changedTemp = true;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Blue Sky (10000K)")) {
                    temp        = 10000.0f;
                    changedTemp = true;
                }
                if (changedTemp) {
                    light.SetColorFromTemperature(temp);
                }

                // ライトの明るさの調整
                if (type == LightType::Directional) {
                    // 平行光源の場合は照度[lx]を設定する
                    float illuminance = light.GetIntensity();
                    if (ImGui::SliderFloat("Illuminance", &illuminance, 100.0f,
                            150000.0f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
                        light.SetIlluminance(illuminance);
                    }
                } else {
                    // 平行光源以外の場合は光度[cd]を設定する
                    float intensity = light.GetIntensity();
                    if (ImGui::SliderFloat("Intensity", &intensity, 1.0f,
                            1000000.0f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
                        light.SetIntensity(intensity);
                    }
                }

                // ライトの方向の調整，方位角と仰角で操作する
                // 方位角と仰角の計算
                if (type != LightType::Point) {
                    float azimuth, elevation, horizontal;
                    float forward[3] = { light.GetTransform().GetForward().x,
                        light.GetTransform().GetForward().y,
                        light.GetTransform().GetForward().z };
                    azimuth          = std::atan2(forward[0], forward[2]);
                    horizontal       = std::sqrt(
                        forward[0] * forward[0] + forward[2] * forward[2]);
                    elevation = std::atan2(forward[1], horizontal);

                    bool changedDirection = false;
                    changedDirection |= ImGui::SliderAngle(
                        "Azimuth", &azimuth, -180.0f, 180.0f);
                    changedDirection |= ImGui::SliderAngle(
                        "Elevation", &elevation, -89.0f, 89.0f);
                    if (changedDirection) {
                        // 方位角と仰角から方向ベクトルを計算
                        float x = std::sin(azimuth) * std::cos(elevation);
                        float y = std::sin(elevation);
                        float z = std::cos(azimuth) * std::cos(elevation);
                        light.GetTransform().LookTo({ x, y, z });
                    }
                }

                // ライト位置の調整
                if (type != LightType::Directional) {
                    float pos[3];
                    pos[0] = light.GetTransform().GetPosition().x;
                    pos[1] = light.GetTransform().GetPosition().y;
                    pos[2] = light.GetTransform().GetPosition().z;
                    if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                        light.GetTransform().SetPosition(
                            { pos[0], pos[1], pos[2] });
                    }
                }

                // ライトの範囲の調整（平行光源以外）
                if (type != LightType::Directional) {
                    float range = light.GetRange();
                    if (ImGui::SliderFloat("Range", &range, 0.1f, 100.0f,
                            "%.2f", ImGuiSliderFlags_Logarithmic)) {
                        light.SetRange(range);
                    }
                }

                // inner Angle, outer Angleの調整（Spotのみ）
                if (type == LightType::Spot) {
                    float innerAngle = light.GetInnerAngle();
                    float outerAngle = light.GetOuterAngle();
                    if (ImGui::SliderFloat(
                            "Inner Angle", &innerAngle, 0.0f, 90.0f)) {
                        light.SetSpotAngles(innerAngle, outerAngle);
                    }
                    if (ImGui::SliderFloat(
                            "Outer Angle", &outerAngle, 0.0f, 90.0f)) {
                        light.SetSpotAngles(innerAngle, outerAngle);
                    }
                }
            }
            ImGui::PopID();  // ライトごとのIDをポップ
        });
    }
    ImGui::End();
}