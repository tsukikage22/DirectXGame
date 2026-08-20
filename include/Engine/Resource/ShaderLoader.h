/// @file ShaderLoader.h
/// @brief シェーダーの読み込みを行う関数定義

#pragma once

#include <d3dcompiler.h>

#include <filesystem>

#include "Engine/Core/ComPtr.h"
#include "Engine/Resource/AssetPath.h"

/// @brief シェーダーを読み込む
/// @param filename シェーダーのファイル名
/// @param outBlob 出力先
[[nodiscard]] inline bool LoadShader(
    const wchar_t* filename, engine::ComPtr<ID3DBlob>& outBlob) {
    // パスの取得
    std::filesystem::path shaderPath;
    AssetPath assetPath;
    if (!assetPath.GetAssetPath(filename, shaderPath)) {
        OutputDebugStringW(L"Failed to find shader file.\n");
        return false;
    }

    // シェーダの読み込み
    HRESULT hr =
        D3DReadFileToBlob(shaderPath.c_str(), outBlob.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        OutputDebugStringW(L"Failed to read shader file.\n");
        return false;
    }

    return true;
}
