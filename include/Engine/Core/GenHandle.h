/// @file Handle.h
/// @brief ハンドルの定義

#pragma once

#include <cstdint>

namespace engine {
template <typename Tag>
struct GenHandle {
    uint32_t index      = UINT32_MAX;  // インデックス
    uint32_t generation = 0;           // 世代カウンタ

    bool IsValid() const { return index != UINT32_MAX; }
};

struct GameObjectTag {};
struct ModelTag {};

using ObjectHandle = GenHandle<GameObjectTag>;
using ModelHandle  = GenHandle<ModelTag>;

}  // namespace engine