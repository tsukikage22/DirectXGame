/// @file ShaderLoader.h
/// @brief シェーダーの読み込みを行う関数定義

#pragma once

#include <cstddef>
#include <vector>

/// @brief シェーダーを読み込む
/// @param filename シェーダーのファイル名
/// @param out 読み込んだシェーダーのバイトコード．失敗時は空．
/// @return 成功した場合はtrue，失敗した場合はfalse
[[nodiscard]] bool LoadShader(
    const wchar_t* filename, std::vector<std::byte>& out);
