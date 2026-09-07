#pragma once

#include <Windows.h>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

class AssetPath
{
public:
    /// @brief コンストラクタ，検索パスの初期化
    AssetPath()
    {
        InitSearchPaths();
    }

    /// @brief 検索パスを初期化
    void InitSearchPaths();

    /// @brief 検索パスを追加
    /// @param path 追加する検索パス
    void AddSearchPath(const std::filesystem::path& path);

    /// @brief アセットのパスを取得
    /// @param filename 探すアセットのファイル名
    /// @return 見つかった場合はアセットのフルパス，見つからなかった場合はstd::nullopt
    [[nodiscard]] std::optional<std::filesystem::path> GetAssetPath(const std::filesystem::path& filename) const;

private:
    std::vector<std::filesystem::path> m_searchPaths;
};
