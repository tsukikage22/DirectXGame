#include "Engine/Resource/ShaderLoader.h"

#include <Windows.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <vector>

#include "Engine/Resource/AssetPath.h"

namespace /* anonymous */
{

std::vector<std::byte> LoadBinary(const std::filesystem::path& path)
{
    // ファイル末尾を開き，サイズを取得する
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
    {
        OutputDebugStringA(("Failed to open shader file: " + path.string() + "\n").c_str());
        return {};
    }

    // ファイルサイズを取得し，先頭に戻す
    const std::streamsize size = file.tellg();
    if (size <= 0)
    {
        OutputDebugStringA(("Shader file is empty: " + path.string() + "\n").c_str());
        return {};
    }
    file.seekg(0, std::ios::beg);

    // ファイルの内容を読み込む
    std::vector<std::byte> data(static_cast<std::size_t>(size));
    if (!file.read(reinterpret_cast<char*>(data.data()), size))
    {
        OutputDebugStringA(("Failed to read shader file: " + path.string() + "\n").c_str());
        return {};
    }
    return data;
}

} // namespace

// シェーダーを読み込む
bool LoadShader(const wchar_t* filename, std::vector<std::byte>& out)
{
    // パスの取得
    std::filesystem::path shaderPath;
    AssetPath assetPath;
    if (!assetPath.GetAssetPath(filename, shaderPath))
    {
        OutputDebugStringA("Failed to find shader file.\n");
        return false;
    }

    // シェーダの読み込み
    out = LoadBinary(shaderPath);
    if (out.empty())
    {
        return false;
    }

    return true;
}
