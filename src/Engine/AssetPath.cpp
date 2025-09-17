
#include <Engine/AssetPath.h>

// 検索パスを初期化
void AssetPath::InitSearchPaths() {
    // 実行ファイルのパスを取得
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    auto path = std::filesystem::path(exePath).parent_path();

    // 実行ファイルのパスを検索パスに追加
    AddSearchPath(path);

    // 実行ファイルの親ディレクトリを検索パスに追加
    AddSearchPath(path.parent_path());

    // 実行ファイルの親ディレクトリの"Assets"フォルダを検索パスに追加
    AddSearchPath(path.parent_path() / "assets");

    AddSearchPath(path.parent_path().parent_path() / "assets");
}

// 検索パスを追加
void AssetPath::AddSearchPath(const std::filesystem::path& path) {
    if (std::filesystem::exists(path)) {
        m_searchPaths.push_back(path);
    }
}

// アセットのパスを取得
bool AssetPath::GetAssetPath(const std::filesystem::path& filename,
    std::filesystem::path& result) const {
    for (const auto& base : m_searchPaths) {
        auto fullPath = base / filename;
        if (std::filesystem::exists(fullPath)) {
            result = fullPath;
            return true;
        }
    }
    return false;
}