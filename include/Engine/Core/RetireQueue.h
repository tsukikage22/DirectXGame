/// @file RetireQueue.h
/// @brief 遅延解放キューの実装

#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

#include "Engine/Core/EngineConfig.h"

template <typename T>
class RetireQueue {
public:
    /// @brief 遅延解放キューにオブジェクトを追加する
    void Retire(T obj, uint32_t frameIndex) {
        assert(frameIndex < config::kFrameCount && "Frame index out of range");
        m_retireQueue[frameIndex].push_back(std::move(obj));
    }

    /// @brief 指定フレームの遅延解放キューをクリアする
    void Clear(uint32_t frameIndex) {
        assert(frameIndex < config::kFrameCount && "Frame index out of range");
        m_retireQueue[frameIndex].clear();
    }

    /// @brief 全フレームの遅延解放キューをクリアする
    void ClearAll() {
        for (auto& queue : m_retireQueue) {
            queue.clear();
        }
    }

private:
    std::array<std::vector<T>, config::kFrameCount>
        m_retireQueue;  // 遅延解放キュー
};