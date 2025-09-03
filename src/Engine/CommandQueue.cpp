#include "Engine/CommandQueue.h"

// コンストラクタ
CommandQueue::CommandQueue()
    : m_pFence(nullptr), m_nextFence(1), m_fenceEvent(nullptr) {}

// デストラクタ
CommandQueue::~CommandQueue() { Term(); }

// コマンドキューとフェンスを作成して初期化
bool CommandQueue::Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type) {
    // 引数チェック
    if (!pDevice) {
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    auto hr = pDevice->CreateCommandQueue(
        &desc, IID_PPV_ARGS(m_pQueue.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    hr = pDevice->CreateFence(
        0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.GetAddressOf()));
    if (FAILED(hr)) {
        return false;
    }

    m_fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    if (m_fenceEvent == nullptr) {
        return false;
    }

    m_nextFence = 1;

    return true;
}

// フェンスが指定した値になるまで待機
void CommandQueue::Wait(UINT64 fenceValue, UINT timeout) {
    if (m_fenceEvent == nullptr || m_pFence == nullptr) {
        return;
    }

    if (m_pFence->GetCompletedValue() >= fenceValue) {
        return;
    }

    auto hr = m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
    if (FAILED(hr)) {
        return;
    }

    WaitForSingleObject(m_fenceEvent, timeout);
}

// 現時点のコマンドをすべて実行させる
void CommandQueue::Flush() {
    if (m_pQueue == nullptr || m_pFence == nullptr) {
        return;
    }

    // シグナル処理
    auto hr = m_pQueue->Signal(m_pFence.Get(), m_nextFence);
    if (FAILED(hr)) {
        return;
    }

    // イベント設定
    hr = m_pFence->SetEventOnCompletion(m_nextFence, m_fenceEvent);
    if (FAILED(hr)) {
        return;
    }

    // 待機
    if (WAIT_OBJECT_0 != WaitForSingleObject(m_fenceEvent, INFINITE)) {
        return;
    }

    m_nextFence++;
}

// コマンドリストの実行
void CommandQueue::Execute(ID3D12CommandList* const* lists, UINT count) {
    // 引数チェック
    if (lists == nullptr || count == 0) {
        return;
    }

    // コマンドリストを実行
    m_pQueue->ExecuteCommandLists(count, lists);
}

// シグナルを送信し，フェンスの値を発行する
UINT64 CommandQueue::Signal() {
    if (m_pQueue == nullptr || m_pFence == nullptr) {
        return 0;
    }

    // シグナル処理
    const auto fenceValue = m_nextFence;
    auto hr = m_pQueue->Signal(m_pFence.Get(), fenceValue);
    if (FAILED(hr)) {
        return 0;
    }

    m_nextFence++;
    return fenceValue;
}

// 終了処理
void CommandQueue::Term() {
    // ハンドルを閉じる
    Flush();
    if (m_fenceEvent != nullptr) {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }

    // フェンスを解放する
    m_pFence.Reset();

    // カウンターをリセット
    m_nextFence = 0;

    // コマンドキューを解放する
    m_pQueue.Reset();
}

ID3D12CommandQueue* CommandQueue::GetD3DQueue() const { return m_pQueue.Get(); }