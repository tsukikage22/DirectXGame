#include "Engine/Core/Renderer.h"

#include "Engine/Core/GraphicsDevice.h"

bool Renderer::Init(
    GraphicsDevice& device, HWND hWnd, uint32_t width, uint32_t height) {
    return m_SwapChain.Init(device, width, height, hWnd);
}

void Renderer::Term() { m_SwapChain.Term(); }
