/// @file SlotMap.h
/// @brief SlotMapの実装

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "Engine/Core/GenHandle.h"

template <typename T, typename Tag>
class SlotMap {
public:
    using HandleType = engine::GenHandle<Tag>;

    /// @brief 空きスロットへ追加しハンドルを返す
    HandleType Insert(T value) {
        uint32_t index;
        if (!m_freeList.empty()) {
            // フリーリストから再利用
            index = m_freeList.back();
            m_freeList.pop_back();
        } else {
            // 新しいスロットを追加
            index = static_cast<uint32_t>(m_slots.size());
            m_slots.push_back(Slot{ 0, 0 });
        }

        // データを追加しスロットを更新
        m_data.push_back(
            std::move(value));  // Tがunique_ptrでも動作するようにmoveを使用
        m_dataToSlot.push_back(index);
        m_slots[index].dataIndex = static_cast<uint32_t>(m_data.size() - 1);

        return HandleType{ index, m_slots[index].generation };
    }

    /// @brief ハンドルに対応する要素を削除する
    std::optional<T> Erase(HandleType h) {
        // 引数のチェック
        if (h.index >= m_slots.size()) return std::nullopt;

        // スロットのgenerationを確認
        Slot& slot = m_slots[h.index];
        if (slot.generation != h.generation) return std::nullopt;

        // 実データ配列は末尾要素を削除対象の位置に移動して詰める
        uint32_t lastDataIndex   = m_data.size() - 1;
        uint32_t erasedDataIndex = slot.dataIndex;
        T erasedData             = std::move(m_data[slot.dataIndex]);
        if (erasedDataIndex != lastDataIndex) {
            // 末尾要素の削除時は自己ムーブ代入になるので回避する
            m_data[erasedDataIndex] = std::move(m_data[lastDataIndex]);

            // dataToSlotも実データ配列に合わせて調整
            uint32_t movedSlot            = m_dataToSlot[lastDataIndex];
            m_dataToSlot[erasedDataIndex] = movedSlot;

            // 間接参照テーブルも実データ配列に合わせて調整
            m_slots[movedSlot].dataIndex = erasedDataIndex;
        }
        m_data.pop_back();
        m_dataToSlot.pop_back();

        // generationを進めて削除した要素のインデックスをフリーリストに追加
        slot.generation++;
        m_freeList.push_back(h.index);

        return erasedData;
    }

    /// @brief generationを確認し有効なら実体を返す
    T* Get(HandleType h) {
        // 引数のチェック
        if (h.index >= m_slots.size()) return nullptr;

        const Slot& slot = m_slots[h.index];
        if (slot.generation != h.generation) {
            return nullptr;
        }
        return &m_data[slot.dataIndex];
    }

    /// @brief 全要素に対してfnを呼び出す
    template <typename Fn>
    void ForEach(Fn&& fn) {
        for (auto& item : m_data) {
            fn(item);
        }
    }

    // イテレータの取得
    auto begin() { return m_data.begin(); }
    auto end() { return m_data.end(); }

private:
    struct Slot {
        uint32_t dataIndex;
        uint32_t generation = 0;
    };
    std::vector<T> m_data;      // 実データ
    std::vector<Slot> m_slots;  // 間接参照テーブル
    std::vector<uint32_t>
        m_dataToSlot;  // データインデックスからスロットインデックスへの変換テーブル
    std::vector<uint32_t> m_freeList;  // フリーリスト
};
