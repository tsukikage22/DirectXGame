#include "Engine/Graphics/ScreenCapture.h"

#include <DirectXPackedVector.h>
#include <DirectXTex.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "Engine/Core/DxDebug.h"

void ScreenCapture::Init(ID3D12Device* device)
{
    m_pDevice = device;
}

bool ScreenCapture::RecordReadback(ID3D12GraphicsCommandList* commandList, ID3D12Resource* backBuffer)
{
    // 必要バイト数を取得する
    D3D12_RESOURCE_DESC desc = backBuffer->GetDesc();
    UINT64 totalBytes        = 0;
    m_pDevice->GetCopyableFootprints(&desc, 0, 1, 0, &m_readbackFootprint, nullptr, nullptr, &totalBytes);

    // READBACKバッファの作成
    D3D12_RESOURCE_DESC readbackDesc = {};
    readbackDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    readbackDesc.Alignment           = 0;
    readbackDesc.Width               = totalBytes;
    readbackDesc.Height              = 1;
    readbackDesc.DepthOrArraySize    = 1;
    readbackDesc.MipLevels           = 1;
    readbackDesc.Format              = DXGI_FORMAT_UNKNOWN;
    readbackDesc.SampleDesc.Count    = 1;
    readbackDesc.SampleDesc.Quality  = 0;
    readbackDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    readbackDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type                  = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask      = 1;
    heapProps.VisibleNodeMask       = 1;

    auto hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &readbackDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(m_pReadbackBuffer.ReleaseAndGetAddressOf()));
    CHECK_HR(m_pDevice, hr);

    // コマンドリストにコピーコマンドを記録する（リソースバリアは呼び出し側）
    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource                   = backBuffer;
    srcLocation.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex            = 0;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource                   = m_pReadbackBuffer.Get();
    dstLocation.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dstLocation.PlacedFootprint             = m_readbackFootprint;

    commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

    return true;
}

bool ScreenCapture::SaveAsPNG(const wchar_t* filename, float paperWhiteNits)
{
    const uint32_t width  = m_readbackFootprint.Footprint.Width;
    const uint32_t height = m_readbackFootprint.Footprint.Height;

    // READBACKバッファをvectorにmapする
    uint8_t* mappedData   = nullptr;
    D3D12_RANGE readRange = { 0, static_cast<SIZE_T>(m_pReadbackBuffer->GetDesc().Width) };
    auto hr               = m_pReadbackBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));
    CHECK_HR(m_pDevice, hr);

    // 出力バッファ
    std::vector<uint8_t> sdrPixels(static_cast<size_t>(width) * height * 4);

    // 保存のためにピクセル変換を行う
    for (UINT row = 0; row < height; row++)
    {
        // 行の開始位置
        // READBACKバッファの各行の後ろにパディングがあるため，１行の長さをRowPitchで計算する
        const uint8_t* srcRow =
            mappedData + m_readbackFootprint.Offset + static_cast<size_t>(row) * m_readbackFootprint.Footprint.RowPitch;
        uint8_t* dstRow = sdrPixels.data() + static_cast<size_t>(row) * width * 4;

        for (uint32_t col = 0; col < width; col++)
        {
            // ピクセルの位置
            const uint8_t* srcPixel = srcRow + col * 8;
            uint8_t* dstPixel       = dstRow + col * 4;

            for (int c = 0; c < 3; c++)
            { // R, G, B
                // halfの２バイトを取り出してfloatに変換
                uint16_t half = 0;
                std::memcpy(&half, srcPixel + c * 2, sizeof(half));
                float v = DirectX::PackedVector::XMConvertHalfToFloat(half);

                // scRGBの1.0を白に設定する
                v = v * 80.0f / paperWhiteNits;

                // 0~1に収める
                v = std::clamp(v, 0.0f, 1.0f);

                // リニア値からsRGBガンマへ
                v = (v <= 0.0031308f) ? (v * 12.92f) : (1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f);

                // 0~255の整数へ
                dstPixel[c] = static_cast<uint8_t>(v * 255.0f + 0.5f);
            }

            // アルファは不透明固定
            dstRow[col * 4 + 3] = 255;
        }
    }

    // READBACKバッファのUnmap
    D3D12_RANGE writeRange = { 0, 0 };
    m_pReadbackBuffer->Unmap(0, &writeRange);

    // 出力バッファをファイルに保存
    DirectX::Image image = {};
    image.width          = width;
    image.height         = height;
    image.format         = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    image.rowPitch       = static_cast<size_t>(width) * 4;
    image.slicePitch     = image.rowPitch * height;
    image.pixels         = sdrPixels.data();

    hr = DirectX::SaveToWICFile(image, DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), filename);
    CHECK_HR(m_pDevice, hr);

    return true;
}
