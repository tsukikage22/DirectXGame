#include <d3d12.h>

#include "Engine/ComPtr.h"

class Fence {
public:
    Fence();
    ~Fence();

private:
    engine::ComPtr<ID3D12Fence> m_pFence;
    UINT64 m_FenceValue;
};