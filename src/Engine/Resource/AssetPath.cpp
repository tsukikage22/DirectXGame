
#include "Engine/Resource/AssetPath.h"

// 検索パスを初期化
void AssetPath::InitSearchPaths()
{
    // 実行ファイルのパスを取得
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    auto path = std::filesystem::path(exePath).parent_path();

    // 実行ファイルのパスを検索パスに追加
    AddSearchPath(path / "assets");

    // 実行ファイルの親ディレクトリを検索パスに追加
    AddSearchPath(path.parent_path());
}

// 検索パスを追加
void AssetPath::AddSearchPath(const std::filesystem::path& path)
{
    if (std::filesystem::exists(path))
    {
        m_searchPaths.push_back(path);
    }
}

// アセットのパスを取得
std::optional<std::filesystem::path> AssetPath::GetAssetPath(const std::filesystem::path& filename) const
{
    for (const auto& base : m_searchPaths)
    {
        auto fullPath = base / filename;
        if (std::filesystem::exists(fullPath))
        {
            return fullPath;
        }
    }
    return std::nullopt;
}
