#include "Engine/TransformGPU.h"

// コンストラクタ
TransformGPU::TransformGPU() : m_constantBuffer(), m_constants() {
    DirectX::XMStoreFloat4x4(&m_constants.world, DirectX::XMMatrixIdentity());
    DirectX::XMStoreFloat4x4(
        &m_constants.worldInverse, DirectX::XMMatrixIdentity());
}

// デストラクタ
TransformGPU::~TransformGPU() { Term(); }

// 初期化処理，定数バッファの作成
bool TransformGPU::Init(ID3D12Device* pDevice, DescriptorPool* pPoolCBV,
    const DirectX::XMMATRIX& world) {
    // 引数チェック
    if (pDevice == nullptr || pPoolCBV == nullptr) {
        return false;
    }

    // 定数バッファの作成
    if (!m_constantBuffer.Init(
            pDevice, pPoolCBV, sizeof(shader::TransformConstants))) {
        return false;
    }

    // 初期値セット
    Update(world);

    return true;
}

// 終了処理，リソースの解放
void TransformGPU::Term() { m_constantBuffer.Term(); }

// ワールド行列の更新
void TransformGPU::Update(const DirectX::XMMATRIX& world) {
    // 逆行列の計算
    // 今はやっていないが剛体変換かどうかを判定することで最適化ができる
    DirectX::XMVECTOR det;  // 行列式
    DirectX::XMMATRIX inv = DirectX::XMMatrixInverse(&det, world);

    // 行列式が0に近い場合は逆行列が不安定になるため単位行列を返す
    if (DirectX::XMVector4NearEqual(
            det, DirectX::XMVectorZero(), DirectX::XMVectorSplatEpsilon())) {
        inv = DirectX::XMMatrixIdentity();
    }

    // 定数バッファの更新，ワールド行列と逆行列の格納
    // HLSLとC++では行列のメモリ上の格納順序が違うため，どちらかの側で転置が必要
    // C++ は Row-major，HLSLは Column-major
    DirectX::XMMATRIX worldT = DirectX::XMMatrixTranspose(world);
    inv                      = DirectX::XMMatrixTranspose(inv);
    DirectX::XMStoreFloat4x4(&m_constants.world, worldT);
    DirectX::XMStoreFloat4x4(&m_constants.worldInverse, inv);

    // 定数バッファの更新
    m_constantBuffer.Update(&m_constants, sizeof(shader::TransformConstants));
}